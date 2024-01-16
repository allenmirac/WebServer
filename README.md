# WebServer
Linux下C++的轻量级的web服务器

## Proactor 异步 I/O
举个你去饭堂吃饭的例子，你好比应用程序，饭堂好比操作系统。

阻塞 I/O 好比，你去饭堂吃饭，但是饭堂的菜还没做好，然后你就一直在那里等啊等，等了好长一段时间终于等到饭堂阿姨把菜端了出来（数据准备的过程），但是你
还得继续等阿姨把菜（内核空间）打到你的饭盒里（用户空间），经历完这两个过程，你才可以离开。

非阻塞 I/O 好比，你去了饭堂，问阿姨菜做好了没有，阿姨告诉你没，你就离开了，过几十分钟，你又来饭堂问阿姨，阿姨说做好了，于是阿姨帮你把菜打到你的饭盒
里，这个过程你是得等待的。

异步 I/O 好比，你让饭堂阿姨将菜做好并把菜打到饭盒里后，把饭盒送到你面前，整个过程你都不需要任何等待。很明显，异步 I/O 比同步 I/O 性能更好，因为异
步 I/O 在「内核数据准备好」和「数据从内核空间拷贝到用户空间」这两个过程都不用等待。

Proactor 正是采用了异步 I/O 技术，所以被称为异步网络模型。

## 定时器

Intro:
https://blog.csdn.net/qq_44859952/article/details/121291792

Utils::sig_handler:
`errno` 是一个全局变量，通常用于表示函数调用中发生的错误。它是一个整数，被设置为表示特定错误代码的值。errno 的值通常由系统调用或库函数在发生错误时进行设置，以便通知程序发生了什么问题。

cb_func: 定时器回调函数:从内核事件表删除非活动连接在socket上的注册事件，关闭文件描述符，释放连接资源, httpconn减少连接数.

## locker

互斥锁（locker 使用 Mutex）、信号量（sem 使用 Semaphore）和条件变量（cond 使用 Condition Variable）是多线程编程中用于同步和协调线程之间操作的三种不同的同步机制。它们有一些区别和适用场景：

互斥锁（Mutex）：

>作用：用于提供对共享资源的独占访问，防止多个线程同时访问临界区。

>特点：只有拥有锁的线程可以访问被保护的资源。其他线程需要等待锁的释放。

>常见操作：lock（获取锁）、unlock（释放锁）。

>适用场景：适用于保护共享资源，防止并发访问引起的数据竞争。

>在本项目中用于connpool-Mysql连接池、httpconn-http连接处理、block_queue-阻塞队列、threadpool-线程池


信号量（Semaphore）：

>作用：用于控制对共享资源的访问数量，可以允许多个线程同时访问。

>特点：信号量维护一个计数器，线程可以尝试获取信号量，当计数器大于零时获取成功，计数器减一；当计数器为零时，线程需要等待。释放资源时，计数器增加，唤醒等待的线程。

>常见操作：sem_wait（等待信号量）、sem_post（发布信号量）。

>适用场景：适用于控制对有限资源的访问，如线程池中的线程数量。

>在本项目中用于connpool-Mysql连接池、threadpool-线程池

条件变量（Condition Variable）：

>作用：用于在线程之间传递条件信息，实现线程的等待和唤醒。

>特点：当某个条件不满足时，线程可以等待条件变量，以避免忙等。当条件满足时，线程可以通过条件变量唤醒等待的线程。

>常见操作：wait（等待条件变量）、signal（唤醒一个等待线程）、broadcast（唤醒所有等待线程）。

>适用场景：适用于线程间需要根据某些条件协调执行的情况，如生产者-消费者问题。

>在本项目中用于block_queue-阻塞队列

互斥锁用于保护共享资源的**独占**访问，信号量用于控制对资源的**访问数量**，条件变量用于**线程之间**的条件同步。这些机制可以结合使用，根据具体的多线程编程场景选择合适的同步方式。