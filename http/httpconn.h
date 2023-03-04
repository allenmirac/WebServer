/*
 * @Author: miracle
 * @Date: 2022-12-7 10.04
 * @LastEditTime: 2022-12-7
 * @LastEditors: miracle
 */

#ifndef WEBSERVER_HTTPCONN_H
#define WEBSERVER_HTTPCONN_H

#include "../InetAddress/inetaddress.h"
#include "../CGImysql/connpool.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <map>
#include <string>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <sys/mman.h>

namespace webserver
{

// HTTP请求的读取与分析
class HttpConn {
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    enum METHOD {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    // 主状态机的三种可能的状态：当前正在分析请求行，当前正在分析头部字段、当前正在分析内容
    enum CHECK_STATE {
        CHECK_STATE_REQUESTLINE = 0,    //当前正在分析请求行
        CHECK_STATE_HEADER,             //当前正在分析头部字段
        CHECK_STATE_CONTENT             //当前正在分析内容
    };
    // 服务器处理HTTP请求的结果
    enum HTTP_CODE {
        NO_REQUEST,                     //请求不完整、需要继续读取用户数据
        GET_REQUEST,                    //获得了一个完整的用户请求
        BAD_REQUEST,                    //用户请求语法错误
        NO_RESOURCE,                    //没有资源
        FORBIDDEN_REQUEST,              //用户对资源没有访问权限
        FILE_REQUEST,                   //文件请求
        INTERNAL_ERROR,                 //服务器内部错误
        CLOSE_CONNECTION                //客户端已关闭连接
    };
    // 从状态机的三种可能的状态，读取到一个完整的行，行出错，行数据尚且不完整
    enum LINE_STATUS {
        LINE_OK = 0,                    //读取到一个完整的行
        LINE_BAD,                       //行出错
        LINE_OPEN                       //行数据尚且不完整
    };
public:
    HttpConn(){}
    ~HttpConn(){}

public:
    void init(int sockfd, const sockaddr_in &addr, char* root, int, int, std::string user, std::string password, std::string dataBaseName);
    void close_conn(bool real_close = true);
    void process();
    bool read_once();
    bool write();
    sockaddr_in get_address(){
        return addr_;
    };
    void init_mysql_result(ConnPool *connPool);
    int timer_flag;
    int improv;

private:
    void init();
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();
    char *get_line() { return read_buf_ + start_line_;};
    LINE_STATUS parse_line();
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int );
    bool add_linger();
    bool add_blank_line();

public:
    static int epfd_;
    static int user_count_;
    sql::Connection *mysql;
    int state_;

private:
    int fd_;
    sockaddr_in addr_;
    char read_buf_[READ_BUFFER_SIZE];
    char write_buf_[WRITE_BUFFER_SIZE];
    int read_idx_;//read index in the message
    int write_idx_;// write index in the message
    int checked_idx_;// cursor
    int start_line_;
    CHECK_STATE check_state_;
    METHOD method_;
    char real_file_[FILENAME_LEN];
    char *url_;
    char *version_;
    char *host_;
    int content_length_;
    bool linger_;//徘徊；逗留,持续连接的开关
    char *file_address_;
    struct stat file_stat_;
    struct iovec iv_[2];
    int iv_count_;
    int cgi_;
    char *str_req_head_;
    int bytes_to_send;
    int bytes_have_send;
    char *doc_root_;

    std::map<std::string, std::string> users_;
    int TRIGMode_;
    int close_log_;

    char sql_user_[100];
    char sql_passwd_[100];
    char sql_name_[100];
};

} // namespace webserver

#endif // WEBSERVER_HTTPCONN_H