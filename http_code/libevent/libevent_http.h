#ifndef __LIBUFEV_H__
#define __LIBUFEV_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <signal.h>
#include <sys/time.h>

#define PORT 9999

typedef enum {
    false, 
    true 
} bool;

void cb_listener(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* clientaddr, int len, void* ptr);

void cb_read(struct bufferevent* bufev, void* arg);

void cb_write(struct bufferevent* bufev, void* arg);

//事件回调
void cb_event(struct bufferevent* bufev, short events_what, void* arg);

void cb_signal(evutil_socket_t sig, short events_what, void* usr_data);

void send_file(struct bufferevent* bufev, const char* path);

void send_dir(struct bufferevent* bufev, const char* path);

void send_response_head(struct bufferevent* bufev, int req_no, const char* descp, const char* type, long len);

void http_response(struct bufferevent* bufev,const char* method, char* path);

// 通过文件名获取文件的类型
const char* get_file_type(const char *name);

int hexit(char c);

void decode_str(char *to, char *from);

#endif