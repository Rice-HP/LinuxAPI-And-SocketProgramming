#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
int sock;//客户端socket
char * IP = "172.18.158.104";//服务器的IP
int PORT = 66666;//服务器服务端口

void init() {
	// 建立连接
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("创建socket失败");
		exit(-1);
	}
	int mw_optval = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		(char *)&mw_optval, sizeof(mw_optval));
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_port = htons(PORT);
	socklen_t len = sizeof(server);

	if (connect(sock, (struct sockaddr*)&server, len) == -1)
	{
		perror("connect error!\n");
		exit(-1);
	}
	printf("客户端启动成功\n");

}
void *sendsocket(void *p)
{
	int fd = *(int *)p;
	while (1)
	{
		char sendbuf[512] = { 0 };
		//	printf("Send:");
		fgets(sendbuf, sizeof(sendbuf), stdin);
		send(sock, sendbuf, strlen(sendbuf), 0); // 会阻塞的写与读
	}
	return NULL;
}
int main(int argc, char* argv[])
{
	pthread_t thrd;
	init();
	pthread_create(&thrd, NULL, sendsocket, &sock);
	while (1)
	{
		char recvbuf[512] = { 0 };
		int l = recv(sock, recvbuf, sizeof(recvbuf), 0);
		if (l > 0)
		{
			recvbuf[l] = '\0';
			printf("Rev.%s", recvbuf);
		}
		else
			exit(-1);

	}
	close(sock);
	return 0;
}
