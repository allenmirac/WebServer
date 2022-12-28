/*
 * @Author: miracle
 * @Date: 2022-12-9 11.25
 * @LastEditTime: 2022-12-9
 * @LastEditors: miracle
 */

#ifndef WEBSERVER_EPOLL_H
#define WEBSERVER_EPOLL_H

#include <sys/epoll.h>
#include <vector>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>

namespace webserver{
class Epoll
{
private:
    int epfd;
    struct epoll_event *events;
public:
    Epoll();
    ~Epoll();
    int setnonblocking(int fd);
    int getEpfd(){return epfd;}
    void setEpfd(int fd){ epfd = fd;}
    epoll_event *getEvents(){ return events;}
    void setEvent(epoll_event *events){this->events = events;};
    void addFd(int fd, bool one_shot, int TRIGMode);
    void removeFd(int fd);
    void modifyFd(int fd, uint32_t ev, int TRIGMode);
    std::vector<epoll_event> poll(int timeout = -1);
};

} // namesapce webserver

#endif // WEBSERVER_EPOLL_H
