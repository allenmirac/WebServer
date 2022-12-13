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
    url_ = 0;
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
        temp = read_buf_[check_idx_];
        if(temp == '\r'){
            if((checked_idx_ + 1) == read_idx_){
                return LINE_OPEN;
            } else if(read_buf_[checked_idx_+1] == '\n'){
                read_buf_[check_idx_++] = '\0';
                read_buf_[checked_idx_] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }else if(temp=='\n'){
            if(check_idx_ > 1 && read_buf_[checked_idx_-1]=='\r'){
                read_buf_[check_idx_-1]='\0';
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

} // namespace webserver
