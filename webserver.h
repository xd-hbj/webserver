#ifndef WEBSERVER_H
#define WEBSERVER_H

#include"http_conn.h"
#include"threadpool.h"

#define MAX_CONN 4096
#define SERV_PORT 8000
#define MAX_EVENT_NUMBER 1000

class webserver{
    public:
        webserver(){}
        void eventListen();
        void eventLoop();

    private:
        http_conn* users;
        threadpool<http_conn> *m_pool;

        
        int m_epollfd;
        int m_listenfd;
};

#endif