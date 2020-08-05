#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>


#define MAX_EVENT_NUM 21
#define BUFF_SIZE 6
#define PORT 9999

typedef enum {
    false, 
    true 
    } bool;
/* 设置文件描述符非阻塞,返回旧描述符 */

int set_nonblocking(int fd)
{
    int oldfd = fcntl(fd,F_GETFL);
    int newfd = oldfd | O_NONBLOCK;
    fcntl(fd,F_SETFL,newfd);
    return oldfd;
}

/* 注册文件描述符fd至epoll内核事件表,并设置是否为ET模式 */

void epoll_addfd(int epollfd,int fd,bool enable_et)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(enable_et)
    {
        event.events |= EPOLLET;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    set_nonblocking(fd);
}


/* lt工作模式 */

void lt_work(struct epoll_event* events,int events_num,int epollfd,int listenfd)
{
    char rev_buff[BUFF_SIZE] = { 0 };
    for (int i = 0; i < events_num; i++)
    {
        int fd = events[i].data.fd;
        /* 非连接 */
        if (fd == listenfd)
        {
            /* 接收连接 */
            struct sockaddr_in caddr;
            int len = sizeof(caddr);
            int cfd = accept(fd,(struct sockaddr*)&caddr,&len);
            //注册
            epoll_addfd(epollfd,cfd,false);

            char ip[64];
            printf("[%s : %d]连接,fd = %d ...\n",
                inet_ntop(AF_INET,&caddr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(caddr.sin_port),cfd);
        }
        else if (events[i].events & EPOLLIN)
        {
            /* 读,lt若缓冲区为读取完毕,则下次继续触发wait读取 */
            int len = read(fd,rev_buff,BUFF_SIZE-1);
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
                epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,NULL);
                close(fd);
            }
            else
            {
                /* 出错 */
                perror("lt erro");
                exit(-1);
            }
        }
        else
        {
           printf("[fd%d]发生了其他事件...\n",fd);
        }
      
    }
    
}

/* et工作模式 */

void et_work(struct epoll_event* events,int events_num,int epollfd,int listenfd)
{
    char rev_buff[BUFF_SIZE] = { 0 };
    for (int i = 0; i < events_num; i++)
    {
        int fd = events[i].data.fd;
        /* 非连接 */
        if (fd == listenfd)
        {
            /* 接收连接 */
            struct sockaddr_in caddr;
            socklen_t len = sizeof(caddr);
            int cfd = accept(fd,(struct sockaddr*)&caddr,&len);
            //注册
            epoll_addfd(epollfd,cfd,true);  //开启et模式

            char ip[64];
            printf("[%s : %d]连接,fd = %d ...\n",
                inet_ntop(AF_INET,&caddr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(caddr.sin_port),cfd);
        }
        else if (events[i].events & EPOLLIN)
        {
            /* 读,et模式下这段不会被重复触发wait,所以循环读取 */

            printf("[fd%d]ET开始读取...\n",fd);
            int total_len = 0;
            int len = 0;
            while ( (len = recv(fd,rev_buff,BUFF_SIZE-1,0)) > 0)
            {       
                total_len += len;
                printf("Recv:%s,len = %d\n",rev_buff,len);
                for (char* ptr = rev_buff; *ptr; ptr++) 
                {  
                    *ptr = toupper(*ptr);  //转大写
                } 
                write(fd,rev_buff,len);
                printf("Send:%s,len = %d\n",rev_buff,len);
            }                        

            if(len == 0)
            {                                                      
                printf("[fd%d]断开连接...\n",fd);
                epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,NULL);
                close(fd);       
            }
            else if(len == -1)
            {
                if(errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
                {
                    printf("[fd%d]ET结束读取%d字节...\n",fd,total_len);
                }
                else
                {                                      
                    /* 出错 */
                    perror("连接已断开");
                    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,NULL);
                    close(fd); 
                }

            }       
        }
        else
        {
           printf("[fd%d]发生了其他事件...\n",fd);
        }

      
    }
}


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

    //创建epoll事件
    int epollfd = epoll_create(5);
    assert(epollfd != -1);

    struct epoll_event events[MAX_EVENT_NUM];
    epoll_addfd(epollfd,lfd,true);
    
    while (1)
    {
        /* 委托内核检测,阻塞等待 */     
        int ret = epoll_wait(epollfd,events,MAX_EVENT_NUM,-1);
        assert(ret != -1);

        if (argc == 1)
        {
            /* 无参则默认lt */
            lt_work(events,ret,epollfd,lfd);
        }
        else
        {
            /* et */
            et_work(events,ret,epollfd,lfd);
        }
        
        
    }
    
    close(lfd);
    
    return 0;
}
