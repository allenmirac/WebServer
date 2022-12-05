/*
 * @Author: miracle
 * @Date: 2022-12-5 18.53
 * @LastEditTime: 2022-12-5
 * @LastEditors: miracle
 */

#ifndef WEBSERVER_LSTTIMER_H
#define WEBSERVER_LSTTIMER_H

#include <arpa/inet.h>

class util_timer;

struct client_data {
    sockaddr_in address;
    int sockfd;
    util_timer* timer;
};

class util_timer {
    
};

#endif // WEBSERVER_LSTTIMER_H