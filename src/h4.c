#if 0
bin="$(basename "$0")" && bin="${bin%%.*}" && gcc "$0" -o"$bin" && exec ./"$bin" "$@"
#endif
#include <arpa/inet.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#define DEFAULT_PORT 20000
#define BUFF_LEN 1024
#define PAUSE "\33[1A\33[2KPAUSED"
#define LOG_VERBOSE "[\x1B[35m*\x1B[0m] "
#define LOG_INFO "[\x1B[32m+\x1B[0m] "
#define LOG_WARN "[\x1B[33m-\x1B[0m] "
#define LOG_ERROR "[\x1B[31m!\x1B[0m] "
#define PACKAGE_INTERVAL 50ul
#define PACKAGE_SIZE 512
#define KB 1024

bool paused = false;
bool speed_is_initialized = false;

static inline __suseconds_t tvdiff(struct timeval new_tv,
                                   struct timeval old_tv) {
  return (new_tv.tv_sec - old_tv.tv_sec) * 1000000 + new_tv.tv_usec -
         old_tv.tv_usec;
}

void updatestatus(size_t total, size_t current, int width) {
  if (paused)
    return;

  static int speed = 0;
  static struct timeval last_tv;
  struct timeval tv;
  if (current % PACKAGE_INTERVAL == 1) {
    // exponential smooth
    // speed'_n = 0.9 speed'_{n - 1} + speed_n
    // speed'_n = \sum_{k = 1}^n{0.9^{n - k} speed_k}
    speed *= 0.9;
    gettimeofday(&tv, NULL);
    if (speed_is_initialized)
      speed += PACKAGE_INTERVAL * 1000 * 1000 * PACKAGE_SIZE / KB /
               tvdiff(tv, last_tv);
    else
      speed_is_initialized = true;
    last_tv = tv;
  }

  width -= sizeof("Progress: []") - sizeof("");
  char *buf = malloc(width + sizeof(""));
  if (buf == NULL)
    err(errno, NULL);
  int len = width * current / total;
  memset(buf, '#', len);
  memset(buf + len, ' ', width - len);

  size_t current_size = current * PACKAGE_SIZE;
  size_t unit_size = 1;
  const char units[] = " KMGTP";
  const char *p_unit = units;
  while (*(p_unit + 1) != '\0' && current_size >= unit_size * KB) {
    p_unit++;
    unit_size *= KB;
  }

  // Progress: [##  ]
  // precentage% current_size current_unit speed KB/s lfs s
  printf("\33[2AProgress: [%s]\n\33[2K%3zd%%\t%6.2lf%cB\t%4dKB/s\t%.1lfs\n",
         buf, current * 100 / total, (double)current_size / unit_size, *p_unit,
         speed, (double)(total - current) * PACKAGE_SIZE / KB / speed);
  free(buf);
}

void *thr_func(void *arg) {
  int sd = *(int *)((void **)arg)[0];
  int ch;
  while ((ch = tolower(getchar())) != 'q') {
    switch (ch) {
    case 'p':
      paused = true;
      puts(PAUSE);
      write(sd, "\x05\x08", 2);
      break;
    case 'r':
      paused = false;
      speed_is_initialized = false;
      write(sd, "\x05\x09", 2);
    }
  }
  paused = true;
  puts("\n" LOG_WARN "Quitting");
  write(sd, "\x05\0", 2);
  struct termios oldattr = *(struct termios *)((void **)arg)[1];
  tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
  exit(0);
}

/**
 * disable line buffer (canonical mode), enable echo
 * must be called after connect() to avoid wrong address family
 */
struct termios set_term() {
  struct termios oldattr, newattr;
  tcgetattr(STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~ICANON;
  newattr.c_lflag |= ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
  return oldattr;
}

int main(int argc, char **argv) {
  if (argc < 2)
  usage:
    errx(EXIT_FAILURE,
         "Usage: \n"
         "  %1$s send ip[:port] filename [speed_limit(KB/s)]\n"
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
  // parse speed limit
  int send_interval = 0;
  if (argc > 4) {
    send_interval = strtol(argv[4], NULL, 0);
    if (send_interval <= 0)
      err(EXIT_FAILURE, "%s is not a legal speed limit", argv[4]);
    // unit is microsecond
    send_interval = 1000 * 1000 * PACKAGE_SIZE / KB / send_interval;
  }
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
  void *thr_arg[2] = {&sd};
  // fopen
  FILE *fp;
  // write request: 2 filename ' ' size ' ' block
  size_t size, totalblk;
  // acknowledgment: 4 blkid ' '
  size_t blkid, rcvd_id;
  // while (cont)
  int cont = 1;
  struct termios oldattr;

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
    sscanf(buf + 1, "%s %zd %zd", filename, &size, &totalblk);
    printf(LOG_INFO "Received WRQ from [%s]:%d\n" LOG_VERBOSE
                    "Filename: %s Size: %zd, accept? (Y/n) ",
           ipstr, ntohs(peer_sa.sin6_port), filename, size);
    if (connect(sd, (struct sockaddr *)&peer_sa, len) == -1)
      err(errno, NULL);
    oldattr = set_term();
    thr_arg[1] = &oldattr;

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
    // double "\n"s is due to progress bar
    printf("\n" LOG_INFO "Accepted %s\n\n", filename);
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
          paused = true;
          puts(PAUSE);
          break;
        case 9:
          paused = false;
          speed_is_initialized = false;
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
        sscanf(buf + 1, "%zd", &blkid);
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
        write(sd, buf, sprintf(buf, "\x04%zd ", rcvd_id));
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
    oldattr = set_term();
    thr_arg[1] = &oldattr;

    // send wait request
    if ((fp = fopen(filename, "r")) == NULL)
      err(errno, "%s", filename);
    struct stat fileinfo;
    if (fstat(fileno(fp), &fileinfo) == -1)
      err(errno, "%s", filename);
    if (!S_ISREG(fileinfo.st_mode))
      errx(EXIT_FAILURE, "%s is not a regular file", filename);
    printf(LOG_INFO "Send %s to [%s]:%d\n", filename, ip, port);
    puts(LOG_INFO "Sent WRQ, waiting for respond...");
    write(sd, buf,
          sprintf(buf, "\x02%s %ld %ld", basename(filename), fileinfo.st_size,
                  fileinfo.st_blocks));

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
      // double "\n"s is due to progress bar
      puts(LOG_INFO "Server accepted\n");
      pthread_create(&thr, NULL, thr_func, thr_arg);

      struct timeval tv_normal = {.tv_sec = 1}, tv_paused = {}, last_tv, tv;
      setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv_normal, sizeof(tv_normal));
      gettimeofday(&last_tv, NULL);
      for (blkid = 1; blkid <= fileinfo.st_blocks && cont; blkid++) {
        // send data
        // data: 3 blkid ' ' data
        cnt = sprintf(buf, "\x03%zd ", blkid);
        int ret;
        if ((ret = fread(buf + cnt, sizeof(char), 512, fp)) == -1) {
          write(sd, "\x05\x03", 2);
          err(errno, "%s", filename);
        }
        write(sd, buf, cnt + ret);
        // handle response
        int acked = -1;
        do {
          do
            cnt = read(sd, buf, BUFF_LEN);
          while (cnt < 2);
          buf[cnt] = '\0';
          switch (buf[0]) {
          case 5:
            switch (buf[1]) {
            case 8:
              paused = true;
              puts(PAUSE);
              setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv_paused,
                         sizeof(tv_paused));
              break;
            case 9:
              paused = false;
              speed_is_initialized = false;
              setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv_normal,
                         sizeof(tv_normal));
              gettimeofday(&last_tv, NULL);
              break;
            case 0:
              puts(LOG_WARN "User cancalled transfer");
              goto exit;
            default:
              puts(LOG_ERROR "Server reports error");
              goto invalid;
            }
            break;
          case 4:
            sscanf(buf + 1, "%d", &acked);
            break;
          default:
            puts(LOG_ERROR "Server reports error");
            goto invalid;
          }
        } while (acked != blkid || paused);
        updatestatus(fileinfo.st_blocks, blkid, ws.ws_col);
        gettimeofday(&tv, NULL);
        __suseconds_t usec = tvdiff(tv, last_tv);
        last_tv = tv;
        if (usec < send_interval) {
          struct timespec nt = {.tv_nsec = (send_interval - usec) * 1000};
          nanosleep(&nt, NULL);
        }
      }
      break;
    default:
    invalid:
      puts(LOG_WARN "Received invalid response for WRQ from server");
    }
  }
exit:
  pthread_cancel(thr);
  tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
  close(sd);
  fclose(fp);
  return EXIT_SUCCESS;
}
