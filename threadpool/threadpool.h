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
#include "../CGImysql/sql_connection_pool.h"

namespace webserver
{
template <typename T>
class threadpool{
public:
    threadpool(int actor_model, connection_pool* connection_pool, int thread_number = 8, int max_requests = 10000);
    ~threadpool();

    bool append(T* request, int state);
    bool append_p(T* request);
private:
    static void *worker(void*arg);
    void run();
private:
    int m_thread_number;        // number of thread;
    int m_max_requests;         // max request number;
    pthread_t *m_threads;       // thread pool id array
    std::list<T*> m_workqueue;  // request list
    locker m_queuelocker;       // mutex
    sem m_queuestat;            // signal of work
    connection_pool *m_connPool;// 
    int m_actor_model;          // model switch
};
} // namespace webserver

#endif // WEBSERVER_THREADPOOL_H