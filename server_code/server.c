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
#define P sem_wait
#define V sem_post
#define mutex &mutex_real

int sock;//客户端socket
char * IP = "172.18.158.104"";//服务器的IP
int PORT = 66666;//服务器服务端口
int size = 20;
int fds[20] = { 0 };
int user = 0;

void init() {
	sock = socket(AF_INET, SOCK_STREAM, 0); // 文件描述符
	if (sock < 0)
	{
		perror("sock error\n");
		exit(-1);
	}
	int mw_optval = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		(char *)&mw_optval, sizeof(mw_optval));
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_port = htons(PORT);
	int b = bind(sock, (struct sockaddr*)&server, sizeof(server));
	if (b < 0)
	{
		perror("bind error\n");
		exit(-1);
	}
	// 监听
	int l = listen(sock, 20); // 最大的连接数量
	if (l < 0)
	{
		perror("listen error\n");
		exit(-1);
	}

	printf("服务器启动\n");

}
void* service_thread(void* p)
{
	int fd = *(int*)p;
	int readbytes;
	char sendbuf[512] = { 0 };
	char recvbuf[512] = { 0 };
	while (1)
	{
		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
		// 接受请求
		if (recv(fd, recvbuf, sizeof(recvbuf), 0) <= 0) {
			int i;
			for (i = 0; i < size; i++) {
				if (fd == fds[i]) {
					fds[i] = 0;
					break;
				}
			}
			user--;
			printf("fd = %d\t连接断开\n当前在线人数：%d\n", fd, user);
			pthread_exit(0);
		}
		else {

			printf("fd%d : %s", fd, recvbuf);
			sprintf(sendbuf, "fd=%d:", fd);
			strcat(sendbuf, recvbuf);
			int i;
			for (i = 0; i <= user; i++) {
				if (fds[i] != fd) {
					send(fds[i], sendbuf, strlen(sendbuf), 0);
				}
			}
		}
	}
}

int main(int argc, char* argv[])
{
	int i = 0;
	pthread_t tid[size];
	init();
	while (1) {
		// 接受请求处理
		struct sockaddr_in client;
		socklen_t len = sizeof(client);
		int new_sock = accept(sock, (struct sockaddr*)&client, &len);
		for (i = 0; i < size; i++) {
			if (fds[i] == 0) {
				//记录客户端的socket
				fds[i] = new_sock;
				user++;
				printf("[%s:%d]连接fd=%d\n当前在线人数：%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), new_sock, user);
				break;
			}
		}
		//有客户端连接之后，启动线程给此客户服务
		if (user < 19)
		{
			pthread_create(&tid[user], 0, service_thread, &new_sock);
		}
	}
	close(sock);
	return 0;
}
