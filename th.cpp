#include"threadpool.h"

threadpool::threadpool(int thread_numer,int max_request):
    m_thread_number(thread_numer),m_max_request(max_request),m_threads(nullptr){
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



bool threadpool::append(http_conn* request, int state){
    m_queuelocker.lock();
    if(m_workqueue.size()>=m_max_request){
        m_queuelocker.unlock();
        return false;
    }
    request->m_state = state;
    m_workqueue.push(request);
    m_queuelocker.unlock();
    m_queuesem.post();
    return true;
}

void* threadpool::worker(void* arg){
    threadpool* pool = (threadpool*)arg; 
    pool->run();
    return pool;
}

void threadpool::run(){
    while(true){
        m_queuesem.wait();//block and wait signal
        m_queuelocker.lock();
        /*if(m_workqueue.empty()){//necessary
            m_queuelocker.unlock();
            continue;
        }*/
        http_conn* request = m_workqueue.front();
        /*if(!request){
            cout<<"-----request is nullptr-----------"<<endl;
        }*/
        m_workqueue.pop();
        m_queuelocker.unlock();

        if(request->m_state==0){//work thread read
            bool readres = request->read();//reactor mode
            if(!readres){
                request->close_conn();
            }
            request->process();
        }
        else if(request->m_state==1){//work thread write
            bool writeres = request->write();
            if(!writeres){
                request->close_conn();
            }
        }
    }
}