#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<queue>
#include"lock.h"
using namespace std;

template<typename T>
class threadpool{
    public:
        threadpool(int thread_numer=10,int max_request=1000);
        ~threadpool(){}
        bool append(T* request);
        void run();
        void* worker(void* arg);

    private:
        int m_thread_number;
        int m_max_request;
        T* m_threads;
        queue<T*> m_workqueue;
        bool m_stop;
        locker m_queuelocker;
        sem m_queuesem;
        cond m_cond;
        bool m_stop//if stop thread?
};


#endif