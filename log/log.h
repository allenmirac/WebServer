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
#include "block_queue.h"

namespace webserver
{
class Log{
public:
    static Log *get_instance(){
        static Log instance;
        return &instance;
    }

    static void *flush_log_thread(void* args){
        Log::get_instance()->async_write_log();
    }
    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush(void);
private:
    Log();
    virtual ~Log();
    void *async_write_log(){
        std::string single_log;
        while(m_log_queue->pop(single_log)){
            m_mutex.lock();
            fputs(single_log.c_str(), m_fp); // write to the m_fp
            m_mutex.unlock();
        }
    }
private:
    char dir_name[128]; // dir name
    char log_name[128]; // file name
    int m_split_lines;  // max lines
    int m_log_buf_size;   // buf size
    long long m_count;  // line count
    int m_today;        // which day the log writed
    FILE *m_fp;         // file pointer
    char *m_buf;
    block_queue<std::string> *m_log_queue;  // Buffered queue, use it when m_is_async is true
    bool m_is_async;    // is asynchronous or not
    locker m_mutex;     // mutex
    int m_close_log;    // close
};

#define LOG_DEBUG(format, ...) if(0==m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0==m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0==m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0==m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}


} // namespace webserver

#endif // WEBSERVER_LOG_H