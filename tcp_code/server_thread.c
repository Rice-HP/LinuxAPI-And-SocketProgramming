#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>

#define PORT 9999
#define MAX_USER 21

int user = 0;

typedef struct sockInfo
{
    pthread_t id; 
    int fd; 
    struct sockaddr_in addr;
}SockInfo;


void* worker(void* arg)
{
    SockInfo* info = (SockInfo*)arg;
    char rev_buff[512] = { 0 };
    char ip[64];

    while (1)
    {
         /* 通信 */
        memset(rev_buff,0,512);
        int len = read(info->fd,rev_buff,512);

        if(len > 0)
        {
            printf("[%s:%d]:%s",
                inet_ntop(AF_INET,&info->addr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(info->addr.sin_port),rev_buff);
            char* ptr = rev_buff;
            for (; *ptr; ptr++) {  
                //*ptr = tolower(*ptr);  //转小写
                *ptr = toupper(*ptr);  //转大写
            } 
            *(ptr-1) = 0; 
            write(info->fd,rev_buff,len);
            printf("send:%s,len = %d\n",rev_buff,len);
        }
        else if(len == 0)
        {        
            printf("[%s : %d]断开连接...\n",
                inet_ntop(AF_INET,&info->addr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(info->addr.sin_port));
            break;
        }
        else
        {
            /* 出错 */
            break;
        }
    }

    close(info->fd);
    info->fd = -1;
    user--;
    pthread_exit(NULL);
    
}


int main(int argc,char** argv)
{
    //创建socket
    int lfd = socket(AF_INET,SOCK_STREAM,0);
    if (lfd < 0)
    {
        perror("socket erro");
        exit(-1);
    }
    
    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    //绑定
    if(-1 == bind(lfd,(struct sockaddr *)&addr,sizeof(addr)))
    {
        perror("bind erro");
        exit(-1);
    }


    //监听
    if(listen(lfd,5) == -1)
    {
        perror("listen erro");
        exit(-1);
    }

    printf("begin listening...\n");

    SockInfo user_sock[MAX_USER];
    int i = 0;
    for (; i < MAX_USER; i++)
    {
        user_sock[i].fd = -1;
    }
    
    int cliaddr_len = sizeof(struct sockaddr_in);
    char ip[64] = {0};
    while (1)
    {
        if(user == MAX_USER)
        {
            printf("客户端超过[%d]...等待5s...\n",MAX_USER);
            sleep(5);
            continue;
        }
        for (i = 0; i < MAX_USER; i++)
        {
            if(user_sock[i].fd == -1)
                break;
        }

        user_sock[i].fd = accept(lfd,(struct sockaddr * restrict)&user_sock[i].addr,&cliaddr_len);
        if(user_sock[i].fd == -1)
        {
            perror("accept erro");
            break;
        }
        pthread_create(&user_sock[i].id,NULL,worker,&user_sock[i]);
        pthread_detach(user_sock[i].id);
        
        printf("[%s : %d]连接,当前在线客户端[%d]\n",
            inet_ntop(AF_INET,&user_sock[i].addr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(user_sock[i].addr.sin_port),++user);
    }
    
    pthread_exit(NULL);
}