#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

void cb_read(struct bufferevent *bufev, void *arg)
{
    char rev_buff[1024] = {0}; 
    int len = bufferevent_read(bufev,rev_buff,sizeof(rev_buff));

    printf("Recv:%s,len = %d\n",rev_buff,len);
    for (char* ptr = rev_buff; *ptr; ptr++) 
    {  
        *ptr = toupper(*ptr);  //转大写
    } 
    bufferevent_write(bufev,rev_buff,len+1);
    printf("Send:%s,len = %d\n",rev_buff,len);

}

void cb_write(struct bufferevent *bufev, void *arg)
{
    printf("------------cd_write called------------\n");
    char ip[64];
    struct sockaddr_in * clientaddr = (struct sockaddr_in*)arg;
    printf("[%s:%d] write end~\n",
            inet_ntop(AF_INET,&clientaddr->sin_addr.s_addr,ip,sizeof(ip)),
            ntohs(clientaddr->sin_port)
            ); 
    printf("------------cd_write called------------\n");
}

//事件回调
void cb_event(struct bufferevent *bufev, short events, void *arg)
{
    if (events | BEV_EVENT_EOF)
    {
        /* 关闭连接 */
        printf("connection closed\n");  
    }
    else if (events | BEV_EVENT_ERROR)
    {
        /* 错误 */
        printf("some other error\n");
    }

    if (NULL != arg)
    {
        /* 释放 */
        free(arg);
    }
    
    bufferevent_free(bufev);    
    printf("buffevent free...\n");  
}

void cb_listener(
        struct evconnlistener *listener, 
        evutil_socket_t fd, 
        struct sockaddr *addr, 
        int len, void *ptr)
{
    char ip[64];
    //struct sockaddr_in * clientaddr = (struct sockaddr_in*)addr;
    struct sockaddr_in * clientaddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    memcpy(clientaddr,addr,sizeof(struct sockaddr_in));
    printf("[%s:%d]connect...\n",
            inet_ntop(AF_INET,&clientaddr->sin_addr.s_addr,ip,sizeof(ip)),
            ntohs(clientaddr->sin_port)
            );
    struct event_base* base = (struct event_base*)ptr;

    // 通信操作
    // 添加新事件
    struct bufferevent* bufev = NULL;
    bufev = bufferevent_socket_new(base,fd, BEV_OPT_CLOSE_ON_FREE);

    // 给bufferevent缓冲区设置回调
    bufferevent_setcb(bufev,cb_read,cb_write,cb_event,clientaddr);
    // 打开默认关闭的读回调使能
    bufferevent_enable(bufev,EV_READ);
}

int main(int argc, const char* argv[])
{
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(9999);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    struct event_base* base = NULL;
    base = event_base_new();

    // 创建套接字
    // 绑定
    // 接收连接请求
    struct evconnlistener* listener = NULL;
    listener = evconnlistener_new_bind(
                base,
                cb_listener,
                base,
                LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                5,
                (struct sockaddr*)&serv,
                sizeof(serv));
    
    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_base_free(base);

    return 0;
}
