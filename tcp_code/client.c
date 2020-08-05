#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

void perro_exit(char *str)
{
    printf("%s ",str);
    perror("erro");
    exit(-1);
}

int main(int argc,char** argv)
{
    if(argc < 3)
    {
        printf("%s : ip port\n",argv[0]);
        exit(-1);
    }

    //创建socket
    int cfd = socket(AF_INET,SOCK_STREAM,0);
    if(cfd == -1) perro_exit("socket");

    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &addr.sin_addr.s_addr);
    addr.sin_port = htons(atoi(argv[2]));

    int c = connect(cfd,(struct sockaddr *)&addr,sizeof(addr));

    if(-1 == c) perro_exit("connect");

	char sendbuf[512] = { 0 };
    while (1)
	{
        memset(sendbuf,0,512);
		printf("Send:");
		fgets(sendbuf, sizeof(sendbuf), stdin);
        if(strcmp(sendbuf,"qqq\n") == 0)
            break;
		write(cfd, sendbuf, strlen(sendbuf)+1);

        // 接收服务器端的数据
        int len = read(cfd, sendbuf, sizeof(sendbuf));
        printf("read_buf = %s,len = %d\n", sendbuf, len);
	}
    close(cfd);
}