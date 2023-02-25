/*
 * @Author: miracle
 * @Date: 2022-12-2 16.45
 * @LastEditTime: 2022-12-3
 * @LastEditors: miracle
 */

#include "log.h"

namespace webserver {

Log *Log::instance_ = nullptr;

Log::Log(){
    count_=0;
    is_async_=false;
}

Log::~Log(){
    if(fp_ != nullptr){
        fclose(fp_);
    }
}

bool Log::init(const char *file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size){
    if(max_queue_size >=1){
        is_async_ = true;
        log_queue_ = new block_queue<std::string>(max_queue_size);
        pthread_t tid;
        pthread_create(&tid, NULL, flush_log_thread, NULL);
    }

    close_log = close_log;
    log_buf_size = log_buf_size;
    buf_ = new char[log_buf_size];
    memset(buf_, '\0', log_buf_size);
    split_lines_ = split_lines; // fix bug: split_line_  init here

    time_t t = time(nullptr);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    // https://www.runoob.com/cprogramming/c-function-strrchr.html
    const char *f_name = strrchr(file_name, '/');
    char log_full_name[300]= {0};
    if(f_name == nullptr){
        // https://www.runoob.com/cprogramming/c-function-snprintf.html
        snprintf(log_full_name, 299, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    } else {
        // eg: file_name: /usr/bin/log1.log
        // log_name=log1.log;
        // dir_name=/usr/bin/
        strcpy(log_name, f_name+1);
        strncpy(dir_name, file_name, f_name-file_name+1);
        // printf("%s%d_%02d_%02d_%s---------------\n", dir_name, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name);
        snprintf(log_full_name, 299, "%s%d_%02d_%02d_%s", dir_name, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name);
    }

    today = my_tm.tm_mday;
    fp_ = fopen(log_full_name, "a");
    if(fp_ == nullptr){
        return false;
    }
    // std::cout<<"log init success"<<std::endl;
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
        strcpy(s, "[debug]:");
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

    mutex_.lock();
    count_++;
    if(today != my_tm.tm_mday || count_ % split_lines_ ==0){//everyday log
        char new_log[300] = {0};
        fflush(fp_);
        fclose(fp_);
        char tail[16] = {0};
        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
        if(today !=my_tm.tm_mday) {
            snprintf(new_log, 299, "%s%s%s", dir_name, tail, log_name);
            today = my_tm.tm_mday;
            count_=0;
        } else {
            snprintf(new_log, 299, "%s%s%s.%lld", dir_name, tail, log_name, count_/split_lines_);
        }
        // a=add, if the file not found, create.
        fp_ = fopen(new_log, "a");
    }
    mutex_.unlock();

    va_list valist;
    va_start(valist, format);
    std::string log_str;// the true output log string.

    mutex_.lock();
    // 写入时间
    int n = snprintf(buf_, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s", my_tm.tm_year + 1900, my_tm.tm_mon+1, my_tm.tm_mday,
                    my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);
    // https://langzi989.github.io/2018/01/01/C%E4%B8%ADsnprintf%E4%B8%8Evsnprintf%E5%87%BD%E6%95%B0/
    int m = vsnprintf(buf_+n, log_buf_size_-1, format, valist);
    buf_[n+m] = '\n';
    buf_[n+m+1] = '\0';
    log_str = buf_;
    // std::cout<<"write_log fini"<<std::endl;
    mutex_.unlock();

    if(is_async_ && !log_queue_->full()) {
        log_queue_->push(log_str);
    } else {
        mutex_.lock();
        // std::cout<<log_str<<std::endl;
        fputs(log_str.c_str(), fp_);
        // if(a>0){
        //     std::cout<<"write_log fini"<<std::endl;
        // }
        mutex_.unlock(); // fix bug: unlock();
    }
    // std::cout<<"write_log fini"<<std::endl;
    va_end(valist);
}

void Log::flush(void){
    mutex_.lock();
    fflush(fp_);
    mutex_.unlock();
}

} // namesapce webserver