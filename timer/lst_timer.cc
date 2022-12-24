#include "lst_timer.h"

namespace webserver
{
SortTimerLst::SortTimerLst(){
    head=nullptr;
    tail=nullptr;
}

SortTimerLst::~SortTimerLst(){
    UtilTimer *tmp = head;
    while (tmp){
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

void SortTimerLst::add_timer(UtilTimer *timer){
    if (!timer){
        return;
    }
    if (!head){
        head = tail = timer;
        return;
    }
    if (timer->expire < head->expire){
        timer->next = head;
        head->prev = timer;
        head = timer;
        return;
    }
    add_timer(timer, head);
}

void SortTimerLst::adjust_timer(UtilTimer *timer){
    if (!timer){
        return;
    }
    UtilTimer *tmp = timer->next;
    if (!tmp || (timer->expire < tmp->expire)){
        return;
    }
    if (timer == head){
        head = head->next;
        head->prev = NULL;
        timer->next = NULL;
        add_timer(timer, head);
    }else{
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer, timer->next);
    }
}

void SortTimerLst::del_timer(UtilTimer *timer){
    if (!timer){
        return;
    }
    if ((timer == head) && (timer == tail)){
        delete timer;
        head = NULL;
        tail = NULL;
        return;
    }
    if (timer == head){
        head = head->next;
        head->prev = NULL;
        delete timer;
        return;
    }
    if (timer == tail){
        tail = tail->prev;
        tail->next = NULL;
        delete timer;
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}

void SortTimerLst::tick(){
    if (!head){
        return;
    }
    
    time_t cur = time(NULL);
    UtilTimer *tmp = head;
    while (tmp)
    {
        if (cur < tmp->expire)
        {
            break;
        }
        tmp->cb_func(tmp->user_data);
        head = tmp->next;
        if (head)
        {
            head->prev = NULL;
        }
        delete tmp;
        tmp = head;
    }
}

void SortTimerLst::add_timer(UtilTimer *timer, UtilTimer *lst_head){
    UtilTimer *prev = lst_head;
    UtilTimer *tmp = prev->next;
    while(tmp){
        if(timer->expire <tmp->expire){
            prev->next=timer;
            timer->next = tmp;
            tmp->prev = timer;
            timer->prev = prev;
            break;
        }
        prev = tmp;
        tmp = tmp->next;
    }
    if(!tmp){
        prev->next = timer;
        timer->prev = prev;
        timer->next = nullptr;
        tail = timer;
    }
}

void Utils::init(int timerslot){
    TIMESLOT_ = timerslot;
}

int Utils::setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void Utils::addFd(int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll.addFd(fd, one_shot, TRIGMode);
    setnonblocking(fd);
}
void Utils::sig_handler(int sig)
{
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

void Utils::timer_handler()
{
    timer_lst_.tick();
    alarm(TIMESLOT_);
}

void Utils::show_error(int connfd, const char *info)
{
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int *Utils::u_pipefd = 0;
Epoll Utils::epoll;

class Utils;
void cb_func(client_data *data){
    Utils::epoll.removeFd(data->sockfd);
    assert(data);
    close(data->sockfd);
    HttpConn::user_count_--;
}
} // namespace webserver
