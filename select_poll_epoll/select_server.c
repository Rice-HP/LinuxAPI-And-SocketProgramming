#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/select.h>

#define PORT 9999

int main(int argc,char** argv)
{
    int lfd = socket(AF_INET,SOCK_STREAM,0);
    if(lfd == -1)
    {
        perror("socket erro");
        exit(-1);
    }
    int mw_optval = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR,
		(char *)&mw_optval, sizeof(mw_optval));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(-1 == bind(lfd,(struct sockaddr*)&addr,sizeof(addr)))
    {
        perror("bind erro");
        exit(-1);
    }

    if(-1 == listen(lfd,5))
    {
        perror("listen erro");
        exit(-1);
    }

    printf("begin listening...\n");

    //文件描述符表
    fd_set client_fds,reads;
    FD_ZERO(&client_fds);
    FD_SET(lfd,&client_fds);

    int max_fd = lfd;

    char rev_buff[512] = { 0 };
    char ip[64];
    while (1)
    {
        /* 委托检测 */
        reads = client_fds;
        int ret = select(max_fd+1,&reads,NULL,NULL,NULL);

        if (FD_ISSET(lfd,&reads))
        {
            /* 接收连接 */
            struct sockaddr_in caddr;
            int len = sizeof(caddr);
            int cfd = accept(lfd,(struct sockaddr*)&caddr,&len);

            FD_SET(cfd,&client_fds);
            max_fd = cfd > max_fd ? cfd : max_fd;
            printf("[%s : %d]连接,fd = %d ...\n",
                inet_ntop(AF_INET,&caddr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(caddr.sin_port),cfd);
        }
        
        for (int i = lfd + 1; i <= max_fd; i++)
        {
            /* 可读缓冲 */
            if(FD_ISSET(i,&reads))
            {
                int len = read(i,rev_buff,512);
                if(len > 0)
                {
                    printf("Recv:%s,len = %d\n",rev_buff,len);
                    for (char* ptr = rev_buff; *ptr; ptr++) 
                    {  
                        *ptr = toupper(*ptr);  //转大写
                    } 
                    write(i,rev_buff,len);
                    printf("Send:%s,len = %d\n",rev_buff,len);
                }
                else if(len == 0)
                {        
                    printf("[fd%d]断开连接...\n",i);
                    close(i);
                    FD_CLR(i,&client_fds);
                }
                else
                {
                    /* 出错 */
                    break;
                }
            }
        }
        
    }

    close(lfd);

    return 0;
}