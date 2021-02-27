#include"threadpool.h"
#include"http_conn.h"

template<typename T>
threadpool<T>::threadpool(int thread_numer=10,int max_request=1000):
    m_thread_number(thread_numer),m_max_request(max_request),m_stop(false),m_threads(nullptr){
        m_threads = new thread_t[thread_numer];
        if(!m_threads){
             throw exception();
        }
        for(int i=0;i<thread_numer;++i){
            if(pthread_create(m_threads+i,NULL,worker,this)!=0){
                delete[] m_threads;
                m_threads = nullptr;
                throw exception();   
            }
            if(pthread_detach(m_threads[i])=!0){
                delete[] m_threads;
                m_threads = nullptr;
                throw exception();
            }
        }
}

template<typename T>
threadpool<T>::~threadpool(){
    if(m_threads){
        delete[] m_threads;
        m_threads = nullptr;
    } 
}

template<typename T>
bool threadpool<T>::append(T* request){
    m_queuelcker.lock();
    if(m_workqueue.size()>=m_max_request){
        m_queuelcker.unlock();
        return false;
    }
    m_workqueue.push(request);
    m_queuelcker.unlock();
    m_queuesem.post();
    return true;
}

template<typename T>
void* threadpool<T>::worker(void* arg){
    this->run();
    return this;
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