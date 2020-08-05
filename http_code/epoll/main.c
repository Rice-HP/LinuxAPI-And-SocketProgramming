#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "http_epoll.h"

#define PORT 9999

int main(int argc,char** argv)
{
    if (argc < 2)
    {
        printf("arg: root_path\n");
        exit(-1);
    }

    int ret = chdir(argv[1]);
    assert(ret != -1);

    int lfd = socket(AF_INET,SOCK_STREAM,0);
    assert(lfd != -1);

    //端口复用
    int mw_optval = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR,
		(char *)&mw_optval, sizeof(mw_optval));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(lfd,(struct sockaddr*)&addr,sizeof(addr));
    assert(ret != -1); 

    listen(lfd,64);  
    printf("begin listening...\n");

    epoll_run(lfd);

    close(lfd);

    return 0;
}