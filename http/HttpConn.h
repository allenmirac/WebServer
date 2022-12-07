/*
 * @Author: miracle
 * @Date: 2022-12-7 10.04
 * @LastEditTime: 2022-12-7
 * @LastEditors: miracle
 */

#ifndef WEBSERVER_HTTPCONN_H
#define WEBSERVER_HTTPCONN_H

#include "../InetAddress/InetAddress.h"
#include "../Buffer/Buffer.h"
#include "../CGImysql/sql_connection_pool.h"
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <mysql/mysql.h>
#include <map>
#include <string>

namespace webserver
{

class HttpConn {
public:
    static const int FILENAME_LEN = 200;
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
    enum CHECK_STATE {
        CHECK_STATE_REQUEST = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSE_CONNECTION
    };
    enum LINE_STATUS {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };
public:
    HttpConn(){}
    ~HttpConn(){}

public:
    void init(int sockfd, const sockaddr_in &addr, char*, int, int, std::string user, std::string password, std::string sqlname);
    void close_conn(bool real_close = true);
    void process();
    bool read_once();
    bool write();
    sockaddr_in get_address(){
        return addr_.getAddr();
    };
    void init_mysql_result(connection_pool *connPool);
    int timer_falg;
    int improv;

private:
    void init();
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);
    HTTP_CODE praserequest_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();
    // char *get_line() { return read_buf_ + start_line_;};
    LINE_STATUS parse_line();
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length();
    bool add_linger();
    bool add_blank_line();

public:
    static int epoll_fd_;
    static int user_count_;
    MYSQL *mysql;
    int state_;

private:
    int fd_;
    InetAddress addr_;
    Buffer read_buf_;
    Buffer write_buf_;
    int read_idx_;
    int write_idx_;
    int checked_idx_;
    int start_line_;
    CHECK_STATE check_state_;
    METHOD method_;
    char real_file_[FILENAME_LEN];
    char *url_;
    char *version_;
    char *host_;
    char *content_length_;
    bool linger_;
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
    int TRIG_Mode_;
    int close_log_;

    char sql_user_[100];
    char sql_passwd_[100];
    char sql_name_[100];
};

} // namespace webserver

#endif // WEBSERVER_HTTPCONN_H