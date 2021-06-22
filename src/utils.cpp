#include"../include/utils.h"

int Util::setnonblocking(int fd){
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

void Util::modfd(int epollfd, int fd, int ev){
    epoll_event event;
    event.data.fd = fd;
    event.events = ev|EPOLLET|EPOLLONESHOT|EPOLLRDHUP;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
}

void Util::removefd(int epollfd,int fd){
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,nullptr);
    --m_user_count;
    close(fd);
}

void Util::addfd(int epollfd,int fd,bool one_shot){
    epoll_event event;
    event.data.fd = fd;
    /*if(http_conn::m_listenfd == fd){
        event.events = EPOLLIN|EPOLLRDHUP;
    }*/
    event.events = EPOLLIN|EPOLLET|EPOLLRDHUP;
    if(one_shot){
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}

void Util::sig_handler(int sig){
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1],(char*)&msg,1,0);
    errno = save_errno;
}

void Util::addsig(int sig, void(*handler)(int)){
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

void Util::time_handler(){
    u_time_heap.tick();
    alarm(u_time_slot);
}

void Util::call_back_func(client_data* user_data){
    removefd(u_epollfd,user_data->sockfd);
}

int* Util::u_pipefd = nullptr;
time_heap Util::u_time_heap = time_heap(10);
time_t Util::u_time_slot = 0;
int Util::u_epollfd = 0;
int Util::m_user_count = 0;