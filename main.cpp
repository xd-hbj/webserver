#include<sys/epoll.h>
#include<assert.h>
#include"webserver.h"
#include"http_conn.h"


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
      
      Webserver webserver;
      webserver.run();
      return 0;
}