#include "../threadpool/threadpool.h"
#include "../CGImysql/connpool.h"
#include <iostream>
using namespace std;
using namespace webserver;
ConnPool *connPool = ConnPool::GetInstance();
int main(){
    connPool->init("tcp://127.0.0.1:3306", "root", "123456", "users", 10, 0);
    ThreadPool<int> *p = new ThreadPool<int>(0, connPool);
    int a=123;
    p->append_p(&a);
    p->worker(p);
    return 0;
}
/*
注释掉：threadpool.h的110、118-134、137-138，run函数的while true循环也得注释；
取消注释：112、139-143；
输出：
hahaha
0
quchulaile 123

g++ -o testThreadPool testThreadPool.cc ../CGImysql/connpool.cc ../threadpool/threadpool.h ../log/log.cc -lpthread -I/usr/include/cppconn -lmysqlcppconn -g
*/