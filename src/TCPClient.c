#if 0
bin="$(basename "$0")" && bin="${bin%%.*}" && gcc "$0" -o"$bin" && exec ./"$bin" "$@"
#endif
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXLINE 1024

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: %s <IPaddress>\n", basename(argv[0]));
    return EXIT_SUCCESS;
  }

  int sd = socket(AF_INET6, SOCK_STREAM, 0);
  if (sd == -1)
    err(errno, NULL);

  struct sockaddr_in6 sa = {
      .sin6_family = AF_INET6,
      .sin6_port = htons(20000),
  };
  if (inet_pton(AF_INET6, argv[1], &sa.sin6_addr) == -1)
    err(errno, NULL);

  if (connect(sd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    errx(errno, "run TCPServer firstly!");

  char line[MAXLINE + 1];
  while (fgets(line, MAXLINE, stdin))
    send(sd, line, strlen(line), 0);

  return close(sd);
}
