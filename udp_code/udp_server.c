#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>
 #include <ctype.h>

#define PORT 9999

int main(int argc,char** argv)
{
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    assert(sockfd > 0);

    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    int ret = bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    if(ret == -1)
    {
        perror("bind error");
        exit(1);
    }

    printf("udp begin receiving...\n");

    struct sockaddr_in client_addr;
    socklen_t cli_len = sizeof(client_addr);

    char recv_buff[1024];
    char ip[64];

    while (1)
    {
        /* 接收报文 */
        int len = recvfrom(sockfd,recv_buff,1024,0,(struct sockaddr*)&client_addr,&cli_len);
        assert(len > 0);

        printf("[Recv: %s:%d]:%s",
                inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(client_addr.sin_port),recv_buff);
            
         for (char* ptr = recv_buff; *ptr; ptr++) {  
            *ptr = toupper(*ptr);  //转大写
        } 

        /* 发送报文 */
        len = sendto(sockfd,recv_buff,len,0,(struct sockaddr*)&client_addr,cli_len);
        assert(len > 0);

        printf("[Send: %s:%d]:%s,len = %d\n",ip,ntohs(client_addr.sin_port),recv_buff,len);
    }
    
}
