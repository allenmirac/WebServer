/*
 * @Author: miracle
 * @Date: 2022-12-1 21.15
 * @LastEditTime: 2022-12-1
 * @LastEditors: miracle
 */
 
 /**
  * use recycle queue: 
  * m_back = (m_back+1)%m_max_size;
  * m_front = (m_front+1)%m_max_size;
  */

#ifndef WEBSERVER_BLOCKQUEUE_H
#define WEBSERVER_BLOCKQUEUE_H

#include <pthread.h>
#include <stdlib.h> /*exit*/
#include "../lock/locker.h"
#include <sys/time.h>

namespace webserver
{
template <typename T>
class block_queue{
public:
    block_queue(int max_size=100){
        if(max_size <=0){
            exit(-1);
        }
        m_max_size = max_size;
        m_array = new T[max_size];
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }
    void clear(){
        m_mutex.lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        m_mutex.unlock();
    }

    ~block_queue(){
        m_mutex.lock();
        if(m_array != nullptr){
            delete[] m_array;
        }
        m_mutex.unlock();
    }
    bool full(){
        m_mutex.lock();
        if(m_size >= m_max_size) {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    bool empty(){
        m_mutex.lock();
        if(m_size == 0) {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    bool front(T& val){
        m_mutex.lock();
        if(0 == m_size){
            m_mutex.unlock();
            return false;
        }
        val = m_array[m_front];
        m_mutex.unlock();
        return true;
    }

    bool back(T& val){
        m_mutex.lock();
        if(0 == m_size){
            m_mutex.lock();
            return false;
        }
        val = m_array[m_back];
        m_mutex.unlock();
        return true;
    }

    int size(){
        int temp=0;
        m_mutex.lock();
        temp=m_size;
        m_mutex.unlock();
        return temp;
    }

    int max_size(){
        int temp=0;
        m_mutex.lock();
        temp=m_max_size;
        m_mutex.unlock();
        return temp;
    }

    bool push(const T& item){
        m_mutex.lock();
        if(m_size >= m_max_size){
            m_cond.broadcast();
            m_mutex.unlock();
            return false;
        }
        m_back = (m_back+1)%m_max_size;
        m_size +=1;
        m_array[m_back] = std::move(item);//!!!!!!!!!!!!!!!!!!!!
        m_cond.broadcast();
        m_mutex.unlock();
        return true;
    }

    bool pop(T& item){
        m_mutex.lock();
        while(m_size <= 0){
            if(!m_cond.wait(&m_mutex)){
                m_mutex.unlock();
                return false;
            }
        }
        m_front = (m_front+1)%m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }

    bool pop(T &item, int ms_timeout)
    {
        struct timespec t = {0, 0};
        struct timeval now = {0, 0};
        gettimeofday(&now, NULL);
        m_mutex.lock();
        if (m_size <= 0) {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if (!m_cond.timewait(&m_mutex, t)) {
                m_mutex.unlock();
                return false;
            }
        }
        if (m_size <= 0) {
            m_mutex.unlock();
            return false;
        }
        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }
private:
    locker m_mutex;
    cond m_cond;

    T* m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};
} // namespace webserver
#endif // WEBSERVER_BLOCKQUEUE_H
