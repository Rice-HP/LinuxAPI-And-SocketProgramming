#include <stdio.h>
#include <stdlib.h>
#include "libevent_http.h"

int main(int argc,char** argv)
{
    if (argc < 2)
    {
        printf("arg: root_path\n");
        exit(-1);
    }
    
    int ret = chdir(argv[1]);
    assert(ret != -1);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    struct event_base* base = NULL;
    base = event_base_new();
    assert(base != NULL);

    struct evconnlistener* listener = NULL;
    listener = evconnlistener_new_bind(
                base,
                cb_listener,
                (void *)base,                                 //cb_listener的传入参数
                LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,   //内部fd断开自动关闭释放与端口复用
                -1,                                          //backlog最大值
                (struct sockaddr*)&servaddr,
                sizeof(servaddr) );
    assert(listener != NULL);

    //创建信号处理函数
    struct event* signal_event = evsignal_new(base, SIGINT, cb_signal, (void *)base);
    if (!signal_event || event_add(signal_event, NULL) < 0)
    {
        /* 发生错误 */
        perror("signal_event erro");
        exit(-1);
    }
    

    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_free(signal_event);
    event_base_free(base);

    printf("------------%s-end-------------\n",__FUNCTION__);

    return 0;
}