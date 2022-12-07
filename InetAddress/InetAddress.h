/*
 * @Author: miracle
 * @Date: 2022-12-7 10.56
 * @LastEditTime: 2022-12-7
 * @LastEditors: miracle
 */

#ifndef WEBSERVER_INETADDRESS_H
#define WEBSERVER_INETADDRESS_H

#include <arpa/inet.h>

namespace webserver
{
class InetAddress{
private:
    struct sockaddr_in addr;
    socklen_t addr_len;
public:
    InetAddress();
    InetAddress(const char* ip, uint16_t port);
    ~InetAddress();
    void setInetAddr(sockaddr_in _addr, socklen_t _addr_len);
    sockaddr_in getAddr();
    socklen_t getAddr_len();
};

} // namespace webserver

#endif //WEBSERVER_INETADDRESS_H