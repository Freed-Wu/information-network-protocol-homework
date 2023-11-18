#if 0
bin="$(basename "$0")" && bin="${bin%%.*}" && gcc "$0" -lm -o"$bin" && exec ./"$bin" "$@"
#endif
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
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

  if (!((current - 1) % 50))
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

  for (size_t i = 0; i < barwidth; ++i)
    buf[i] = i < barwidth * current / total ? '#' : ' ';

  printf("\33[2AProgress: "
         "[%s]\n\33[2K%3d%%\t%6.2lf%cB\t%4dKBps\t%.1lfs\n",
         buf, current * 100 / total, (double)current * 512 / pow(1024, lb),
         unit[lb], speed, (double)(total - current) / 2 / speed);
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
      sendto(sd, buf, 2, 0, (struct sockaddr *)&peeraddr,
             sizeof(struct sockaddr_in6));
      paused = 1;
      printf("\33[1A\33[2KPAUSED\n");
    } else if (ch == 'r') {
      buf[0] = 5;
      buf[1] = 9;
      sendto(sd, buf, 2, 0, (struct sockaddr *)&peeraddr,
             sizeof(struct sockaddr_in6));
      paused = 0;
      speed_init = 1;
    }
  }
  paused = 1;
  puts("\n[\x1B[33m-\x1B[0m] Quitting");
  buf[0] = 5;
  buf[1] = 0;
  sendto(sd, buf, 2, 0, (struct sockaddr *)&peeraddr,
         sizeof(struct sockaddr_in6));
  exit(0);
}

int main(int argc, char **argv) {
  struct sockaddr_in6 serveraddr = {.sin6_family = AF_INET6,
                                    .sin6_addr = in6addr_any},
                      peeraddr;
  int port = DEFAULT_PORT, sd = -1, blkid, cnt,
      len = sizeof(struct sockaddr_in6);
  char buf[BUFF_LEN];
  FILE *fp;
  struct winsize ws;
  pthread_t thr;
  void *thr_arg[2];
  ioctl(0, TIOCGWINSZ, &ws);
  if (argc < 2)
    printf("Usage:\n%1$s send ip[:port] filename "
           "[speedlimit(KBps)]\n%1$s "
           "recv [port]\n",
           argv[0]);
  else if (strcasecmp(argv[1], "send") == 0) {
    if (argc < 4)
      printf("Usage: %s send ip[:port] filename "
             "[speedlimit(KBps)]\n",
             argv[0]);
    else {
      char *ip = argv[2], *ftmp;
      int ret, acked, sendinterval = 0, ref = 0;
      struct stat fileinfo;
      struct timeval tv = {.tv_sec = 1}, tv0 = {}, tv1, tv2;
      struct timespec nt = {};
      if (!(fp = fopen(argv[3], "r")))
        errx(errno, "[\x1B[31m!\x1B[0m] fopen() error");
      if (fstat(fileno(fp), &fileinfo) < 0)
        errx(errno, "[\x1B[31m!\x1B[0m] fstat() error");
      if (!S_ISREG(fileinfo.st_mode)) {
        fprintf(stderr,
                "[\x1B[31m!\x1B[0m] %s is not a "
                "regular file",
                argv[3]);
        exit(1);
      }
      if (argc > 4) {
        sscanf(argv[4], "%d", &ret);
        sendinterval = 500000 / (ret + 8);
      }
      printf("send %s to %s\n", argv[3], ip);
      if ((sd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
        errx(errno, "[\x1B[31m!\x1B[0m] socket() error");
      char *idx = strchr(ip, ']');
      if (idx && strlen(idx) > 2) {
        sscanf(idx + 2, "%d", &port);
        *idx = 0;
        ip++;
      }
      ret = inet_pton(AF_INET6, ip, &serveraddr.sin6_addr);
      if (ret == 0) {
        fprintf(stderr,
                "[\x1B[31m!\x1B[0m] %s is not a valid "
                "IPv6 address",
                ip);
        exit(1);
      } else if (ret < 0)
        errx(errno, "[\x1B[31m!\x1B[0m] inet_pton() error");
      serveraddr.sin6_port = htons(port);
      cnt = sprintf(buf, "\x02%s %ld %ld",
                    (ftmp = strrchr(argv[3], '/')) ? ftmp + 1 : argv[3],
                    fileinfo.st_size, fileinfo.st_blocks);
      printf("[\x1B[32m+\x1B[0m] Sent WRQ, waiting for "
             "respond...\n");
      sendto(sd, buf, cnt, 0, (struct sockaddr *)&serveraddr,
             sizeof(struct sockaddr_in6));
      do
        cnt =
            recvfrom(sd, buf, BUFF_LEN, 0, (struct sockaddr *)&peeraddr, &len);
      while (cnt <= 0 ||
             memcmp(&serveraddr.sin6_addr, &peeraddr.sin6_addr,
                    sizeof(struct in6_addr)) ||
             serveraddr.sin6_port != peeraddr.sin6_port);
      thr_arg[0] = &sd;
      thr_arg[1] = &serveraddr;
      setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
      if (cnt == 2 && buf[0] == 5) {
        if (buf[1] == 2)
          puts("[\x1B[33m-\x1B[0m] Server "
               "rejected");
        else
          puts("[\x1B[31m!\x1B[0m] Server "
               "reports error");
      } else if (cnt == 2 && buf[0] == 4 && buf[1] == '0')
        for (puts("[\x1B[32m+\x1B[0m] Server "
                  "accepted\n\n"),
             blkid = 1, pthread_create(&thr, NULL, thr_func, thr_arg),
             gettimeofday(&tv1, NULL);
             blkid <= fileinfo.st_blocks; blkid++) {
          cnt = sprintf(buf, "\x03%d ", blkid);
          ret = fread(buf + cnt, sizeof(char), 512, fp);
          if (ferror(fp)) {
            fprintf(stderr, "[\x1B[31m!\x1B[0m] "
                            "Error in reading from "
                            "file\n");
            buf[0] = 5;
            buf[1] = 3;
            sendto(sd, buf, 2, 0, (struct sockaddr *)&serveraddr,
                   sizeof(struct sockaddr_in6));
            exit(1);
          }
          sendto(sd, buf, cnt + ret, 0, (struct sockaddr *)&serveraddr,
                 sizeof(struct sockaddr_in6));
        recv:
          do
            cnt = recvfrom(sd, buf, BUFF_LEN, 0, (struct sockaddr *)&peeraddr,
                           &len);
          while (cnt <= 0 && errno != EAGAIN ||
                 memcmp(&serveraddr.sin6_addr, &peeraddr.sin6_addr,
                        sizeof(struct in6_addr)) ||
                 serveraddr.sin6_port != peeraddr.sin6_port);
          buf[cnt] = 0;
          if (buf[0] == 5) {
            if (buf[1] == 8) {
              paused = 1;
              printf("\33[1A\33["
                     "2KPAUSED\n");
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
                puts("[\x1B["
                     "33m-\x1B["
                     "0m] User "
                     "cancalled"
                     " "
                     "transfe"
                     "r");
              else
                puts("[\x1B["
                     "31m!\x1B["
                     "0m] "
                     "Server "
                     "reports "
                     "error");
              break;
            }
          }
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
      else
        puts("[\x1B[33m-\x1B[0m] Received invalid "
             "response for WRQ "
             "from server");
      close(sd);
      fclose(fp);
    }
  } else if (!strcasecmp(argv[1], "recv")) {
    int size, ch, totalblk, rcvd_id;
    struct sockaddr_in6 clientaddr;
    char ipstr[40], filename[255], *directory = getcwd(NULL, 0);
    if (!directory)
      errx(errno, "[\x1B[31m!\x1B[0m] getcwd() error");
    printf("[\x1B[32m+\x1B[0m] Receive and save file to %s\n", directory);
    if ((sd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
      errx(errno, "[\x1B[31m!\x1B[0m] socket() error");
    serveraddr.sin6_port = htons(port);
    if (bind(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
      errx(errno, "[\x1B[31m!\x1B[0m] bind() error");
  next:
    puts("[\x1B[32m+\x1B[0m] Waiting for WRQ");
    do
      cnt =
          recvfrom(sd, buf, BUFF_LEN, 0, (struct sockaddr *)&clientaddr, &len);
    while (cnt < 4 || buf[0] != 2);
    buf[cnt] = 0;
    inet_ntop(AF_INET6, &clientaddr.sin6_addr, ipstr, sizeof(ipstr));
    sscanf(buf + 1, "%s%d%d", filename, &size, &totalblk);
    printf("[\x1B[32m+\x1B[0m] Received WRQ from %s port %d\n", ipstr,
           ntohs(clientaddr.sin6_port));
    printf("[\x1B[35m*\x1B[0m] Filename: %s Size: %d, accept? "
           "(Y/n)",
           filename, size);
    while ((ch = getchar()) != EOF && ch != '\n' && ch != 'y' && ch != 'Y')
      if (ch == 'n' || ch == 'N') {
        printf("\n[\x1B[33m-\x1B[0m] Rejected %s\n", filename);
        buf[0] = 5;
        buf[1] = 2;
        sendto(sd, buf, 2, 0, (struct sockaddr *)&clientaddr,
               sizeof(struct sockaddr_in6));
        goto next;
      } else
        puts("\n[\x1B[31m!\x1B[0m] Please input 'y' or "
             "'n'");
    printf("\n[\x1B[32m+\x1B[0m] Accepted %s\n\n\n", filename);
    memcpy(&peeraddr, &clientaddr, sizeof(struct sockaddr_in6));
    if (!(fp = fopen(filename, "w"))) {
      buf[0] = 5;
      buf[1] = 3;
      sendto(sd, buf, 2, 0, (struct sockaddr *)&clientaddr,
             sizeof(struct sockaddr_in6));
      errx(errno, "[\x1B[31m!\x1B[0m] fopen() error");
    }
    buf[0] = 4;
    buf[1] = '0';
    sendto(sd, buf, 2, 0, (struct sockaddr *)&clientaddr,
           sizeof(struct sockaddr_in6));
    rcvd_id = 0;
    thr_arg[0] = &sd;
    thr_arg[1] = &clientaddr;
    pthread_create(&thr, NULL, thr_func, thr_arg);
    while (rcvd_id < totalblk) {
      cnt =
          recvfrom(sd, buf, BUFF_LEN, 0, (struct sockaddr *)&clientaddr, &len);
      if (cnt <= 0 ||
          memcmp(&clientaddr, &peeraddr, sizeof(struct sockaddr_in6)))
        continue;
      buf[cnt] = 0;
      if (buf[0] == 5) {
        if (buf[1] == 8) {
          paused = 1;
          printf("\33[1A\33[2KPAUSED\n");
        } else if (buf[1] == 9) {
          paused = 0;
          speed_init = 1;
        } else {
          if (buf[1] == 0)
            puts("[\x1B[33m-\x1B[0m] User "
                 "cancalled transfer");
          else
            puts("[\x1B[31m!\x1B[0m] "
                 "Client reports error");
          break;
        }
      } else if (buf[0] == 3) {
        sscanf(buf + 1, "%d", &blkid);
        if (blkid == rcvd_id + 1) {
          ++rcvd_id;
          char *data = strchr(buf, ' ');
          if (!data++) {
            puts("[\x1B[33m-\x1B[0m] "
                 "Received invalid data "
                 "from "
                 "client");
            continue;
          }
          fwrite(data, sizeof(char), cnt - (data - buf), fp);
        }
        cnt = sprintf(buf, "\x04%d ", rcvd_id);
        sendto(sd, buf, cnt, 0, (struct sockaddr *)&clientaddr,
               sizeof(struct sockaddr_in6));
      } else
        puts("[\x1B[33m-\x1B[0m] Received invalid data "
             "from client");
      updatestatus(totalblk, rcvd_id, ws.ws_col);
    }
    close(sd);
    fclose(fp);
  } else
    printf("Unknown command:%s\n", argv[1]);
  return EXIT_SUCCESS;
}
