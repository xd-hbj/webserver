#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include<unistd.h>
#include<stdarg.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/uio.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<string.h>
#include<cstdlib>
#include<cstdio>
#include<iostream>

using namespace std;

class http_conn{
    public:
        static const int FILENAME_LEN = 200;//最长文件名长度
        static const int READ_BUFFER_SIZE = 2048;
        static const int WRITE_BUFFER_SIZE = 2048;

        enum METHOD{GET,POST};

        enum CHECK_STATE{CHECK_STATE_REQUESTLINE, CHECK_STATE_HEADER};//主状态机状态

        enum HTTP_CODE{NO_REQUEST,GET_REQUEST,BAD_REQUEST,NO_RESOURCE,FORBIDDEN_FILEPERMISSION,FILE_REQUEST,
            INTERNAL_ERROR};//请求不完整，得到一个完整的http请求，请求有语法错误

        enum LINE_STATUS{LINE_OK,LINE_BAD,LINE_OPEN};//请求到完整的行，请求行出现错误，请求到行的一部分
    
    public:
        http_conn(){};
        ~http_conn(){};
        /*~http_conn(){
            freeptr(m_url);
            freeptr(m_version);
            freeptr(m_host);
            freeptr(m_file_address);
        };

        void freeptr(void* ptr){
            delete[] ptr;
            ptr = nullptr;
        }*/

        void init(int sockfd, const sockaddr_in& addr);//初始化新接受的链接

        void close_conn(bool real_close=true);//关闭连接
        void process();//处理客户请求
        bool read();//非阻塞读
        bool write();//非阻塞写

        
            void init();//初始化链接
            HTTP_CODE  process_read();//解析http请求
            bool process_write(HTTP_CODE ret);//填充http应答

            /*下面这组函数被process_read()调用以分析http请求*/
            HTTP_CODE  parse_request_line(char* text);
            HTTP_CODE parse_headers(char* text);
            HTTP_CODE parse_content(char* text);
            HTTP_CODE do_request();

            LINE_STATUS parse_line();

            /*下面这组函数被process_write()调用以填充http应答*/
            void unmap();
            bool add_response(const char* format,...);
            bool add_content(const char* content);
            bool add_status_line(int status, const char* title);
            bool add_headers(int content_length);
            bool add_content_length(int content_length);
            bool add_linger();
            bool add_blank_line();

            static int m_epollfd;
            static int m_listenfd;
            static int m_user_count;
            static const char* doc_root;

            private:
                int m_sockfd;
                sockaddr_in m_address;

                char m_read_buf[READ_BUFFER_SIZE];
                int m_read_idx;
                int m_checked_idx;
                int m_start_line;
                int bytes_have_send;
                int bytes_to_send;

                char m_write_buf[WRITE_BUFFER_SIZE];
                int m_write_idx;

                CHECK_STATE m_check_state;
                METHOD m_method;

                char m_real_file[FILENAME_LEN];
                
                char* m_url;//客户请求的目标文件的文件名
                char* m_version;//http版本号
                char* m_host;//主机名
                int m_content_length;//http请求的消息体长度
                bool m_linger = false;//http请求是否要求保持连接

                char* m_file_address;//客户请求的目标文件被mmap到内存中的开始位置

                struct stat m_file_stat;//目标文件状态

                struct iovec m_iv[2];
                int m_iv_count;

};



//extern void addfd(int epollfd,int fd,bool one_shot);

#endif