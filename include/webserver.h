#ifndef WEBSERVER_H
#define WEBSERVER_H

#include"http_conn.h"
#include"threadpool.h"
#include"utils.h"
using namespace std;

#define MAX_CONN 65536
#define SERV_PORT 8000
#define MAX_EVENT_NUMBER 10000
const int TIMESLOT=5;



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

        void init_timer(int confd,struct sockaddr_in addr);
        void handleSignal(bool& timeout, bool& stopserver);
        void handleRead(int confd);
        void handleWrite(int confd);
        void adjust_timer(heap_timer* timer);

        int pipefd[2];
        //client_data* user_timer;
        //Util util;

    private:
        http_conn* users;
        threadpool *m_pool;

        epoll_event events[MAX_EVENT_NUMBER];

        int m_epollfd;
        int m_listenfd;
};

#endif