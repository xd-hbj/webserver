#include"../include/timer.h"

time_heap::time_heap(int cap):capacity(cap),cur_size(0){
    array = new heap_timer*[capacity]();
    if(!array){
        cerr<<"----init time_heap failed!----"<<endl;
        exit(1);
    }
    for(int i=0;i<capacity;++i){
        array[i] = nullptr;
    }
}

time_heap::time_heap(heap_timer** init_array, int size, int _capacity): cur_size(size),capacity(_capacity){
    if(capacity<size){
        cerr<<"----time_heap capacity < size!----"<<endl;
        exit(1);
    }
    array = new heap_timer*[capacity];
    if(!array){
        cerr<<"----init time_heap failed!----"<<endl;
        exit(1);
    }
    for(int i=0;i<capacity;++i){
        array[i] = nullptr;
    }

    if(cur_size>0){
        for(int i=0;i<cur_size;++i){
            array[i] = init_array[i];
        }
        for(int i=(cur_size-1)/2;i>=0;--i){
            sink(i);
        }
    }
}

time_heap::time_heap(){
    this->array = nullptr;
    this->capacity = 0;
    this->cur_size = 0;
};

time_heap::time_heap(const time_heap& th){
    time_heap(th.getArray(),th.getSize(),th.getCapacity());
}

time_heap::~time_heap(){
    for(int i=0;i<cur_size;++i){
        delete array[i];
    }
    delete[] array;
}

void time_heap::add_timer(heap_timer* timer){
    if(!timer) return;
    if(cur_size >= capacity){
        resize();
    }

    int hole = cur_size++;
    int parent = 0;
    for(;hole>0;hole=parent){
        parent = (hole-1)/2;
        if(array[parent]->expire <= timer->expire){
            break;
        }
        array[hole] = array[parent];
    }
    array[hole] = timer;
}

void time_heap::del_timer(heap_timer* timer){
    if(!timer) return;
    timer->cb_func = nullptr;//lazy remove
}

heap_timer* time_heap::top(){
    if(empty()){
        return nullptr;
    }
    return array[0];
}

void time_heap::pop_timer(){
    if(empty()){
        return;
    }
    if(array[0]){
        delete array[0];
        array[0] = array[--cur_size];
        sink(0);
    }
}

void time_heap::tick(){
    heap_timer* tmp = array[0];
    time_t cur = time(0);
    while(!empty()){
        if(!tmp) break;
        if(tmp->expire > cur){
            break;
        }
        if(tmp->cb_func){
            tmp->cb_func(tmp->user_data);
        }
        pop_timer();
        tmp = array[0];
    }
}

void time_heap::sink(int index){
    heap_timer* tmp = array[index];
    int child = 0;
    for(;((index*2+1)<=(cur_size-1));index=child){
        child = 2*index+1;
        if(child+1<=cur_size-1 && array[child]->expire>array[child+1]->expire){//rigtchild larger
            ++child;
        }
        if(tmp->expire>array[child]->expire){
            array[index] = array[child];
        }
        else{
            break;
        }
    }
    array[index] = tmp;
}

void time_heap::resize(){
    int new_capacity = 2*capacity+1;
    heap_timer** tmp = new heap_timer*[new_capacity];
    if(!tmp){
        cerr<<"----time_heap resize failed!----"<<endl;
        exit(1);
    }
    for(int i=0;i<new_capacity;++i){
        tmp[i] = nullptr;
    }
    this->capacity = new_capacity;
    for(int i=0;i<(this->capacity);++i){
        tmp[i] = this->array[i];
        this->array[i] = nullptr;
    }
    delete[] array;
    this->array = tmp;
}

int time_heap::find_timer(heap_timer* timer){
    int pos = -1;
    for(int i=0;i<cur_size;++i){
        if(array[i]==timer) return i;
    }
    return -1;
}


void time_heap::adjust_timer(heap_timer* timer){
    int index = find_timer(timer);
    timer->expire = timer->expire + 3*(Util::u_time_slot);
    sink(index);
}