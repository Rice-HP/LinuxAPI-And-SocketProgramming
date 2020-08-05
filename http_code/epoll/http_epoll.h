#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <dirent.h>

typedef enum {
    false, 
    true 
} bool;

void epoll_run(int lfd);

void epoll_addfd(int epollfd,int fd,int enable_et);

int set_nonblocking(int fd);

void do_accept(int lfd,int epollfd);

void do_read(int client_fd,int epollfd);

void http_get_request(int client_fd,char* line);

void send_response_head(int client_fd,int req_no,const char* descp,const char* type,long len);

void send_dir(int client_fd,char* path);

void send_file(int client_fd,char* path);

void disconnect(int client_fd,int epollfd);

int get_line(int sock, char *buf, int size);

const char *get_file_type(const char *name);