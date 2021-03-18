#ifndef WEBSERVER_H
#define WEBSERVER_H

#include"http_conn.h"
#include"threadpool.h"


using namespace std;

#define MAX_CONN 4096
#define SERV_PORT 8000
#define MAX_EVENT_NUMBER 1000

class Webserver{
    public:
        Webserver();
        ~Webserver(){
            delete[] users;
            users = nullptr;
            delete[] m_pool;
            m_pool = nullptr;
            close(m_listenfd);
            close(m_epollfd);

        };
        void eventListen();
        void eventLoop();
        void initThreadpool();
        void run();

    private:
        http_conn* users;
        threadpool *m_pool;

        epoll_event events[MAX_EVENT_NUMBER];

        int m_epollfd;
        int m_listenfd;
};

#endif