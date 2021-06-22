#ifndef UTIL_H
#define UTIL_H

#include<unistd.h>
#include<assert.h>
#include<stdarg.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/uio.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<string.h>
#include<cstdlib>
#include<cstdio>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include<iostream>
#include"timer.h"

class time_heap;
class Util{
    public:
        static int setnonblocking(int fd);
        static void modfd(int epollfd, int fd, int ev);
        static void removefd(int epollfd,int fd);
        static void addfd(int epollfd,int fd,bool one_shot);

        static void sig_handler(int sig);
        static void addsig(int sig,void(*handler)(int));

        static void time_handler();
        static void call_back_func(struct client_data* user_data);

    public:
        static int *u_pipefd;
        static time_heap u_time_heap;
        static time_t u_time_slot;
        static int u_epollfd;
        static int m_user_count;
};

#endif