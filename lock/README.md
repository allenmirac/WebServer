# Locker
多线程同步
> 信号量:semaphore
> 互斥锁:mutex
> 条件变量:not C++11, condition_variable

 使用pthread_cond_wait方式如下：
    pthread _mutex_lock(&mutex)
    while或if(线程执行的条件是否成立)
          pthread_cond_wait(&cond, &mutex);
    线程执行
    pthread_mutex_unlock(&mutex);
条件不满足，就会执行pthread_cond_wait，这个时候等把这个任务放入等待队列后就会自动将这个锁归还。
