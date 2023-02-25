/*
 * @Author: miracle
 * @Date: 2022-12-1 21.13
 * @LastEditTime: 2022-12-2
 * @LastEditors: miracle
 */

#ifndef WEBSERVER_LOG_H
#define WEBSERVER_LOG_H

#include <pthread.h>
#include <iostream>
#include <string>
#include <cstring>
#include <stdarg.h> /*va_start*/
#include "block_queue.h"

namespace webserver
{
class Log{
public:
    static Log *get_instance(){
        // std::cout<<"Log getinstance()"<<std::endl;
        static Log instance_;
        // std::cout<<"get_instance"<<std::endl;
        return &instance_;
    }

    static void *flush_log_thread(void* args){
        Log::get_instance()->async_write_log();
        return nullptr;
    }
    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush(void);

    int get_close_log(){ return close_log_; };
private:
    Log();
    virtual ~Log();
    void *async_write_log(){
        std::string single_log;
        while(log_queue_->pop(single_log)){
            mutex_.lock();
            fputs(single_log.c_str(), fp_); // write to the fp
            mutex_.unlock();
        }
        return nullptr;
    }
private:
    char dir_name[128]; // dir name
    char log_name[128]; // file name
    int split_lines_;  // max lines
    int log_buf_size_;   // buf size
    long long count_;  // line count
    int today;        // which day the log writed
    FILE *fp_;         // file pointer
    char *buf_;
    block_queue<std::string> *log_queue_;  // Buffered queue, use it when is_async is true
    bool is_async_;    // is asynchronous or not
    locker mutex_;     // mutex
    int close_log_;    // close
    static Log *instance_;
};

#define LOG_DEBUG(format, ...) if(0==Log::get_instance()->get_close_log()) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0==Log::get_instance()->get_close_log()) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0==Log::get_instance()->get_close_log()) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0==Log::get_instance()->get_close_log()) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}


} // namespace webserver

#endif // WEBSERVER_LOG_H