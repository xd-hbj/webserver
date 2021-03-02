#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<queue>
#include<pthread.h>
#include"lock.h"
using namespace std;

extern void deleteAndsetnull(void *ptr);

template<typename T>
class threadpool{
    public:
        threadpool(int thread_numer=10,int max_request=1000);
        ~threadpool(){
            deleteAndsetnull(m_threads);
        }
        bool append(T* request);
        void run();
        static void* worker(void* arg);

    private:
        int m_thread_number;
        int m_max_request;
        pthread_t* m_threads;
        queue<T*> m_workqueue;
        locker m_queuelocker;
        sem m_queuesem;
        cond m_cond;
        bool m_stop;//if stop thread?
};



template<typename T>
threadpool<T>::threadpool(int thread_numer,int max_request):
    m_thread_number(thread_numer),m_max_request(max_request),m_stop(false),m_threads(nullptr){
        m_threads = new pthread_t[thread_numer];
        if(!m_threads){
             throw exception();
        }
        for(int i=0;i<thread_numer;++i){
            if(pthread_create(m_threads+i,NULL,this->worker,this)!=0){
                delete[] m_threads;
                m_threads = nullptr;
                throw exception();   
            }
            if(pthread_detach(m_threads[i])!=0){
                delete[] m_threads;
                m_threads = nullptr;
                throw exception();
            }
        }
}


template<typename T>
bool threadpool<T>::append(T* request){
    m_queuelocker.lock();
    if(m_workqueue.size()>=m_max_request){
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push(request);
    m_queuelocker.unlock();
    m_queuesem.post();
    return true;
}

template<typename T>
void* threadpool<T>::worker(void* arg){
    threadpool* pool = (threadpool*)arg; 
    pool->run();
    return pool;
}

template<typename T>
void threadpool<T>::run(){
    while(!m_stop){
        m_queuesem.wait();//block and wait signal
        m_queuelocker.lock();
        if(m_workqueue.empty()){//necessary
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop();
        m_queuelocker.unlock();
        request->process();
    }
}

#endif