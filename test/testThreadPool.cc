#include "../threadpool/threadpool.h"
#include "../CGImysql/connpool.h"
#include <cppconn/resultset.h>
#include <iostream>
using namespace std;
using namespace webserver;
ConnPool *connPool = ConnPool::GetInstance();
int main(){
    connPool->init("tcp://127.0.0.1:3306", "root", "123456", "users", 10, 0);
    sql::Connection *con;
	sql::Statement *stat;
    sql::ResultSet *rs;
    string sql = "select * from user";
    ThreadPool<int> *p = new ThreadPool<int>(0, connPool);
    p->worker(p);
    return 0;
}
/*
g++ -o testThreadPool testThreadPool.cc ../CGImysql/connpool.cc ../threadpool/threadpool.cc ../log/log.cc -lpthread -I/usr/include/cppconn -lmysqlcppconn
*/