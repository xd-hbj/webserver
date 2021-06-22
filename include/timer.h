#ifndef TIMER_H
#define TIMER_H

#include<iostream>
#include<netinet/in.h>
#include<time.h>
#include"utils.h"
using namespace std;

#define BUFFER_SIZE 64

class Util;
class heap_timer;
struct client_data{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    heap_timer* h_timer;
};

class heap_timer{
    public:
        heap_timer(){}
        heap_timer(int delay){
            expire = time(NULL) + delay;
        }
    
    public:
        time_t expire;
        void (*cb_func)(client_data* );
        client_data* user_data;
};

class time_heap{
    public:
        time_heap();
        time_heap(int cap);
        time_heap(const time_heap& th);

        time_heap(heap_timer** init_array, int size, int _capacity);

        ~time_heap();

        heap_timer** getArray() const {return this->array;}
        int getSize() const { return this->cur_size;}
        int getCapacity() const { return this->capacity;}
        void add_timer(heap_timer* timer);
        void del_timer(heap_timer* timer);
        bool empty(){
            return cur_size==0;
        }
        heap_timer* top();
        void pop_timer();
        void tick();
        int find_timer(heap_timer* timer);
        void adjust_timer(heap_timer* timer);

    private:
        void sink(int index);
        void resize();
        

    private:
        heap_timer** array;
        int capacity;
        int cur_size;
};



#endif