#include"http_conn.h"
#include<sys/epoll.h>
#include<assert.h>

#define MAX_CONN 4096
#define SERV_PORT 8000
#define MAX_EVENT_NUMBER 1000


extern void modfd(int epollfd, int fd, int ev);
extern void addfd(int epollfd,int fd,bool one_shot);

int main(int argc,char** argv){
      if(argc>1){
            cout<<" wrong parameters "<<endl;
            exit(1);
      }
      http_conn* users = new http_conn[MAX_CONN];

      int i, listenfd,sockfd;
      ssize_t nready;
      
      listenfd = socket(AF_INET,SOCK_STREAM,0);
      assert(listenfd>=0);
      int opt = 1;
      setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

      int ret = 0;
      struct sockaddr_in cliaddr, servaddr;
      bzero(&servaddr, sizeof(servaddr));
      servaddr.sin_family = AF_INET;
      servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
      servaddr.sin_port = htons(SERV_PORT);

      ret = bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
      assert(ret>=0);

      ret = listen(listenfd, 20);
      assert(ret>=0);

      struct epoll_event events[MAX_EVENT_NUMBER];
      int epollfd = epoll_create(MAX_EVENT_NUMBER);
      assert(epollfd>=0);
      http_conn::m_epollfd = epollfd;

      addfd(epollfd,listenfd,false);

      while(1){
            nready = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,0);
            if(nready==-1){
                  perror("epoll wait failure");
                  exit(1);
            }
            for(int i=0;i<nready;++i){
                  int sockfd = events[i].data.fd;
                  if(sockfd==listenfd && events[i].events&EPOLLIN){
                        socklen_t len = sizeof(cliaddr);
                        int confd = accept(listenfd,(struct sockaddr*)&cliaddr,&len);
                        assert(confd>=0);
                        if(http_conn::m_user_count >= MAX_CONN){
                              cout<<confd<<": INternal server busy!"<<endl;
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
                              users[sockfd].process();   
                        } 
                  }
                  else if(events[i].events&EPOLLOUT){
                        if(!users[sockfd].write()){
                              users[sockfd].close_conn();
                        }
                  }
            }
      }
      close(epollfd);
      close(listenfd);
      delete[] users;
      return 0;
}