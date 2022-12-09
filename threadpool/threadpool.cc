#include "threadpool.h"

namespace webserver
{
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
        throw std::exception;
    }
    for(int i=0; i<thread_number; i++){
        if(pthread_create(m_threads + i, NULL, worker, this) != 0){
            delete[] m_threads;
        }
        if(pthread_detach(m_threads[i])){
            delete[] m_threads;
            throw std::exception;
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
    request->m_state = state;
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
    return false;
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

        T * request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if(1==m_actor_model){
            if(0 == request->m_state){
                if(request->read_once()){
                    request->improv = 1;
                    connectionRAII mysqlconn(&request->mysql, m_connPool);
                } else {
                    request->improv = 1;
                    request->time_flag=1;
                }
            } else {
                if(request->write()){
                    request->improv=1;
                } else {
                    request->improv=1;
                    request->time_flag=1;

                }
            }
        }
        else {
            connectionRAII mysqlconn(&request->mysql, m_connPool);
            request->process();
        }
    }
}

template <typename T>
void *ThreadPool<T>::worker(void*arg){
    ThreadPool *pool = std::static_cast<ThreadPool *> arg;
    pool->run();
    return pool;
}
} // namespace webserver
