#include"webserver.h"
#include<assert.h>

Webserver::Webserver(){
    users = new http_conn[MAX_CONN]();
    //user_timer = new client_data[MAX_CONN];
    if(!users){
        cerr<<" users allocate memory failed "<<endl;
        exit(1);
    }
}

void Webserver::eventListen(){
    m_listenfd = socket(AF_INET,SOCK_STREAM,0);
    http_conn::m_listenfd = m_listenfd;
    assert(m_listenfd>=0);
    int opt = 1;
    setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    int ret = 0;
    ret = bind(m_listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    assert(ret>=0);

    ret = listen(m_listenfd, 20);
    assert(ret>=0);

    struct epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(MAX_EVENT_NUMBER);
    assert(m_epollfd>=0);
    http_conn::m_epollfd = m_epollfd;
    Util::addfd(m_epollfd,m_listenfd,false);

    ret = socketpair(PF_UNIX,SOCK_STREAM,0,pipefd);
    assert(ret!=-1);
    Util::setnonblocking(pipefd[1]);
    Util::addfd(m_epollfd,pipefd[0],false);

    Util::addsig(SIGALRM,Util::sig_handler);
    Util::addsig(SIGTERM,Util::sig_handler);

    Util::u_epollfd = m_epollfd;
    Util::u_pipefd = pipefd;
    Util::u_time_slot = TIMESLOT;
    alarm(TIMESLOT);

}

void Webserver::initThreadpool(){
    m_pool = new threadpool();
    if(!m_pool){
        cerr<<" m_pool allocate memory failed "<<endl;
        delete[] users;
        exit(1);
    }
}

void Webserver::eventLoop(){
    struct sockaddr_in cliaddr;
    bool timeout=false;
    bool stopserver = false;
    while(!stopserver){
            int nready = epoll_wait(m_epollfd,events,MAX_EVENT_NUMBER,0);
            //cout<<"nready: "<<nready<<endl;
            if(nready<0){
                //cout<<"============"<<endl;
                perror("epoll wait failure");
                break;
            }
            for(int i=0;i<nready;++i){
                int sockfd = events[i].data.fd;
                if(sockfd==m_listenfd){
                    socklen_t len = sizeof(cliaddr);
                    int confd = accept(m_listenfd,(struct sockaddr*)&cliaddr,&len);
                    assert(confd>=0);
                    if(Util::m_user_count >= MAX_CONN){
                        cout<<confd<<": Internal server busy!"<<endl;
                        continue;
                    }
                    users[confd].init(confd,cliaddr);
                    init_timer(confd,cliaddr);
                    continue;
                }
                  /*else if(events[i].events&EPOLLIN){
                        bool readres = users[sockfd].read();
                        if(!readres){
                            users[sockfd].close_conn();
                        }
                        else{
                            m_pool->append(users+sockfd);
                        } 
                  }
                  else if(events[i].events&EPOLLOUT){
                        if(!users[sockfd].write()){
                            users[sockfd].close_conn();
                        }
                  }*/
                else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN)){
                    handleSignal(timeout,stopserver);
                }
                else if(events[i].events&EPOLLIN){
                    handleRead(sockfd);
                }
                else if(events[i].events&EPOLLOUT){
                    handleWrite(sockfd);
                }

            }
            if(timeout){
                //cout<<"-----alarm time out-----"<<endl;
                Util::time_handler();
                timeout = false;
            }
      }
      
}

void Webserver::run(){
    initThreadpool();
    eventListen();
    eventLoop();
}

void Webserver::init_timer(int confd, struct sockaddr_in cliaddr){//create timer
    users[confd].cli_data->sockfd = confd;
    users[confd].cli_data->address = cliaddr;
    
    heap_timer* timer = new heap_timer;
    timer->user_data = users[confd].cli_data;
    timer->cb_func = Util::call_back_func;
    timer->expire = time(0)+3*TIMESLOT;
    users[confd].cli_data->h_timer = timer;
    Util::u_time_heap.add_timer(timer);

    /*user_timer[confd].address = cliaddr;
    heap_timer* timer = new heap_timer;
    timer->user_data = &user_timer[confd];
    timer->cb_func = Util::call_back_func;
    time_t curTime = time(0);
    timer->expire = curTime+3*TIMESLOT;
    user_timer[confd].h_timer = timer;
    Util::u_time_heap.add_timer(timer);*/
}

void Webserver::handleSignal(bool& timeout, bool& stopserver){
    int sig;
    char signals[1024];
    int ret = recv(pipefd[0],signals,sizeof(signals),0);
    if(ret==-1 || ret == 0){
        return;
    }
    else{
        for(int i=0;i<ret;++i){
            switch(signals[i]){
                case SIGALRM:
                    timeout = true;
                    break;
                case SIGTERM:
                    stopserver = true;
                    break;
            }
        }
    }
    return;
}

void Webserver::handleRead(int confd){
    //heap_timer* timer = user_timer[confd].h_timer;
    heap_timer* timer = users[confd].cli_data->h_timer;
    if(timer){
        Util::u_time_heap.adjust_timer(timer);//only adjust expire time
    }
    m_pool->append(users+confd,0);
    
    
}

void Webserver::handleWrite(int confd){
    //heap_timer* timer = user_timer[confd].h_timer;
    heap_timer* timer = users[confd].cli_data->h_timer;
    if(timer){
        Util::u_time_heap.adjust_timer(timer);//only adjust expire time
    }
    m_pool->append(users+confd,1);
}