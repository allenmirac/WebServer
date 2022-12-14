#include "httpconn.h"

namespace webserver
{
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";

void HttpConn::init_mysql_result(ConnPool *connPool){
    sql::Connection *con = connPool->GetConnection();
    connectionRAII mysqlcon(&mysql, connPool);
    std::string sql = "select name,password from user";
    sql::Statement *stat = mysql->createStatement();
    sql::ResultSet *rs = stat->executeQuery(sql);
    if(rs->getFetchSize()==0){
        LOG_ERROR("select error:%s\n");
    }
    while(rs->next()){
        std::string name = rs->getString(1);
        std::string password = rs->getString(2);
        users_[name] = password;
    }

    delete stat;
    delete rs;
}

void HttpConn::close_conn(bool real_close){
    if(real_close && (fd_!=-1)){
        printf("close fd=%d", fd_);
        epoll.removeFd(fd_);
        user_count_--;
        fd_=-1;
    }
}

void HttpConn::init(int sockfd, InetAddress addr, char* root, int TRIGMode, int close_log, std::string user, std::string password, std::string dataBaseName){
    fd_=sockfd;
    addr_ = addr;

    epoll.addFd(fd_, true, TRIGMode_);
    user_count_++;

    doc_root_ = root;
    TRIGMode_  = TRIGMode;
    close_log_ = close_log;
    strcpy(sql_user_, user.c_str());
    strcpy(sql_passwd_, password.c_str());
    strcpy(sql_name_, dataBaseName.c_str());

    init();
}
void HttpConn::init(){
    mysql = nullptr;
    bytes_to_send = 0;
    bytes_have_send = 0;
    check_state_ = CHECK_STATE_REQUESTLINE;
    linger_ = false;
    method_ = GET;
    url_ = nullptr;
    version_ = 0;
    content_length_ = 0;
    host_ = 0;
    start_line_ = 0;
    checked_idx_ = 0;
    cgi_ = 0;
    state_ = 0;
    timer_falg = 0;
    improv = 0;
    memset(read_buf_, '\0', READ_BUFFER_SIZE);
    memset(write_buf_, '\0', WRITE_BUFFER_SIZE);
    memset(real_file_, '\0', FILENAME_LEN);
}

HttpConn::LINE_STATUS HttpConn::parse_line(){
    char temp;
    for(; checked_idx_ < read_idx_; ++checked_idx_){
        temp = read_buf_[checked_idx_];
        if(temp == '\r'){
            if((checked_idx_ + 1) == read_idx_){
                return LINE_OPEN;
            } else if(read_buf_[checked_idx_+1] == '\n'){
                read_buf_[checked_idx_++] = '\0';
                read_buf_[checked_idx_] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }else if(temp=='\n'){
            if(checked_idx_ > 1 && read_buf_[checked_idx_-1]=='\r'){
                read_buf_[checked_idx_-1]='\0';
                read_buf_[checked_idx_++]='\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

bool HttpConn::read_once(){
    if(read_idx_ >= READ_BUFFER_SIZE){
        return false;
    }
    int bytes_read=0;

    // LT
    if(0 == TRIGMode_){
        bytes_read = recv(fd_, read_buf_ + read_idx_, READ_BUFFER_SIZE-read_idx_, 0);
        if(bytes_read<=0){
            return false;
        }
        return true;
    }else{ // ET
        while(true){
            bytes_read = recv(fd_, read_buf_+read_idx_, READ_BUFFER_SIZE-read_idx_, 0);
            if(bytes_read==-1){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    break;
                }
                return false;
            } else if(bytes_read == 0){
                return false;
            }
            read_idx_ += bytes_read;
        }
        return true;
    }
}

HttpConn::HTTP_CODE HttpConn::parse_request_line(char *text){
    // https://cplusplus.com/reference/cstring/strpbrk/
    url_ = strpbrk(text, " \t");
    if(!url_){
        return BAD_REQUEST;
    }
    *url_ ++='\0';
    char *method = text;
    if(strcasecmp(method, "GET") == 0){
        method_ = GET;
    } else if(strcasecmp(method, "POST") == 0){
        method_ = POST;
        cgi_ = 1;
    } else {
        return BAD_REQUEST;
    }
    url_ +=strspn(url_, " \t");
    version_ +=strspn(version_, " \t");

    if(strcasecmp(version_, "HTTP/1.1")!=0){
        return BAD_REQUEST;
    }
    if(strncasecmp(url_, "http://", 7) == 0){
        url_ += 7;
        url_ = strchr(url_, '/');
    }
    if(strncasecmp(url_, "https://", 8) == 0){
        url_ += 8;
        url_ = strchr(url_, '/');
    }
    if(!url_ || url_[0] != '/'){
        return BAD_REQUEST;
    }
    if(strlen(url_)==1){
        strcat(url_, "judge.html");
    }

    check_state_ = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HttpConn::HTTP_CODE HttpConn::parse_headers(char *text){
    if(text[0] == '\0'){
        if(content_length_ != 0){
            checked_idx_ = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        } else if(strncasecmp(text, "Connection:", 11)==0){
            text += 11;
            text += strspn(text, " \t");
            if(strcasecmp(text, "keep-alive")==0){
                linger_ = true;
            }
        } else if(strncasecmp(text, "Content-length:", 15)==0){
            text += 15;
            text += strspn(text, " \t");
            content_length_ = atol(text);
        } else if(strncasecmp(text, "Host:", 5) == 0){
            text += 5;
            text += strspn(text, " \t");
            host_ = text;
        } else {
            LOG_INFO("oop!unknow header: %s", text);
        }
        return NO_REQUEST;
    }
}

HttpConn::HTTP_CODE HttpConn::parse_content(char *text){
    if(read_idx_ >= (content_length_ + checked_idx_)){
        text[content_length_] = '\0';
        str_req_head_ = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

HttpConn::HTTP_CODE HttpConn::process_read(){
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *text =nullptr;

    while((check_state_ == CHECK_STATE_CONTENT && line_status == LINE_OK)){
        text = get_line();
        start_line_ = checked_idx_;
        LOG_INFO("%s", text);
        switch (check_state_)
        {
        case CHECK_STATE_REQUESTLINE:{
            ret = parse_request_line(text);
            if(ret == BAD_REQUEST){
                return BAD_REQUEST;
            }
            break;
        }
        case CHECK_STATE_HEADER:{
            ret = parse_headers(text);
            if(ret == BAD_REQUEST){
                return BAD_REQUEST;
            } else if(ret == GET_REQUEST){
                return do_request();
                line_status = LINE_OPEN;
                break;
            }
        }
        default:
            return INTERNAL_ERROR;
        }
    }
    return NO_REQUEST;
}

locker lock;
HttpConn::HTTP_CODE HttpConn::do_request()
{
    strcpy(real_file_, doc_root_);
    int len = strlen(doc_root_);
    //printf("url_:%s\n", url_);
    const char *p = strrchr(url_, '/');

    if (cgi_ == 1 && (*(p + 1) == '2' || *(p + 1) == '3')){
        char flag = url_[1];
        char *url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(url_real, "/");
        strcat(url_real, url_ + 2);
        strncpy(real_file_ + len, url_real, FILENAME_LEN - len - 1);
        free(url_real);

        //将用户名和密码提取出来
        //user=123&passwd=123
        char name[100], password[100];
        int i;
        for (i = 5; str_req_head_[i] != '&'; ++i)
            name[i - 5] = str_req_head_[i];
        name[i - 5] = '\0';

        int j = 0;
        for (i = i + 10; str_req_head_[i] != '\0'; ++i, ++j)
            password[j] = str_req_head_[i];
        password[j] = '\0';

        if (*(p + 1) == '3'){
            //如果是注册，先检测数据库中是否有重名的
            //没有重名的，进行增加数据
            char *sql_insert = (char *)malloc(sizeof(char) * 200);
            strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
            strcat(sql_insert, "'");
            strcat(sql_insert, name);
            strcat(sql_insert, "', '");
            strcat(sql_insert, password);
            strcat(sql_insert, "')");

            if (users_.find(name) == users_.end()){
                lock.lock();
                sql::Statement *stat;
                sql::ResultSet *rs;
                stat = mysql->createStatement();
                rs = stat->executeQuery(sql_insert);
                users_.insert(std::pair<std::string, std::string>(name, password));
                lock.unlock();

                if (rs->wasNull()){
                    strcpy(url_, "/log.html");
                } else {
                    strcpy(url_, "/registerError.html");
                }
            } else {
                strcpy(url_, "/registerError.html");
            }
        }
        //如果是登录，直接判断
        //若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
        else if (*(p + 1) == '2'){
            if (users_.find(name) != users_.end() && users_[name] == password){
                strcpy(url_, "/welcome.html");
            } else {
                strcpy(url_, "/logError.html");
            }
        }
    }

    if (*(p + 1) == '0'){
        char *url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(url_real, "/register.html");
        strncpy(real_file_ + len, url_real, strlen(url_real));

        free(url_real);
    }else if (*(p + 1) == '1'){
        char *url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(url_real, "/log.html");
        strncpy(real_file_ + len, url_real, strlen(url_real));

        free(url_real);
    }else if (*(p + 1) == '5'){
        char *url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(url_real, "/picture.html");
        strncpy(real_file_ + len, url_real, strlen(url_real));

        free(url_real);
    }else if (*(p + 1) == '6'){
        char *url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(url_real, "/video.html");
        strncpy(real_file_ + len, url_real, strlen(url_real));

        free(url_real);
    }else if (*(p + 1) == '7'){
        char *url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(url_real, "/fans.html");
        strncpy(real_file_ + len, url_real, strlen(url_real));

        free(url_real);
    }
    else {
        strncpy(real_file_ + len, url_, FILENAME_LEN - len - 1);
    }
    if (stat(real_file_, &file_stat_) < 0)
        return NO_RESOURCE;

    if (!(file_stat_.st_mode & S_IROTH))
        return FORBIDDEN_REQUEST;

    if (S_ISDIR(file_stat_.st_mode))
        return BAD_REQUEST;

    int fd = open(real_file_, O_RDONLY);
    file_address_ = (char *)mmap(0, file_stat_.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}

void HttpConn::unmap(){
    if(file_address_){
        munmap(file_address_, file_stat_.st_size);
        file_address_ = 0;
    }
}

bool HttpConn::write()
{
    int temp = 0;

    if (bytes_to_send == 0){
        epoll.modifyFd(fd_, EPOLLIN, TRIGMode_);
        init();
        return true;
    }

    while (1){
        temp = writev(fd_, iv_, iv_count_);

        if (temp < 0){
            if (errno == EAGAIN){
                epoll.modifyFd(fd_, EPOLLOUT, TRIGMode_);
                return true;
            }
            unmap();
            return false;
        }

        bytes_have_send += temp;
        bytes_to_send -= temp;
        if (bytes_have_send >= iv_[0].iov_len){
            iv_[0].iov_len = 0;
            iv_[1].iov_base = file_address_ + (bytes_have_send - write_idx_);
            iv_[1].iov_len = bytes_to_send;
        }else{
            iv_[0].iov_base = write_buf_ + bytes_have_send;
            iv_[0].iov_len = iv_[0].iov_len - bytes_have_send;
        }

        if (bytes_to_send <= 0){
            unmap();
            epoll.modifyFd(fd_, EPOLLIN, TRIGMode_);

            if (linger_){
                init();
                return true;
            }else{
                return false;
            }
        }
    }
}

} // namespace webserver
