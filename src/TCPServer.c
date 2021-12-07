#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#define MAXLINE 1024 // 最大通信信息行数
#define TRUE 1
int main(int argc, char** argv) {
	int sockfd, fd, n, m;
	char line[MAXLINE + 1];
	// 定义记录己方和客户端的套接口地址信息的结构体
	struct sockaddr_in6 servaddr, cliaddr;
	time_t t0 = time(NULL);
	printf("time #: %ld\n", t0);
	fputs(ctime(&t0), stdout);
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
	// 自动填上所运行的机器的 IP 地址
	servaddr.sin6_addr = in6addr_any;
	// 使用 bind()将上面得到的套接口绑定到本地某个端口，如果失败，显示错误原因；成功就接着运行
	if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
		perror("bind error");
	// listen()监听，等待接入请求；其中 5 是进入队列中允许的连接数目
	if (listen(sockfd, 5) == -1)
		perror("listen error");

	while (TRUE) {
		printf("> Waiting clients ...\r\n");
		socklen_t clilen = sizeof(struct sockaddr);
		// fd 描述接收连接(accept())
		fd = accept(sockfd, (struct sockaddr*)&cliaddr, &clilen);
		if (fd == -1) {
			perror("accept error");
		}
		printf("> Accepted.\r\n");
		while ((n = read(fd, line, MAXLINE)) > 0) {
			line[n] = 0;
			if (fputs(line, stdout) == EOF)
				// 服务器端打印读入的信息
				perror("fputs error");
		}
		close(fd);
	}
	if (n < 0)
		perror("read error");
	exit(0);
}
