#include "libevent_http.h"

#define MAX_BUFF_SIZE 4096

void cb_listener(
        struct evconnlistener* listener, 
        evutil_socket_t fd, 
        struct sockaddr* clientaddr, 
        int len, 
        void* ptr )
{
    char ip[64];
    // struct sockaddr_in* caddr = (struct sockaddr_in*)clientaddr;
    struct sockaddr_in* caddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    memcpy(caddr,clientaddr,sizeof(struct sockaddr_in));
    printf("[%s:%d]connect...\n",
            inet_ntop(AF_INET, &caddr->sin_addr.s_addr, ip, sizeof(ip)),
            ntohs(caddr->sin_port)
            );
    struct event_base* base = (struct event_base*)ptr;

    // 通信操作
    // 添加新事件
    struct bufferevent* bufev = NULL;
    bufev = bufferevent_socket_new(base,fd, BEV_OPT_CLOSE_ON_FREE);
    assert(bufev != NULL);

    //强制发送缓冲区数据
    bufferevent_flush(bufev, EV_READ | EV_WRITE, BEV_NORMAL);
    // 给bufferevent缓冲区设置回调
    bufferevent_setcb(bufev,cb_read,cb_write,cb_event,caddr);
    // 打开默认关闭的读回调使能
    bufferevent_enable(bufev,EV_READ | EV_WRITE);
}



void cb_read(struct bufferevent* bufev, void* arg)
{
    char buff[MAX_BUFF_SIZE] = {0};
    char method[50],path[1024],protocol[32];

    bufferevent_read(bufev,buff,sizeof(buff));
    printf("\n-------Head-Begin--------\n");
    printf("%s",buff);
    printf("-------Head-End--------\n");

    //正则表达式匹配获取字段
    sscanf(buff,"%[^ ] %[^ ] %[^ \r\n]",method,path,protocol);

    // printf("-------method-%s--------\n",method);
    // printf("-------path-%s--------\n",path);
    // printf("-------protocol-%s--------\n",protocol);

    /* 比较忽略大小写 */
    if (strcasecmp("get",method) == 0)
    {
        /* 回复请求后关闭连接 */
        http_response(bufev,method,path);       
    }
    
}

void cb_write(struct bufferevent* bufev, void* arg)
{
    char ip[64];
    struct sockaddr_in * caddr = (struct sockaddr_in*)arg;
    printf("[%s:%d] %s called\n",
            inet_ntop(AF_INET, &caddr->sin_addr.s_addr, ip, sizeof(ip)),
            ntohs(caddr->sin_port),
            __FUNCTION__
          );

}

//事件回调
void cb_event(struct bufferevent* bufev, short events_what, void* arg)
{
    if (events_what | BEV_EVENT_EOF)
    {
        /* 关闭连接 */
        char ip[64];
        struct sockaddr_in * caddr = (struct sockaddr_in*)arg;
        printf("[%s:%d] connection closed\n",
                inet_ntop(AF_INET, &caddr->sin_addr.s_addr, ip, sizeof(ip)),
                ntohs(caddr->sin_port)
              );  
    }
    else if (events_what | BEV_EVENT_ERROR)
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

//信号处理回调
void cb_signal(evutil_socket_t sig, short events_what, void* usr_data)
{

    printf("------------%s-catch-signo : %d-----------\n",__FUNCTION__,sig);
    //退出主循环
    
    // int event_base_loopexit(struct event_base *, const struct timeval *);
    // int event_base_loopbreak(struct event_base *);

    event_base_loopbreak((struct event_base *)usr_data);

}

void http_response(struct bufferevent* bufev, const char* method, char* path)
{
    printf("\n------------%s-begin-------------\n",method);

    //解码 http传输中文时会拆分编码
    char destr_path[1024] = {0};
    decode_str(destr_path, path);
    
    // 去掉path中的/
    char* file_path = destr_path + 1;
    
    if(0 == *file_path)
    {
        sprintf(file_path,".");
    }

    struct stat st;
    int ret = stat(file_path,&st);
    
    if (ret == -1)
    {
        /* 调用错误,文件打开失败*/
        perror("stat erro");
        send_response_head(bufev,404,"ERRO",get_file_type(".html"), 1024);
        send_file(bufev,"404.html");
        return;
    }
    
    if (S_ISDIR(st.st_mode))
    {
        /* 目录 */
        send_response_head(bufev,200,"OK",get_file_type(".html"), 99999);
        send_dir(bufev, file_path);
    }
    else if (S_ISREG(st.st_mode))
    {
        /* 发送文件 */
        send_response_head(bufev,200,"OK",get_file_type(file_path), st.st_size);
        send_file(bufev,file_path);
    }
}

void send_response_head(struct bufferevent* bufev, int req_no, const char* descp, const char* type, long len)
{
    char buff[MAX_BUFF_SIZE] = {0};

    //状态行,状态码,描述
    sprintf(buff,"http/1.1 %d %s\r\n",req_no,descp);
    bufferevent_write(bufev,buff,strlen(buff));

    //消息头
    memset(buff,0,MAX_BUFF_SIZE);
    sprintf(buff,"Content-Type:%s\r\n",type);
    sprintf(buff+strlen(buff),"Content-Length:%ld\r\n",len);
    //主动关闭,保持连接Keep-Alive
    sprintf(buff+strlen(buff),"Connection: close\r\n");
    bufferevent_write(bufev,buff,strlen(buff));

    //空行
    bufferevent_write(bufev,"\r\n",2);
}

void send_file(struct bufferevent* bufev, const char* path)
{
    printf("\n-------Open-File---%s-------\n",path);
    int fd = open(path,O_RDONLY);
    if (fd == -1)
    {
        /* 找不到资源404 */
        perror("open erro");
        return;
    }

    char buff[MAX_BUFF_SIZE] = {0};

    int ret = 0;

    while ( (ret = read(fd,buff,sizeof(buff))) > 0 )
    {
        /* 发送文件内容 */
        bufferevent_write(bufev,buff,ret);
    }
    
    close(fd);
}

void send_dir(struct bufferevent* bufev, const char* path)
{
    char file[MAX_BUFF_SIZE] = {0};
    char buff[MAX_BUFF_SIZE] = {0};

    sprintf(buff,"<html><head><title>curpath-%s</title></head>",path);
    sprintf(buff+strlen(buff),"<body><h1 align=\"center\">Current Directory:%s</h1><center><table border>",path);

    struct dirent** file_list = NULL;
    
    int num = scandir(path,&file_list,NULL,alphasort);

    int i;
    for (i = 0; i < num; i++)
    {
        char* name = file_list[i]->d_name;
        sprintf(file,"%s/%s",path,name);

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
        bufferevent_write(bufev, buff, strlen(buff));
        memset(buff, 0, sizeof(buff));

        free(file_list[i]);
    }

    sprintf(buff+strlen(buff), "</table></center></body></html>");
    bufferevent_write(bufev, buff, strlen(buff)); 

    free(file_list);
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

// 16进制数转化为10进制
int hexit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

void decode_str(char *to, char *from)
{
    for ( ; *from != '\0'; ++to, ++from  ) 
    {
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) 
        { 

            *to = hexit(from[1])*16 + hexit(from[2]);
            from += 2;                      
        } 
        else
        {
            *to = *from;
        }

    }
    *to = '\0';
}