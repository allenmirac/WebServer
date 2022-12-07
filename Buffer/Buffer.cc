#include <iostream>
#include "Buffer.h"

namespace webserver
{

Buffer::Buffer(){

}

Buffer::~Buffer() {

}

void Buffer::append(const char *str, int _size) {
    for(int i=0; i<_size; i++){
        if(str[i]=='\0') break;
        buf.push_back(str[i]);
    }
}

ssize_t Buffer::size() {
    return buf.size();
}

char* Buffer::c_str() const {
    return const_cast<char *>(buf.c_str());
}

void Buffer::clear() {
    buf.clear();
}

void Buffer::getline() {
    buf.clear();
    std::getline(std::cin, buf);
}

} // namespace webserver
