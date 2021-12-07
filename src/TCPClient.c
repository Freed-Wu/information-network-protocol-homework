#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define MAXLINE 1024
#define TRUE 1
int main(int argc, char** argv) {
	int sockfd, n, m;
	char line[MAXLINE + 1];
	struct sockaddr_in6 servaddr;
	time_t t0 = time(NULL);
	printf("time #: %ld\n", t0);
	fputs(ctime(&t0), stdout);
	if (argc != 2)
		// 检测 main()函数是不是只有一个参数输入
		perror("usage: a.out <IPaddress>");
	// 调用 socket()函数，得到对应套接口的文件描述符 sockfd；如果调用失败，perror 会显示错误信息，然后非正常退出
	if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
		perror("socket error");
	// 套接口信息初始化
	// 结构体其余清零
	bzero(&servaddr, sizeof(servaddr));
	// ADDRESS FAMILY 地址族
	servaddr.sin6_family = AF_INET6;
	// 把输入整数转换成“网络字节顺序”
	servaddr.sin6_port = htons(20000);
	// 将输入参数转换成二进制格式，并存入 servaddr.sin6_addr
	if (inet_pton(AF_INET6, argv[1], &servaddr.sin6_addr) <= 0)
		perror("inet_pton error");
	// 连接正在监听的服务器
	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		perror("connect error");
	// 发送数据
	while (fgets(line, MAXLINE, stdin) != NULL) {
		send(sockfd, line, strlen(line), 0);
	}
	// 关闭套接口
	close(sockfd);
	exit(0);
}
