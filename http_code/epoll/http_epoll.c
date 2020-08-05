#include "http_epoll.h"

#define MAX_EVENTS 1024
#define MAX_BUFF_SIZE 1024

void epoll_run(int lfd)
{
    //创建epoll事件
    int epollfd = epoll_create(MAX_EVENTS);
    assert(epollfd != -1);

    struct epoll_event events[MAX_EVENTS];

    //ET非阻塞模式
    epoll_addfd(epollfd,lfd,true);

    while (1)
    {
        /* 主循环 */
        /* 委托内核检测,阻塞等待 */ 
        int ret = epoll_wait(epollfd,events,MAX_EVENTS,-1);

        for (int i = 0; i < ret; i++)
        {
            /* 新连接 */
            if (events[i].data.fd == lfd)
            {
                /* 接收处理 */
                do_accept(lfd,epollfd);
            }
            else
            {
                /* 读取请求 */
                do_read(events[i].data.fd,epollfd);
            }
                       
        }
        
    }
    
}

void epoll_addfd(int epollfd,int fd,int enable_et)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;

    /* 注册文件描述符fd至epoll内核事件表,并设置是否为ET模式 */
    if(enable_et)
    {
        event.events |= EPOLLET;
    }

    int ret = epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    assert(ret != -1); 

    set_nonblocking(fd); 
}

int set_nonblocking(int fd)
{
    /* 设置非阻塞 */
    int oldfd = fcntl(fd,F_GETFL);
    int newfd = oldfd | O_NONBLOCK;
    fcntl(fd,F_SETFL,newfd);
    return oldfd;
}

void do_accept(int lfd,int epollfd)
{
    /* 接收连接 */
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    int cfd = accept(lfd,(struct sockaddr*)&caddr,&len);
    assert(cfd != -1); 

    //注册
    epoll_addfd(epollfd,cfd,true);  //开启et模式

    char ip[64];
    printf("[%s : %d]连接,fd = %d ...\n",
        inet_ntop(AF_INET,&caddr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(caddr.sin_port),cfd);
}

void do_read(int client_fd,int epollfd)
{
    char line[MAX_BUFF_SIZE] = {0};
    int len = get_line(client_fd,line,sizeof(line));

    printf("-------Head------%s-------\n",line);

    while (len)
    {
        /* 继续读取 */
        char buff[MAX_BUFF_SIZE];
        len = get_line(client_fd,buff,sizeof(buff));
        printf("-------Head------%s-------\n",buff);
    }

    /* 比较前n个字符,忽略大小写 */
    if (strncasecmp("get",line,3) == 0)
    {
        /* 回复请求后关闭连接 */
        http_get_request(client_fd,line);
        disconnect(client_fd,epollfd);
    }
    
    
}

void http_get_request(int client_fd,char* line)
{
    char mothod[10] = {0};
    char buff[MAX_BUFF_SIZE] = {0};

    //正则表达式匹配获取字段
    sscanf(line,"%[^ ] %[^ ]",mothod,buff);

    //中文转码




    //去掉'/'斜杠
    char* path = buff + 1;
    if(0 == *path)
    {
        sprintf(path,".");
    }
    printf("-------Path------%s-------\n",path);

    struct stat st;
    int ret = stat(path,&st);
    
    if (ret == -1)
    {
        /* 调用错误*/
        perror("stat erro");
        send_response_head(client_fd,404,"ERRO",get_file_type(".html"), 1024);
        send_file(client_fd,"404.html");
        return;
        //exit(-1);
    }
    
    if (S_ISDIR(st.st_mode))
    {
        /* 目录 */
        send_response_head(client_fd,200,"OK",get_file_type(".html"), 99999);
        send_dir(client_fd, path);
    }
    else if (S_ISREG(st.st_mode))
    {
        /* 发送文件 */
        send_response_head(client_fd,200,"OK",get_file_type(path), st.st_size);
        send_file(client_fd,path);
    }

}

void disconnect(int client_fd,int epollfd)
{
    int ret = epoll_ctl(epollfd,EPOLL_CTL_DEL,client_fd,NULL);
    assert(ret != -1); 
    close(client_fd);   
}


void send_response_head(int client_fd,int req_no,const char* descp,const char* type,long len)
{
    char buff[MAX_BUFF_SIZE] = {0};

    //状态行,状态码,描述
    sprintf(buff,"http/1.1 %d %s\r\n",req_no,descp);
    send(client_fd,buff,strlen(buff),0);

    //消息头
    memset(buff,0,MAX_BUFF_SIZE);
    sprintf(buff,"Content-Type:%s\r\n",type);
    sprintf(buff+strlen(buff),"Content-Length:%ld\r\n",len);
    send(client_fd,buff,strlen(buff),0);

    //空行
    send(client_fd,"\r\n",2,0);
}

void send_dir(int client_fd,char* path)
{
    char file[MAX_BUFF_SIZE] = {0};
    char buff[MAX_BUFF_SIZE*4] = {0};

    sprintf(buff,"<html><head><title>curpath-%s</title></head>",path);
    sprintf(buff+strlen(buff),"<body><h1 align=\"center\">Current Directory:%s</h1><center><table>",path);

    struct dirent** file_list = NULL;
    
    int num = scandir(path,&file_list,NULL,alphasort);
    for (int i = 0; i < num; i++)
    {
        char* name = file_list[i]->d_name;
        sprintf(file,"%s/%s",path,name);
        //printf("*********send_dir----------%s/%s********\n",path,name);
        //sprintf(file,"%s",name);
        //printf("*********file----------%s********\n",file);
        struct stat st;
        stat(file,&st);

        // 如果是文件
        if(S_ISREG(st.st_mode))
        {
            sprintf(buff+strlen(buff), 
                "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
                name, name, (long)st.st_size);
        }
        // 如果是目录
        else if(S_ISDIR(st.st_mode))
        {
            sprintf(buff+strlen(buff), 
                "<tr><td><a href=\"%s/\">%s/</a></td><td>%ld</td></tr>",
                name, name, (long)st.st_size);
        }
        send(client_fd, buff, strlen(buff), 0);
        memset(buff, 0, sizeof(buff));
    }

    sprintf(buff+strlen(buff), "</table></center></body></html>");
    send(client_fd, buff, strlen(buff), 0);
}

void send_file(int client_fd,char* path)
{
    printf("-------Open------%s-------\n",path);
    int fd = open(path,O_RDONLY);
    if (fd == -1)
    {
        /* 找不到资源404 */
        perror("open erro");
        fd = open("404.html",O_RDONLY);
        close(fd);
        return;
    }

    char buff[MAX_BUFF_SIZE] = {0};

    int ret = 0;

    while ( (ret = read(fd,buff,sizeof(buff))) > 0 )
    {
        /* 发送文件内容 */
        send(client_fd,buff,ret,0);
    }
    
    close(fd);
}

int get_line(int sock, char *buf, int size)
{
    /* 解析http请求消息的每一行内容 */
    int i = 0;
    char c = '\0';
    int n;
    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0);
        if (n > 0)
        {
            if (c == '\r')
            {
                /* 把tcp 缓冲区中的数据读取到buf中，没有把已读取的数据从tcp 缓冲区中移除 */
                /* 如果再次调用recv()函数仍然可以读到刚才读到的数据 */
                n = recv(sock, &c, 1, MSG_PEEK);

                if ((n > 0) && (c == '\n'))
                {
                    recv(sock, &c, 1, 0);
                }
                else
                {
                    c = '\n';
                }
            }
            buf[i] = c;
            i++;
        }
        else
        {
            c = '\n';
        }
    }

    buf[i] = '\0';

    return i;
}


// 通过文件名获取文件的类型
const char *get_file_type(const char *name)
{
    char* dot;

    // 自右向左查找‘.’字符, 如不存在返回NULL
    dot = strrchr(name, '.');   
    if (dot == NULL) return "text/plain; charset=utf-8";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(dot, ".gif") == 0) return "image/gif";
    if (strcmp(dot, ".png") == 0) return "image/png";
    if (strcmp(dot, ".css") == 0) return "text/css";
    if (strcmp(dot, ".au") == 0) return "audio/basic";
    if (strcmp( dot, ".wav" ) == 0) return "audio/wav";
    if (strcmp(dot, ".avi") == 0) return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0) return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0) return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0) return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0) return "audio/midi";
    if (strcmp(dot, ".mp3") == 0) return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0) return "application/ogg";
    if (strcmp(dot, ".pac") == 0) return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}