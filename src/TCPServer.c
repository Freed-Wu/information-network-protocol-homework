#if 0
bin="$(basename "$0")" && bin="${bin%%.*}" && gcc "$0" -o"$bin" && exec ./"$bin" "$@"
#endif
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXLINE 1024

int main(int argc, char **argv) {
  int sd = socket(AF_INET6, SOCK_STREAM, 0);
  if (sd == -1)
    err(errno, NULL);

  struct sockaddr_in6 sa = {
      .sin6_family = AF_INET6,
      .sin6_port = htons(20000),
      .sin6_addr = in6addr_any,
  };

  if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    err(errno, NULL);

  if (listen(sd, 5) == -1)
    err(errno, NULL);

  while (1) {
    int fd = accept(sd, NULL, NULL);
    if (fd == -1)
      err(errno, NULL);

    char line[MAXLINE + 1] = "";
    while (read(fd, line, MAXLINE) > 0)
      puts(line);

    close(fd);
  }
}
