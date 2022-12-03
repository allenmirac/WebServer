/*
 * @Author: miracle
 * @Date: 2022-12-2 16.45
 * @LastEditTime: 2022-12-3
 * @LastEditors: miracle
 */

#include "log.h"
#include <cstring>
#include <stdarg.h> /*va_start*/

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
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    char s[16] = {0};
    switch(level){
    case 0:
        strcpy(s, "[debug:");
        break;
    case 1:
        strcpy(s, "[info]:");
        break;
    case 2:
        strcpy(s, "[warn]:");
        break;
    case 3:
        strcpy(s, "[erro]:");
        break;
    default:
        strcpy(s, "[info]:");
        break;
    }

    m_mutex.lock();
    m_count++;
    if(m_today != my_tm.tm_mday || m_count % m_split_lines ==0){
        char new_log[256] = {0};
        fflush(m_fp);
        fclose(m_fp);
        char tail[16] = {0};
        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
        if(m_today !=my_tm.tm_mday) {
            snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name);
            m_today = my_tm.tm_mday;
            m_count=0;
        } else {
            snprintf(new_log, 255, "%s%s%s.%lld", dir_name, tail, log_name, m_count/m_split_lines);
        }
        // a=add, if the file not found, create.
        m_fp = fopen(new_log, "a");
    }
    m_mutex.unlock();

    va_list valist;
    va_start(valist, format);
    std::string log_str;// the true output log string.

    m_mutex.lock();
    int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s", my_tm.tm_year + 1900, my_tm.tm_mon+1, my_tm.tm_mday,
                    my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);
    // https://langzi989.github.io/2018/01/01/C%E4%B8%ADsnprintf%E4%B8%8Evsnprintf%E5%87%BD%E6%95%B0/
    int m = vsnprintf(m_buf+n, m_log_buf_size-1, format, valist);
    m_buf[n+m] = '\n';
    m_buf[n+m+1] = '\0';
    log_str = m_buf;
    m_mutex.unlock();

    if(m_is_async && !m_log_queue->full()) {
        m_log_queue->push(log_str);
    } else {
        m_mutex.lock();
        fputs(log_str.c_str(), m_fp);
        m_mutex.lock();
    }
    va_end(valist);
}

void Log::flush(void){
    m_mutex.lock();
    fflush(m_fp);
    m_mutex.unlock();
}
