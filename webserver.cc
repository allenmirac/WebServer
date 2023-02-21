#include "webserver.h"

namespace webserver
{

WebServer::WebServer(){
    users_ = new HttpConn[MAX_FD/1000];//epoll_create1 failed: Too many open files

    char server_path[200];
    getcwd(server_path, 200);
    char root[10]="/resource";
    root_ = (char *)malloc(strlen(server_path)+strlen(root)+1);
    strcpy(root_, server_path);
    strcat(root_, root);
    std::cout<<root_<<std::endl;
    //定时器
    users_timer = new client_data[MAX_FD];
}

WebServer::~WebServer(){
    close(listenfd_);
    close(pipefd_[0]);
    close(pipefd_[1]);
    delete[] users_;
    delete[] users_timer;
}

void WebServer::init(int port, std::string user, std::string passWord, std::string databaseName, int log_write, 
                    int opt_linger, int trigmode, int sql_num, int thread_num, int close_log, int actor_model)
{
    port_ = port;
    user_ = user;
    passWord_ = passWord;
    databaseName_ = databaseName;
    sql_num_ = sql_num;
    thread_num_ = thread_num;
    log_write_ = log_write;
    OPT_LINGER_ = opt_linger;
    TRIGMode_ = trigmode;
    close_log_ = close_log;
    actormodel_ = actor_model;
}

// trigger mode
void WebServer::trig_mode()
{
    //LT + LT
    if (0 == TRIGMode_){
        LISTENTrigmode_ = 0;
        CONNTrigmode_ = 0;
    }
    //LT + ET
    else if (1 == TRIGMode_){
        LISTENTrigmode_ = 0;
        CONNTrigmode_ = 1;
    }
    //ET + LT
    else if (2 == TRIGMode_){
        LISTENTrigmode_ = 1;
        CONNTrigmode_ = 0;
    }
    //ET + ET
    else if (3 == TRIGMode_){
        LISTENTrigmode_ = 1;
        CONNTrigmode_ = 1;
    }
}

void WebServer::log_write()
{
    if (0 == close_log_){
        //初始化日志
        if (1 == log_write_){
            Log::get_instance()->init("./ServerLog", close_log_, 2000, 800000, 800);
        } else {
            Log::get_instance()->init("./ServerLog", close_log_, 2000, 800000, 0);
        }
    }
}

void WebServer::sql_pool()
{
    //初始化数据库连接池
    connPool_ = ConnPool::GetInstance();
    connPool_->init(url_, user_, passWord_, databaseName_, sql_num_, close_log_);

    //初始化数据库读取表
    users_->init_mysql_result(connPool_);
}

void WebServer::thread_pool()
{
    //线程池
    pool_ = new ThreadPool<HttpConn>(actormodel_, connPool_, thread_num_);
}

void WebServer::eventListen()
{
    //网络编程基础步骤
    listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd_ >= 0);

    //优雅关闭连接:https://www.cnblogs.com/caosiyang/archive/2012/03/29/2422956.html
    if (0 == OPT_LINGER_)
    {
        struct linger tmp = {0, 1};
        setsockopt(listenfd_, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }else if (1 == OPT_LINGER_){
        struct linger tmp = {1, 1};
        setsockopt(listenfd_, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port_);
    // InetAddress addr("127.0.0.1", port_);////服务器ip地址, 端口

    int flag = 1;
    setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    // sockaddr_in address = addr.getAddr();
    ret = bind(listenfd_, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(listenfd_, 5);
    assert(ret >= 0);

    utils.init(TIMESLOT);

    //epoll创建内核事件表
    epoll_event events[MAX_EVENT_NUMBER];
    epfd = epoll_create(5);
    // struct epoll_event ev;
    // ev.data.fd = listenfd_;
    // ev.events = EPOLLIN;
    // epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd_, &ev);
    assert(epfd != -1);

    

    utils.addFd(epfd, listenfd_, false, LISTENTrigmode_);
    HttpConn::epfd_ = epfd;

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd_);//pipefd_[0],pipefd_[1],这对套接字可以用于全双工通信.
    assert(ret != -1);
    utils.setnonblocking(pipefd_[1]);
    // utils.addFd(epfd, pipefd_[0], false, 0);

    //添加信号处理
    utils.addsig(SIGPIPE, SIG_IGN);
    utils.addsig(SIGALRM, utils.sig_handler, false);
    utils.addsig(SIGTERM, utils.sig_handler, false);

    alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    Utils::u_pipefd = pipefd_;
    Utils::epfd = epfd;
}

void WebServer::timer(int connfd, InetAddress client_address)
{
    users_[connfd].init(connfd, client_address, root_, CONNTrigmode_, close_log_, user_, passWord_, databaseName_);

    //初始化client_data数据
    //创建定时器，设置回调函数和超时时间，绑定用户数据，将定时器添加到链表中
    users_timer[connfd].address = client_address;
    users_timer[connfd].sockfd = connfd;
    UtilTimer *timer = new UtilTimer();
    timer->user_data = &users_timer[connfd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    users_timer[connfd].timer = timer;
    utils.timer_lst_.add_timer(timer);
}

void WebServer::adjust_timer(UtilTimer *timer)
{
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    utils.timer_lst_.adjust_timer(timer);

    LOG_INFO("%s", "adjust timer once");
}

void WebServer::deal_timer(UtilTimer *timer, int sockfd)
{
    timer->cb_func(&users_timer[sockfd]);
    if (timer){
        utils.timer_lst_.del_timer(timer);
    }

    LOG_INFO("close fd %d", users_timer[sockfd].sockfd);
}

bool WebServer::dealclientdata()
{
    InetAddress client_addr;
    sockaddr_in addr = client_addr.getAddr();
    socklen_t client_addrlength = sizeof(client_addr);
    if (0 == LISTENTrigmode_){
        int connfd = accept(listenfd_, (struct sockaddr *)&addr, &client_addrlength);
        if (connfd < 0){
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            return false;
        }
        // else{
        //     std::cout<<"dealclientdata accpet succeed"<<std::endl;
        // }
        if (HttpConn::user_count_ >= MAX_FD){
            utils.show_error(connfd, "Internal server busy");
            LOG_ERROR("%s", "Internal server busy");
            return false;
        }
        timer(connfd, client_addr);
    }else {
        while (1){
            int connfd = accept(listenfd_, (struct sockaddr *)&addr, &client_addrlength);
            if (connfd < 0)
            {
                LOG_ERROR("%s:errno is:%d", "accept error", errno);
                break;
            }
            if (HttpConn::user_count_ >= MAX_FD)
            {
                utils.show_error(connfd, "Internal server busy");
                LOG_ERROR("%s", "Internal server busy");
                break;
            }
            timer(connfd, client_addr);
        }
        return false;
    }
    return true;
}

bool WebServer::dealwithsignal(bool &timeout, bool &stop_server)
{
    int ret = 0;
    int sig;
    char signals[1024];
    ret = recv(pipefd_[0], signals, sizeof(signals), 0);
    if (ret == -1){
        std::cout<<"ret == -1"<<std::endl;
        return false;
    }else if (ret == 0) {
        std::cout<<"ret == 0"<<std::endl;
        return false;
    }else {
        // std::cout<<"ret != 0,-1"<<std::endl;
        for (int i = 0; i < ret; ++i){
            switch (signals[i])
            {
            case SIGALRM:{
                timeout = true;
                // std::cout<<"timeout = true;"<<std::endl;
                break;
            }
            case SIGTERM:{
                stop_server = true;
                std::cout<<"stop_server = true;"<<std::endl;
                break;
            }
            }
        }
    }
    return true;
}

void WebServer::dealwithread(int sockfd)
{
    UtilTimer *timer = users_timer[sockfd].timer;

    //reactor
    if (1 == actormodel_){
        if (timer){
            adjust_timer(timer);
        }

        //若监测到读事件，将该事件放入请求队列
        pool_->append(users_ + sockfd, 0);

        while (true){
            if (1 == users_[sockfd].improv){
                if (1 == users_[sockfd].timer_flag){
                    deal_timer(timer, sockfd);
                    users_[sockfd].timer_flag = 0;
                }
                users_[sockfd].improv = 0;
                break;
            }
        }
    }
    else
    {
        //proactor
        if (users_[sockfd].read_once()){
            LOG_INFO("deal with the client(%s)", inet_ntoa(users_[sockfd].get_address().sin_addr));

            //若监测到读事件，将该事件放入请求队列
            pool_->append_p(users_ + sockfd);

            if (timer){
                adjust_timer(timer);
            }
        }else{
            deal_timer(timer, sockfd);
            std::cout<<"read_once failed"<<std::endl;
        }
    }
}

void WebServer::dealwithwrite(int sockfd)
{
    UtilTimer *timer = users_timer[sockfd].timer;
    //reactor
    if (1 == actormodel_){
        if (timer){
            adjust_timer(timer);
        }

        pool_->append(users_ + sockfd, 1);

        while (true){
            if (1 == users_[sockfd].improv){
                if (1 == users_[sockfd].timer_flag){
                    deal_timer(timer, sockfd);
                    users_[sockfd].timer_flag = 0;
                }
                users_[sockfd].improv = 0;
                break;
            }
        }
    }else{
        //proactor
        if (users_[sockfd].write()){
            LOG_INFO("send data to the client(%s)", inet_ntoa(users_[sockfd].get_address().sin_addr));

            if (timer){
                adjust_timer(timer);
            }
        }else{
            deal_timer(timer, sockfd);
        }
    }
}

void WebServer::eventLoop()
{
    bool timeout = false;
    bool stop_server = false;

    while (!stop_server){
        // std::cout<<"Epoll_wait:"<<std::endl;
        int number = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
        std::cout<<"Epoll_wait number=:"<<number<<std::endl;
        if (number < 0 && errno != EINTR){
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;

            //处理新到的客户连接
            if (sockfd == listenfd_){
                std::cout<<"sockfd == listenfd_"<<std::endl;
                bool flag = dealclientdata();
                std::cout<<"flag= "<<flag<<", i= "<<i<<std::endl;
                if (false == flag)
                    continue;
            }else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                std::cout<<"client close"<<std::endl;
                //服务器端关闭连接，移除对应的定时器
                UtilTimer *timer = users_timer[sockfd].timer;
                deal_timer(timer, sockfd);
            }
            
            //处理客户连接上接收到的数据
            else if (events[i].events & EPOLLIN){
                std::cout<<"Epollin"<<std::endl;
                dealwithread(sockfd);
            }else if (events[i].events & EPOLLOUT){
                std::cout<<"Epollout"<<std::endl;
                dealwithwrite(sockfd);
            }
            //处理信号
            else if ((sockfd == pipefd_[0]) && (events[i].events & EPOLLIN)){
                std::cout<<"Dealwithsignal"<<std::endl;
                bool flag = dealwithsignal(timeout, stop_server);
                // std::cout<<"flag= "<<flag<<", i= "<<i<<std::endl;
                if (false == flag)
                    LOG_ERROR("%s", "dealclientdata failure");
            }
            std::cout<<"i= "<<i<<std::endl;
        }
        std::cout<<"timeout= "<<timeout<<std::endl;
        if (timeout){
            utils.timer_handler();

            LOG_INFO("%s", "timer tick");

            timeout = false;
        }
    }
}

} // namespace webserver