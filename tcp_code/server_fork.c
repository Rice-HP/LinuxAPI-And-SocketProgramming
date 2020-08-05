#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define PORT 9999

int user = 0;

void recycle_child(int signo){
    pid_t pid;

    while( (pid = waitpid(0,NULL,WNOHANG)) > 0){
        printf("-------child wait pid : %d exit----------\n",pid);
        user--;
    }

}

void perro_exit(char *str)
{
    printf("%s erro...\n",str);
    perror("erro");
    exit(-1);
}

int main(int argc,char** argv)
{
    //创建socket
    int lfd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //addr.sin_addr.s_addr = htonl("172.18.158.104");  //inet_pton
    addr.sin_port = htons(PORT);

    //绑定
    int b = bind(lfd,(struct sockaddr *)&addr,sizeof(addr));
    if(b == -1)
    {
        perro_exit("bind");
    }

    //监听
    int l = listen(lfd,5);
    if(l == -1)
    {
        perro_exit("listen");
    }

    printf("begin listening...\n");

    //子进程回收信号处理
    /*struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = recycle_child;
    sigemptyset(&act.sa_mask);

    sigaction(SIGCHLD,&act,NULL);*/

    //临时屏蔽sigchld信号
    sigset_t myset;
    sigemptyset(&myset);
    sigaddset(&myset, SIGCHLD);
    // 自定义信号集 -> 内核阻塞信号集
    sigprocmask(SIG_BLOCK, &myset, NULL);

    int cfd;
    struct sockaddr_in client_addr;
    socklen_t cliaddr_len;
    cliaddr_len = sizeof(client_addr);

    while (1)
    {
        /* 接收连接 */
        cfd = accept(lfd,(struct sockaddr * restrict)&client_addr,&cliaddr_len);

        while (cfd == -1 && errno == EINTR)
        {
            cfd = accept(lfd,(struct sockaddr * restrict)&client_addr,&cliaddr_len);
        }
        
        if(cfd == -1)
        {
            perro_exit("accept");
        }

        printf("[%s:%d]连接...当前在线客户端：[%d]\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), ++user);
				
        pid_t pid = fork();

        if(pid == 0)
        {
            //子进程
            close(lfd);
            char rev_buff[512] = { 0 };
            
            while (1)
            {
                /* 通信 */

                int len = read(cfd,rev_buff,sizeof(rev_buff));

                if(len > 0)
                {
                    printf("recv:%s",rev_buff);
                    char* ptr = rev_buff;
                    for (; *ptr; ptr++) {  
                        //*ptr = tolower(*ptr);  //转小写
                        *ptr = toupper(*ptr);  //转大写
                    } 
                    *(ptr-1) = 0; 
                    write(cfd,rev_buff,sizeof(rev_buff));
                }
                else if(len == 0)
                {
                    printf("%d断开连接...\n",getpid());
                    close(cfd);
                    break;
                }
                else
                {
                    /* 出错 */
                    exit(-1);
                }
                

            }
            //退出
            exit(1);
            //return 0;
        }
        else if(pid > 0)
        {
            struct sigaction act;
            act.sa_flags = 0;
            act.sa_handler = recycle_child;
            sigemptyset(&act.sa_mask);
            sigaction(SIGCHLD, &act, NULL);
            // 解除对sigchld信号的屏蔽
            sigprocmask(SIG_UNBLOCK, &myset, NULL);

            close(cfd);
        }
        else
        {
           perro_exit("fork");
        }
        
    }
    
}