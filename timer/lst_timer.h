/*
 * @Author: miracle
 * @Date: 2022-12-5 18.53
 * @LastEditTime: 2022-12-22
 * @LastEditors: miracle
 */

#ifndef WEBSERVER_LSTTIMER_H
#define WEBSERVER_LSTTIMER_H

#include "../Epoll/epoll.h"
#include "../http/httpconn.h"
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <cstring>
#include <assert.h>

namespace webserver
{
class UtilTimer;

struct client_data {
    sockaddr_in address;
    int sockfd;
    UtilTimer* timer;
};

class UtilTimer {
public:
    UtilTimer():prev(nullptr) ,next(nullptr) {};
public:
    time_t expire;
    void(*cb_func)(client_data* );
    client_data *user_data;
    UtilTimer *prev;
    UtilTimer *next;
};
class SortTimerLst{
public:
    SortTimerLst();
    ~SortTimerLst();
    void add_timer(UtilTimer *timer);
    void adjust_timer(UtilTimer *timer);
    void del_timer(UtilTimer *timer);
    void tick();
private:
    void add_timer(UtilTimer *timer, UtilTimer *lst_head);
    UtilTimer *head;
    UtilTimer *tail;
};

class Utils{
public:
    Utils(){}
    ~Utils(){}
    void init(int timeslot);
    int setnonblocking(int fd);
    void addsig(int sig, void(handler)(int), bool restart=true);
    void timer_handler();
    void show_error(int connfd, const char *info);
    void addFd(int fd, bool one_shot, int TRIGMode);
    static void sig_handler(int sig);
public:
    static int *u_pipefd;
    SortTimerLst timer_lst_;
    static Epoll epoll;
    int TIMESLOT_;
};

void cb_func(client_data *user_data);

} // namespace webserver

#endif // WEBSERVER_LSTTIMER_H