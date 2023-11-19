#if 0
bin="$(basename "$0")" && bin="${bin%%.*}" && gcc "$0" -lm -o"$bin" && exec ./"$bin" "$@"
#endif
#include <arpa/inet.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#define DEFAULT_PORT 20000
#define BUFF_LEN 1024
#define PAUSE "\33[1A\33[2KPAUSED"
#define LOG_VERBOSE "[\x1B[35m*\x1B[0m] "
#define LOG_INFO "[\x1B[32m+\x1B[0m] "
#define LOG_WARN "[\x1B[33m-\x1B[0m] "
#define LOG_ERROR "[\x1B[31m!\x1B[0m] "

int paused = 0;
int speed_init = 1;

void updatestatus(int total, int current, int width) {
  if (paused)
    return;
  int barwidth = width - 12;
  char *buf = malloc(barwidth + 1);
  int lb = log10(current * 512) / log10(1024);
  static int speed = 0;
  static struct timeval t1;
  struct timeval t2;
  const char unit[] = " KMGTP";

  if (!((current - 1) % 50)) {
    if (speed_init == 1) {
      gettimeofday(&t1, NULL);
      speed_init = 2;
    } else if (speed_init == 2) {
      gettimeofday(&t2, NULL);
      speed = 25000000 /
              ((t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec);
      memcpy(&t1, &t2, sizeof(struct timeval));
      speed_init = 0;
    } else {
      gettimeofday(&t2, NULL);
      speed *= 0.9;
      speed += 2500000 /
               ((t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec);
      memcpy(&t1, &t2, sizeof(struct timeval));
    }
  }

  for (size_t i = 0; i < barwidth; ++i)
    buf[i] = i < barwidth * current / total ? '#' : ' ';

  printf("\33[2AProgress: [%s]\n\33[2K%3d%%\t%6.2lf%cB\t%4dKBps\t%.1lfs\n", buf,
         current * 100 / total, (double)current * 512 / pow(1024, lb), unit[lb],
         speed, (double)(total - current) / 2 / speed);
  free(buf);
}

void *thr_func(void *arg) {
  char buf[2];
  struct sockaddr_in6 peeraddr = *(struct sockaddr_in6 *)((void **)arg)[1];
  int ch, sd = *(int *)((void **)arg)[0];
  ch = getchar();
  while (ch != 'q') {
    if (ch == 'p') {
      buf[0] = 5;
      buf[1] = 8;
      sendto(sd, buf, 2, 0, (struct sockaddr *)&peeraddr, sizeof(peeraddr));
      paused = 1;
      puts(PAUSE);
    } else if (ch == 'r') {
      buf[0] = 5;
      buf[1] = 9;
      sendto(sd, buf, 2, 0, (struct sockaddr *)&peeraddr, sizeof(peeraddr));
      paused = 0;
      speed_init = 1;
    }
  }
  paused = 1;
  puts("\n" LOG_WARN "Quitting");
  buf[0] = 5;
  buf[1] = 0;
  sendto(sd, buf, 2, 0, (struct sockaddr *)&peeraddr, sizeof(peeraddr));
  exit(0);
}

int main(int argc, char **argv) {
  if (argc < 2)
  usage:
    errx(EXIT_FAILURE,
         "Usage: \n"
         "  %1$s send ip[:port] filename [speedlimit(KBps)]\n"
         "  %1$s recv [ip][:port]",
         argv[0]);
  // parse ip and port
  // FIXME: https://github.com/llvm/llvm-project/issues/68823
  // NOLINTNEXTLINE(readability-misleading-indentation)
  int port = DEFAULT_PORT;
  char *ip = NULL;
  if (argc > 2) {
    ip = argv[2];
    char *idx = strchr(ip, ']');
    if (idx && strlen(idx) > 0) {
      // [ip]:port, strlen("]:") == 2
      sscanf(idx + 2, "%d", &port);
      // change [ip]:port to ip\0:port
      ip++;
      *idx = '\0';
    }
  }
  struct sockaddr_in6 sa = {.sin6_family = AF_INET6,
                            .sin6_port = htons(port),
                            .sin6_addr = in6addr_any},
                      peer_sa;
  if (ip != NULL)
    switch (inet_pton(AF_INET6, ip, &sa.sin6_addr)) {
    case -1:
      err(errno, NULL);
    case 0:
      errx(EXIT_FAILURE, "%s is not a valid IPv6 address", ip);
    }
  // parse speedlimit
  int sendinterval = 0;
  if (argc > 4)
    sendinterval = 500000 / (strtol(argv[4], NULL, 0) + 8);
  // parse command
  char *filename = NULL;
  if (strcasecmp(argv[1], "send") == 0) {
    filename = argv[3];
    if (argc < 4 || strlen(filename) == 0)
      goto usage;
  } else if (strcasecmp(argv[1], "recv") != 0)
    goto usage;

  int sd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sd == -1)
    err(errno, NULL);

  // updatestatus
  struct winsize ws;
  ioctl(0, TIOCGWINSZ, &ws);
  // recv_from
  char buf[BUFF_LEN] = "";
  int cnt;
  // peer_sa
  socklen_t len;
  // pthread_create
  pthread_t thr;
  void *thr_arg[2] = {&sd, &sa};
  // fopen
  FILE *fp;
  // write request: 2 filename ' ' size ' ' block
  int size, totalblk;
  // acknowledgment: 4 blkid ' '
  int blkid, rcvd_id;
  // while (cont)
  int cont = 1;

  if (filename == NULL) {
    // server
    if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
      err(errno, NULL);
    printf(LOG_INFO "Receive and save file to %s\n", getcwd(NULL, 0));

    // wait write request
  wrq:
    puts(LOG_INFO "Waiting for WRQ");
    do
      cnt = recvfrom(sd, buf, BUFF_LEN, 0, (struct sockaddr *)&peer_sa, &len);
    // write request: 2 filename ' ' size ' ' block
    while (cnt < 4 || buf[0] != 2);
    buf[cnt] = '\0';
    char ipstr[40];
    inet_ntop(AF_INET6, &peer_sa.sin6_addr, ipstr, sizeof(ipstr));
    char _filename[256];
    filename = _filename;
    sscanf(buf + 1, "%s %d %d", filename, &size, &totalblk);
    printf(LOG_INFO "Received WRQ from [%s]:%d\n" LOG_VERBOSE
                    "Filename: %s Size: %d, accept? (Y/n) ",
           ipstr, ntohs(peer_sa.sin6_port), filename, size);
    if (connect(sd, (struct sockaddr *)&peer_sa, len) == -1)
      err(errno, NULL);

    // handle write request
    cont = 1;
    while (cont) {
      switch (tolower(getchar())) {
      case 'n':
        // \n is due to (Y/n)
        printf("\n" LOG_WARN "Rejected %s\n", filename);
        write(sd, "\x05\x02", 2);
        goto wrq;
      case 'y':
      case '\n':
      case EOF:
        cont = 0;
        break;
      default:
        // \n is due to (Y/n)
        puts("\n" LOG_ERROR "Please input 'y' or 'n'");
      }
    }
    // \n is due to (Y/n)
    printf("\n" LOG_INFO "Accepted %s\n", filename);
    if ((fp = fopen(filename, "w")) == NULL) {
      write(sd, "\x05\x03", 2);
      err(errno, NULL);
    }

    // acknowledgment: 4 '0'
    write(sd,
          "\x04"
          "0",
          2);
    rcvd_id = 0;
    thr_arg[1] = &peer_sa;
    pthread_create(&thr, NULL, thr_func, thr_arg);
    cont = 1;
    while (rcvd_id < totalblk && cont) {
      if ((cnt = read(sd, buf, BUFF_LEN)) == -1)
        err(errno, NULL);
      buf[cnt] = '\0';
      switch (buf[0]) {
      case 5:
        switch (buf[1]) {
        case 8:
          paused = 1;
          puts(PAUSE);
          break;
        case 9:
          paused = 0;
          speed_init = 1;
          break;
        default:
          if (buf[1] == 0)
            puts(LOG_WARN "User cancalled transfer");
          else
            puts(LOG_ERROR "Client reports error");
          cont = 0;
        }
        break;
      case 3:
        // data: 3 blkid ' ' data
        sscanf(buf + 1, "%d", &blkid);
        if (blkid == rcvd_id + 1) {
          rcvd_id++;
          char *data = strchr(buf, ' ');
          if (!data++) {
            puts(LOG_WARN "Received invalid data from client");
            continue;
          }
          fwrite(data, sizeof(char), cnt - (data - buf), fp);
        }
        // acknowledgment: 4 blkid ' '
        write(sd, buf, sprintf(buf, "\x04%d ", rcvd_id));
        break;
      default:
        puts(LOG_WARN "Received invalid data from client");
      }
      updatestatus(totalblk, rcvd_id, ws.ws_col);
    }

  } else {
    // client
    if (connect(sd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
      err(errno, NULL);

    // send wait request
    if ((fp = fopen(filename, "r")) == NULL)
      err(errno, "%s", filename);
    struct stat fileinfo;
    if (fstat(fileno(fp), &fileinfo) == -1)
      err(errno, "%s", filename);
    if (!S_ISREG(fileinfo.st_mode))
      errx(EXIT_FAILURE, "%s is not a regular file", filename);
    printf("send %s to [%s]:%d\n", filename, ip, port);
    puts(LOG_INFO "Sent WRQ, waiting for respond...");
    write(sd, buf,
          sprintf(buf, "\x02%s %ld %ld", basename(filename), fileinfo.st_size,
                  fileinfo.st_blocks));

    int ref = 0;
    struct timeval tv = {.tv_sec = 1}, tv0 = {}, tv1, tv2;
    struct timespec nt = {};
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // wait acknowledgment
    do
      cnt = read(sd, buf, BUFF_LEN);
    while (cnt == -1);
    if (cnt != 2)
      goto invalid;
    switch (buf[0]) {
    case 5:
      if (buf[1] == 2)
        puts(LOG_WARN "Server rejected");
      else
        puts(LOG_ERROR "Server reports error");
      break;
    case 4:
      if (buf[1] != '0')
        goto invalid;
      puts(LOG_INFO "Server accepted\n");
      gettimeofday(&tv1, NULL);
      pthread_create(&thr, NULL, thr_func, thr_arg);
      for (blkid = 1; blkid <= fileinfo.st_blocks; blkid++) {
        cnt = sprintf(buf, "\x03%d ", blkid);
        int ret;
        if ((ret = fread(buf + cnt, sizeof(char), 512, fp)) == -1) {
          fprintf(stderr, LOG_ERROR "Error in reading from file\n");
          buf[0] = 5;
          buf[1] = 3;
          write(sd, buf, 2);
          exit(1);
        }
        write(sd, buf, cnt + ret);
      recv:
        do
          cnt = write(sd, buf, BUFF_LEN);
        while (cnt < 2);
        buf[cnt] = '\0';
        if (buf[0] == 5) {
          if (buf[1] == 8) {
            paused = 1;
            puts(PAUSE);
            setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv0,
                       sizeof(struct timeval));
          } else if (buf[1] == 9) {
            paused = 0;
            setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv,
                       sizeof(struct timeval));
            gettimeofday(&tv1, NULL);
            ref = blkid;
            speed_init = 1;
          } else {
            if (buf[1] == 0)
              puts(LOG_WARN "User cancalled transfer");
            else
              puts(LOG_ERROR "Server reports error");
            break;
          }
        }
        int acked;
        if (buf[0] == 4)
          sscanf(buf + 1, "%d", &acked);
        if (acked != blkid || paused)
          goto recv;
        updatestatus(fileinfo.st_blocks, acked, ws.ws_col);
        gettimeofday(&tv2, NULL);
        if ((tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec <
            sendinterval * (blkid - ref)) {
          nt.tv_nsec = (sendinterval * (blkid - ref) -
                        ((tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec -
                         tv1.tv_usec)) *
                       1000;
          nanosleep(&nt, NULL);
        }
      }
      break;
    default:
    invalid:
      puts(LOG_WARN "Received invalid response for WRQ from server");
    }
  }
  close(sd);
  fclose(fp);
  return EXIT_SUCCESS;
}
