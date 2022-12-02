/*
 * @Author: mirac
 * @Date: 2022-12-2 16.45
 * @LastEditTime: 2022-12-2
 * @LastEditors: mirac
 */

#include "log.h"
#include <cstring>

using namespace webserver;

Log::Log(){
    m_count=0;
    m_is_async=false;
}

Log::~Log(){
    if(m_fp != nullptr){
        fclose(m_fp);
    }
}

bool Log::init(const char *file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size){
    if(max_queue_size >=1){
        m_is_async = true;
        m_log_queue = new block_queue<std::string>(max_queue_size);
        pthread_t tid;
        pthread_create(&tid, NULL, flush_log_thread, NULL);
    }

    m_close_log = close_log;
    m_log_buf_size = log_buf_size;
    m_buf = new char[m_log_buf_size];
    memset(m_buf, '\0', m_log_buf_size);
    m_split_lines = split_lines;

    time_t t = time(nullptr);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    // https://www.runoob.com/cprogramming/c-function-strrchr.html
    const char *f_name = strrchr(file_name, '/');
    char log_full_name[256]= {0};
    if(f_name == nullptr){
        // https://www.runoob.com/cprogramming/c-function-snprintf.html
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    } else {
        // eg: file_name: /usr/bin/log1.log
        // log_name=log1.log;
        // dir_name=/usr/bin/
        strcpy(log_name, f_name+1);
        strncpy(dir_name, file_name, f_name-file_name+1);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name);
    }

    m_today = my_tm.tm_mday;
    m_fp = fopen(log_full_name, "a");
    if(m_fp != nullptr){
        return false;
    }
    return true;
}

void Log::write_log(int level, const char *format, ...){

}

void Log::flush(void){
    m_mutex.lock();
    fflush(m_fp);
    m_mutex.unlock();
}
