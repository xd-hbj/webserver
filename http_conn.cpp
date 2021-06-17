#include"http_conn.h"
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get the file from the server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n";

const char* doc_root = "/home/hbj/code/hbj_webserver/root";

int setnonblocking(int fd){
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

void modfd(int epollfd, int fd, int ev){
    epoll_event event;
    event.data.fd = fd;
    event.events = ev|EPOLLET|EPOLLONESHOT|EPOLLRDHUP;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
}

void removefd(int epollfd,int fd){
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,nullptr);
    close(fd);
}

void addfd(int epollfd,int fd,bool one_shot){
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

http_conn::LINE_STATUS  http_conn::parse_line(){
    char temp;
    for(;m_checked_idx<m_read_idx;++m_checked_idx){
        temp = m_read_buf[m_checked_idx];
        if(temp=='\r'){
            if(m_checked_idx+1==m_read_idx){
                return LINE_OPEN;
            }
            if(m_read_buf[m_checked_idx+1]=='\n'){
                m_read_buf[m_checked_idx++]='\0';
                m_read_buf[m_checked_idx++]='\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }

        else if(temp=='\n'){
            if(m_checked_idx>1 && m_read_buf[m_checked_idx-1]=='\r'){
                m_read_buf[m_checked_idx-1]='\0';
                m_read_buf[m_checked_idx++]='\0';
                return LINE_OK;
            }
            else return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

http_conn::HTTP_CODE http_conn::parse_request_line(char* text){
    m_url = strpbrk(text," \t");
    if(!m_url){
        return BAD_REQUEST;
    }

    *m_url = '\0';
    ++m_url;

    char* method = text;
    if(strcasecmp(method,"get")!=0){//仅支持get方法
        //cout<<"THE REQUEST METHOD IS NOT GET"<<endl;
        return BAD_REQUEST;
    }
    m_method = GET;
    m_url += strspn(m_url," \t");

    m_version = strpbrk(m_url," \t");
    if(!m_version){
        return BAD_REQUEST;
    }
    *m_version = '\0';
    ++m_version;
    m_version += strspn(m_version," \t");
    if(strcasecmp(m_version,"http/1.1")!=0){//仅支持http/1.1
        return BAD_REQUEST;
    }

    /*检查url是否合法*/
    if(strncasecmp(m_url,"http://",7)==0){
        m_url += 7;
        m_url = strchr(m_url,'/');
    }
    if(!m_url || m_url[0]!='/'){
        return BAD_REQUEST;
    }

    //cout<<"THE REQUEST URL IS: "<<m_url<<endl;
    /*http请求行处理���毕*/
    m_check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;

}

http_conn::HTTP_CODE http_conn::parse_headers(char* temp){
    if(temp[0]=='\0'){
        return GET_REQUEST;
    }
    else if(strncasecmp(temp,"Host:",5)==0){
        temp += 5;
        temp += strspn(temp," \t");
        //cout<<" the request host is: "<<temp<<endl;
    }
    else if(strncasecmp(temp,"connection:",11)==0){
        temp += 11;
        temp += strspn(temp," \t");
        if(strcasecmp(temp,"keep-alive")==0){
            m_linger = true;
            //cout<<"---long connection----"<<endl;
        }
    }
    /*else{
        cout<<"we do not handle this header"<<endl;
    }*/
    return NO_REQUEST;
}



http_conn::HTTP_CODE http_conn::do_request(){
    strcpy(m_real_file,doc_root);
    int len = strlen(doc_root);
    strncpy(m_real_file+len,m_url,FILENAME_LEN-len);
    //cout<<"m_real_file:  "<<m_real_file<<endl;

    if(stat(m_real_file,&m_file_stat)<0){
        return NO_RESOURCE;
    }
    if(!(m_file_stat.st_mode&S_IROTH)){
        return FORBIDDEN_FILEPERMISSION;
    }
    if( S_ISDIR(m_file_stat.st_mode) ){
        return BAD_REQUEST;
    }

    int fd = open(m_real_file,O_RDONLY);
    if(fd<0){
        cout<<"http_conn.cpp line 142: open file failed!"<<endl;
        exit(1);
    }
    m_file_address = (char*)mmap(NULL,m_file_stat.st_size,PROT_READ,MAP_PRIVATE,fd,0);
    /*cout<<"==============="<<endl;
    if((void*)m_file_address==MAP_FAILED) cout<<"mmap failed"<<endl;*/
    close(fd);
    return FILE_REQUEST;
}


bool http_conn::read(){
    if(m_read_idx>=READ_BUFFER_SIZE){
        return false;
    }
    int byte_read = 0;
    while (true)
    {

        byte_read = recv(m_sockfd,m_read_buf+m_read_idx,READ_BUFFER_SIZE-m_read_idx,0);
        //cout<<"byte_read: "<<byte_read<<endl;

        if(byte_read==-1){
            if(errno==EAGAIN || errno==EWOULDBLOCK){
                break;
            }
            return false;
        }
        else if(byte_read==0){//???????EOF
            return false;
        }
        m_read_idx += byte_read;
    }
    return true;
    
}

void http_conn::unmap(){
    if(m_file_address){
        munmap(m_file_address,m_file_stat.st_size);
        m_file_address = nullptr;
    }
}

bool http_conn::write(){
    int temp = 0;
    if(bytes_to_send==0){
        modfd(m_epollfd,m_sockfd,EPOLLIN);
        init();
        return true;
    }

    while(1){
        
        temp = writev(m_sockfd,m_iv,m_iv_count);
        if(temp==-1){
            if(errno==EAGAIN){
                modfd(m_epollfd,m_sockfd,EPOLLOUT);
                break;
            }
            unmap();
            return false;
        }

        bytes_have_send += temp;
        bytes_to_send -= temp;

        if(bytes_have_send >= m_write_idx){
            m_iv[0].iov_len = 0;
            m_iv[1].iov_base = m_file_address+(bytes_have_send-m_write_idx);
            m_iv[1].iov_len = bytes_to_send;
        }
        else{
            m_iv[0].iov_len = m_write_idx-bytes_have_send;
            m_iv[0].iov_base = m_write_buf+bytes_have_send;
        }

        //cout<<"bytes_to_send: "<<bytes_to_send<<endl;
        if(bytes_to_send==0){
            unmap();
            //cout<<"mlinger: "<<m_linger<<endl;
            if(m_linger){
                init();
                modfd(m_epollfd,m_sockfd,EPOLLIN);
                return true;
            }
            else return false;
        }

    }
    return true;
}

bool http_conn::add_response(const char* format,...){
    if(m_write_idx>=WRITE_BUFFER_SIZE){
        return false;
    }
    va_list arglist;
    va_start(arglist,format);
    int len = vsnprintf(m_write_buf+m_write_idx,WRITE_BUFFER_SIZE-m_write_idx,format,arglist);
    if(len>=WRITE_BUFFER_SIZE-m_write_idx){
        va_end(arglist);
        return false;
    }
    m_write_idx += len;
    va_end(arglist);
    return true;
}

bool http_conn::add_status_line(int status, const char* title){
    return add_response("%s %d %s\r\n", "HTTP/1.1",status,title);
}

bool http_conn::add_content_length(int content_length){
    return add_response("Content-length: %d\r\n",content_length);
}

bool http_conn::add_blank_line(){
    return add_response("%s","\r\n");
}

bool http_conn::add_headers(int content_length){
    return add_content_length(content_length)&&add_blank_line();
}

bool http_conn::add_content(const char* content){
    return add_response("%s",content);
}


bool http_conn::process_write(HTTP_CODE retcode){
    switch(retcode){
        case BAD_REQUEST:
        {
            //cout<<"----BAD_REQUEST----"<<endl;
            add_status_line(400,error_400_title);
            add_headers(strlen(error_400_form));
            if(!add_content(error_400_form)){
                return false;
            }
            break;
        }
        case NO_RESOURCE:
        {
            //cout<<"----NO_RESOURCE----"<<endl;
            add_status_line(404,error_404_title);
            add_headers(strlen(error_404_form));
            if(!add_content(error_404_form)){
                return false;
            }
            break;
        }
        case FORBIDDEN_FILEPERMISSION:
        {
            //cout<<"----FORBIDDEN_FILEPERMISSION----"<<endl;
            add_status_line(403,error_403_title);
            add_headers(strlen(error_403_form));
            if(!add_content(error_403_form)){
                return false;
            }
            break;
        }
        case INTERNAL_ERROR:
        {
            //cout<<"----INTERNAL_ERROR----"<<endl;
            add_status_line(500,error_500_title);
            add_headers(strlen(error_500_form));
            if(!add_content(error_500_form)){
                return false;
            }
            break;
        } 
        case FILE_REQUEST:
        {
            //cout<<"get file successed"<<endl;
            add_status_line(200,ok_200_title);
            
            if(m_file_stat.st_size!=0){
                //cout<<m_file_stat.st_size<<endl;
                add_headers(m_file_stat.st_size);
                m_iv[0].iov_base = m_write_buf;
                m_iv[0].iov_len = m_write_idx;
                m_iv[1].iov_base = m_file_address;
                m_iv[1].iov_len = m_file_stat.st_size;
                bytes_to_send = m_write_idx+m_file_stat.st_size;
                m_iv_count = 2;
                return true;
            }
            else{
                const char* ok_string = "<html><head> you have requested a empty file </head></html>";
                add_headers(strlen(ok_string));
                if(!add_content(ok_string)){
                    return false;
                }
                return true;
            }
        }
        default:
            //cout<<"----DEFAULT----"<<endl;
            return false;
    }
    m_iv_count = 1;
    m_iv[0].iov_base = m_write_buf;
    m_iv[0].iov_len = m_write_idx;
    bytes_to_send = m_write_idx;
    return true;
}

http_conn::HTTP_CODE http_conn::process_read(){
    LINE_STATUS linestatus = LINE_OK;
    HTTP_CODE retcode = GET_REQUEST;
    while((linestatus=parse_line())==LINE_OK){
        char* temp = m_read_buf+m_start_line;
        m_start_line = m_checked_idx;

        switch(m_check_state){
            case CHECK_STATE_REQUESTLINE:
                retcode = parse_request_line(temp);
                if(retcode==BAD_REQUEST){
                    return BAD_REQUEST;
                }
                break;
            case CHECK_STATE_HEADER:
                retcode = parse_headers(temp);
                if(retcode==BAD_REQUEST){
                    return BAD_REQUEST;
                }
                else if(retcode==GET_REQUEST){
                    return do_request();
                }
                break;
            default:
                return INTERNAL_ERROR;
                break;
        }
    }
    if(linestatus==LINE_OPEN){
        return NO_REQUEST;
    }
    return BAD_REQUEST;
}

void http_conn::process(){
    HTTP_CODE retcode = process_read();
    if(retcode==NO_REQUEST){
        modfd(m_epollfd,m_sockfd,EPOLLIN);
        return;
    }
    if(!process_write(retcode)){
        close_conn();
    }
    modfd(m_epollfd,m_sockfd,EPOLLOUT);

}

void http_conn::close_conn(bool real_close){
    if(real_close && m_sockfd!=-1){
        removefd(m_epollfd,m_sockfd);
        m_sockfd = -1;
        --m_user_count;
    }
}

void http_conn::init(){
    m_read_idx = 0;
    m_checked_idx = 0;
    m_start_line = 0;
    bytes_to_send = 0;
    bytes_have_send = 0;
    m_write_idx = 0;
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_iv_count = 0;
    m_content_length = 0;
    m_url = nullptr;
    m_version = nullptr;
    m_host = nullptr;
    m_file_address = nullptr;
    memset(m_read_buf,'\0',sizeof(m_read_buf));
    memset(m_write_buf,'\0',sizeof(m_write_buf));
    memset(m_real_file,'\0',sizeof(m_real_file));
}

void http_conn::init(int sockfd, const sockaddr_in& addr){
    m_sockfd = sockfd;
    m_address = addr;
    ++m_user_count;
    int opt = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    addfd(m_epollfd,sockfd,true);
    init();
}

int http_conn::m_user_count = 0;
int http_conn::m_epollfd = -1;
int http_conn::m_listenfd = -1;
const char* http_conn::doc_root = "/home/hbj/hbj_webserver/webserver/root";

