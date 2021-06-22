#include<sys/epoll.h>
#include<assert.h>
#include"webserver.h"
#include"http_conn.h"

int main(int argc,char** argv){
      if(argc>1){
            cout<<" wrong parameters "<<endl;
            exit(1);
      }
      
      Webserver webserver;
      webserver.run();
      return 0;
}