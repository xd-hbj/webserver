#ifndef WEBSERVER_H
#define WEBSERVER_H

#include"http_conn.h"
#include"threadpool.h"


using namespace std;

#define MAX_CONN 4096
#define SERV_PORT 8000
#define MAX_EVENT_NUMBER 1000

extern void deleteAndsetnull(void *ptr);

class Webserver{
    public:
        Webserver();
        ~Webserver(){
            deleteAndsetnull(users);
            deleteAndsetnull(m_pool);
            close(m_listenfd);
            close(m_epollfd);

        };
        void eventListen();
        void eventLoop();
        void initThreadpool();
        void run();

    private:
        http_conn* users;
        threadpool<http_conn> *m_pool;

        epoll_event events[MAX_EVENT_NUMBER];

        int m_epollfd;
        int m_listenfd;
};

#endif