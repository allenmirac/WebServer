#include "epoll.h"
#include "../util/util.h"

#define MAX_EVENTS 1000

namespace webserver
{

Epoll::Epoll() : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    error_if(epfd == -1, "epoll_create1 failed");
    events = new epoll_event[MAX_EVENTS];
    bzero(events, sizeof(*events) * MAX_EVENTS);
}

Epoll::~Epoll() {
    if(epfd != -1) {
        close(epfd);
        epfd = -1;
    }
    delete[] events;
}
int Epoll::setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void Epoll::addFd(int fd, bool one_shot, int TRIGMode) {
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.fd = fd;
    if(1 == TRIGMode){
        events->events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    } else {
        events->events = EPOLLIN | EPOLLRDHUP;
    }

    if(one_shot) {
        events->events |= EPOLLONESHOT;
    }
    error_if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll_ctl add failed");
    setnonblocking(fd);
}

void Epoll::removeFd(int fd){
    error_if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, 0)==-1, "remove fd error");
}

void Epoll::modifyFd(int fd, uint32_t ev, int TRIGMode){
    epoll_event event;
    event.data.fd = fd;
    if(1 == TRIGMode){
        events->events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    } else {
        events->events = ev | EPOLLONESHOT | EPOLLRDHUP;
    }
    error_if(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event)==-1, "modify fd error");
}

std::vector<epoll_event> Epoll::poll(int timeout) {
    std::vector<epoll_event> activeEvents;
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    error_if(nfds == -1, "epoll_wait failed");
    for(int i=0; i<nfds; ++i) {
        activeEvents.push_back(events[i]);
    }
    return activeEvents;
}

} // namespace webserver
