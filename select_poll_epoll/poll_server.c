#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <assert.h>
#include <poll.h>

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

    //poll结构体数组
    struct pollfd client_fds[1024];

    for (int i = 0; i < 1024; i++)
    {
        /* 初始化 */
        client_fds[i].fd = -1;
        client_fds[i].events = POLLIN;
    }
    
    int max_fd = 0;
    client_fds[0].fd = lfd;

    char rev_buff[512] = { 0 };
    char ip[64];
    while (1)
    {
        /* 委托检测 */
        int ret = poll(client_fds,max_fd+1,-1);
        assert(ret >= 0);

        if (client_fds[0].revents & POLLIN)
        {
            /* 接收连接 */
            struct sockaddr_in caddr;
            int len = sizeof(caddr);
            int cfd = accept(lfd,(struct sockaddr*)&caddr,&len);

            int i = 1;
            for (; i < 1024; i++)
            {
                if (-1 == client_fds[i].fd)
                {
                    /* 加入 */
                    client_fds[i].fd = cfd;
                    break;
                }
                
            }
            
            max_fd = i > max_fd ? i : max_fd;
            printf("[%s : %d]连接,fd = %d ...\n",
                inet_ntop(AF_INET,&caddr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(caddr.sin_port),cfd);
        }
        
        for (int i = 1; i <= max_fd; i++)
        {
            int fd = client_fds[i].fd;
            if(fd == -1) continue;

            /* 可读缓冲 */
            if(client_fds[i].revents & POLLIN)
            {             
                int len = read(fd,rev_buff,512);
                if(len > 0)
                {
                    printf("Recv:%s,len = %d\n",rev_buff,len);
                    for (char* ptr = rev_buff; *ptr; ptr++) 
                    {  
                        *ptr = toupper(*ptr);  //转大写
                    } 
                    write(fd,rev_buff,len);
                    printf("Send:%s,len = %d\n",rev_buff,len);
                }
                else if(len == 0)
                {        
                    printf("[fd%d]断开连接...\n",fd);
                    close(fd);
                    client_fds[i].fd = -1;
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