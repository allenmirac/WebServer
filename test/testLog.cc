#include "../log/log.h"
#include <cstring>
#include <cstdio>

using namespace webserver;
int main(){
    // https://blog.csdn.net/whatday/article/details/106267398
    // const char *file_name="/use/bin/log1.log";
    // const char *f_name = strrchr(file_name, '/');
    // char *log_name=new char[20];
    // char *dir_name=new char[20];
    // strcpy(log_name, f_name+1);
    // strncpy(dir_name, file_name, f_name-file_name+1);
    // printf("%s + %s = %s", dir_name, log_name, file_name);
    // delete[] dir_name;
    // delete[] log_name;
    Log *log = Log::get_instance();
    log->init("./server.log", 0);
    LOG_DEBUG("123456");
    return 0;
}
/*
g++ -g -o testLog testLog.cc ../log/log.cc -lpthread
*/