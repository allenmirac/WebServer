/*
 * @Author: miracle
 * @Date: 2022-12-1 16.49
 * @LastEditTime: 2022-12-1
 * @LastEditors: miracle
 */

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <exception>
#include <pthread.h>
#include <list>
#include "../lock/locker.h"
#include "../CGImysql/connpool.h"

namespace webserver // 模板类的定义和实现不能分开
{
// 定义
template <typename T>
class ThreadPool{
public:
    ThreadPool(int actor_model, ConnPool* connection_pool, int thread_number = 8, int max_requests = 10000);
    ~ThreadPool();

    bool append(T* request, int state);
    bool append_p(T* request);
    static void *worker(void*arg);
private:
    void run();
private:
    int m_thread_number;        // number of thread;
    int m_max_requests;         // max request number;
    pthread_t *m_threads;       // thread pool id array
    std::list<T*> m_workqueue;  // request list
    locker m_queuelocker;       // mutex
    sem m_queuestat;            // signal of work
    ConnPool *m_connPool;// 
    int m_actor_model;          // model switch
};

// 实现
template <typename T>
ThreadPool<T>::ThreadPool(int actor_model, ConnPool*connPool, int thread_number, int max_requests)
    :m_actor_model(actor_model),
    m_connPool(connPool), 
    m_thread_number(thread_number), 
    m_max_requests(max_requests),
    m_threads(nullptr) {
    if(thread_number<=0 || max_requests <=0){
        throw std::exception();
    }
    m_threads = new pthread_t[m_thread_number];
    if(!m_threads){
        throw std::exception();
    }
    for(int i=0; i<thread_number; i++){
        if(pthread_create(m_threads + i, NULL, worker, this) != 0){
            delete[] m_threads;
        }
        if(pthread_detach(m_threads[i])){
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <typename T>
ThreadPool<T>::~ThreadPool(){
    if(!m_threads){
        delete[] m_threads;
    }
}

template <typename T>
bool ThreadPool<T>::append(T *request, int state){
    m_queuelocker.lock();
    if(m_workqueue.size() >= m_max_requests){
        m_queuelocker.unlock();
        return false;
    }
    request->state_ = state;
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template <typename T>
bool ThreadPool<T>::append_p(T *request){
    m_queuelocker.lock();
    if(m_workqueue.size() >= m_max_requests){
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template <typename T>
void ThreadPool<T>::run(){
    while(true){
        m_queuestat.wait();
        m_queuelocker.lock();

        if(m_workqueue.empty()){
            m_queuelocker.unlock();
            continue;
        }
        // std::cout<<"m_workqueue size: "<<m_workqueue.size()<<std::endl;
        T * request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        // printf("m_actor_model:%d\n", m_actor_model);
        if(1==m_actor_model){
            if(0 == request->state_){
                if(request->read_once()){
                    request->improv = 1;
                    connectionRAII mysqlconn(&request->mysql, m_connPool);
                } else {
                    request->improv = 1;
                    request->timer_flag=1;
                }
            } else {
                if(request->write()){
                    request->improv=1;
                } else {
                    request->improv=1;
                    request->timer_flag=1;

                }
            }
        }
        else {
            connectionRAII mysqlconn(&request->mysql, m_connPool);
            // std::cout<<"request->process()"<<std::endl;
            request->process();
            // sql::Connection *conn = m_connPool->GetConnection();
            // if(conn!=nullptr){
            //     printf("quchulaile");
            //     printf(" %d\n", *request);
            // }
        }
    }
}

template <typename T>
void *ThreadPool<T>::worker(void*arg){
    ThreadPool *pool = static_cast<ThreadPool *> (arg);
    // std::cout<<"ThreadPool<T>::worker run"<<std::endl;
    pool->run();
    return pool;
}
} // namespace webserver

#endif // WEBSERVER_ThreadPool_H