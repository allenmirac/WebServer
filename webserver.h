#include "./threadpool/threadpool.h"
#include "./http/httpconn.h"
#include "./InetAddress/inetaddress.h"
#include "./timer/lst_timer.h"
#include "./log/log.h"
#include <string>
#include <assert.h>
#include <signal.h>

const int MAX_FD = 65536;
const int MAX_EVENT_NUMBER = 10000;
const int TIMESLOT = 5;
namespace webserver
{

class WebServer{
public:
    WebServer();
    ~WebServer();
    void init(int port, std::string user, std::string password, 
              std::string databaseName, int log_write, int opt_linger,
              int trigmode, int sql_num, int thread_nu, int close_log, 
              int actor_model);
    void thread_pool();
    void sql_pool();
    void log_write();
    void trig_mode();
    void eventListen();
    void eventLoop();
    void timer(int connfd, sockaddr_in client_addr);
    void adjust_timer(UtilTimer *timer);
    void deal_timer(UtilTimer *timer, int sockfd);
    bool dealclientdata();
    bool dealwithsignal(bool& timeout, bool& stop_server);
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);

public:
    //基础
    int port_;
    char *root_;
    // 是否异步写
    int log_write_;
    // 0 open log 1 close log
    int close_log_;
    // 
    int actormodel_;

    int pipefd_[2];
    int epfd;
    HttpConn *users_;

    //数据库相关
    ConnPool *connPool_;
    std::string url_;
    std::string user_;         //登陆数据库用户名
    std::string passWord_;     //登陆数据库密码
    std::string databaseName_; //使用数据库名
    int sql_num_;

    //线程池相关
    ThreadPool<HttpConn> *pool_;
    int thread_num_;

    //epoll_event相关
    epoll_event events[MAX_EVENT_NUMBER];

    int listenfd_;
    //优雅关闭连接，默认不使用
    int OPT_LINGER_;
    //触发组合模式,默认listenfd LT + connfd LT
    int TRIGMode_;
    //listenfd触发模式，默认LT
    int LISTENTrigmode_;
    //connfd触发模式，默认LT
    int CONNTrigmode_;

    //定时器相关
    client_data *users_timer;
    Utils utils;
};

} // namespace webserver