#include"webserver.h"
#include<assert.h>

extern void modfd(int epollfd, int fd, int ev);
extern void addfd(int epollfd,int fd,bool one_shot);

void deleteAndsetnull(void *ptr){
    if(ptr){
        delete[] ptr;
        ptr = nullptr;
    }
}

Webserver::Webserver(){
    users = new http_conn[MAX_CONN];
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
    addfd(m_epollfd,m_listenfd,false);
}

void Webserver::initThreadpool(){
    m_pool = new threadpool<http_conn>();
    if(!m_pool){
        cerr<<" m_pool allocate memory failed "<<endl;
        delete[] users;
        exit(1);
    }
}

void Webserver::eventLoop(){
    struct sockaddr_in cliaddr;
    while(1){
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
                        if(http_conn::m_user_count >= MAX_CONN){
                            cout<<confd<<": Internal server busy!"<<endl;
                            continue;
                        }
                        users[confd].init(confd,cliaddr);
                        continue;
                  }
                  else if(events[i].events&EPOLLIN){
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
                  }
            }
      }
      
}

void Webserver::run(){
    initThreadpool();
    eventListen();
    eventLoop();
}