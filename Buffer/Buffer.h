/*
 * @Author: miracle
 * @Date: 2022-12-7 11.03
 * @LastEditTime: 2022-12-7
 * @LastEditors: miracle
 */

#ifndef WEBSERVER_BUFFER_H
#define WEBSERVER_BUFFER_H

#include <string>
namespace webserver
{

class Buffer
{
private:
    std::string buf;
public:
    Buffer();
    ~Buffer();
    void append(const char* _str, int _size);
    ssize_t size();
    char* c_str() const;
    void clear();
    void getline();
};

} // namespace webserver
#endif // WEBSERVER_BUFFER_H
