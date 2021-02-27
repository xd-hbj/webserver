#ifndef LOCK_H
#define LOCK_H

#include<exception>
#include<pthread.h>
#include<semaphore.h>
using namespace std;

class locker{
    public:
        locker(){
            if(pthread_mutex_init(&m_mutex,nullptr)!=0){
                throw exception();
            }
        }
        ~locker(){
            pthread_mutex_destroy(&m_mutex);
        }

        bool lock(){
            return pthread_mutex_lock(&m_mutex)==0;
        }

        bool unlock(){
            return pthread_mutex_unlock(&m_mutex)==0;
        }

    private:
        pthread_mutex_t m_mutex;
};

class sem{
    public:
        sem(){
            if(sem_init(&m_sem,0,0)!=0){
                throw exception();
            }
        }
        ~sem(){
            sem_destroy(&m_sem);
        }

        bool wait(){//-1
            return sem_wait(&m_sem)==0;
        }

        bool post(){//+1
            return sem_post(&m_sem)==0;
        }
    
    private:
        sem_t m_sem;
};

class cond{
    public:
        cond(){
            if(pthread_mutex_init(&m_mutex,nullptr)!=0){
                throw exception();
            }
            if(pthread_cond_init(&m_cond,nullptr)!=0){
                pthread_mutex_destroy(&m_mutex);
                throw exception();
            }
        }
        ~cond(){
            pthread_mutex_destroy(&m_mutex);
            pthread_cond_destroy(&m_cond);
        }

        bool wait_cond(){
            int ret = 0;
            pthread_mutex_lock(&m_mutex);
            ret = pthread_cond_wait(&m_cond,&m_mutex);
            pthread_mutex_unlock(&m_mutex);
            return ret==0;
        }

        bool signal(){
            return pthread_cond_signal(&m_cond)==0;
        }
        
    private:
        pthread_cond_t m_cond;
        pthread_mutex_t m_mutex;
};

#endif