#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<queue>
#include<pthread.h>
#include"lock.h"
#include"http_conn.h"
using namespace std;

class threadpool{
    public:
        threadpool(int thread_numer=50,int max_request=10000);
        ~threadpool(){
            delete[] m_threads;
            m_threads = nullptr;
        }
        bool append(http_conn* request,int state);//state: 0-->read,1-->write 
        void run();
        static void* worker(void* arg);

    private:
        int m_thread_number;
        int m_max_request;
        pthread_t* m_threads;
        queue<http_conn*> m_workqueue;
        locker m_queuelocker;
        sem m_queuesem;
        cond m_cond;
};

#endif