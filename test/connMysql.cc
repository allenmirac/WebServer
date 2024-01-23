#include <iostream>
#include <stdlib.h>
#include <string>

#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_driver.h>
#include "../CGImysql/connpool.h"

using namespace std;
using namespace webserver;

ConnPool *connPool = ConnPool::GetInstance();
int main() {
    if(connPool != nullptr){
        connPool->init("tcp://127.0.0.1:3306", "root", "123456", "users", 10, 0);
        sql::Connection *con;
        sql::Statement *stat;
        sql::ResultSet *rs;
        string sql = "select * from user";
        // sql::mysql::MySQL_Driver *driver;
        // Create a connection
        // driver = sql::mysql::get_mysql_driver_instance();
        // con = driver->connect("tcp://127.0.0.1:3306", "root", "123456");
        // con->setSchema("users");
        cout<<connPool->GetFreeConn()<<endl;
        con = connPool->GetConnection();
        stat = con->createStatement();
        rs = stat->executeQuery(sql);
        while(rs->next()){
            cout<<"id = "<<rs->getInt(1)<<endl;
            cout<<"name = "<<rs->getString(2)<<endl;

        }
        cout<<connPool->GetFreeConn()<<endl;
        con = connPool->GetConnection();

        con = connPool->GetConnection();
        stat = con->createStatement();
        rs = stat->executeQuery(sql);
        while(rs->next()){
            cout<<"id = "<<rs->getInt(1)<<endl;
            cout<<"name = "<<rs->getString(2)<<endl;
        }
        connPool->ReleaseConnection(con);
        connPool->DestroyPool();
    }else{
        printf("GetInstance defeated!!!");
    }

    
    return 0;
}
/* 
g++ -o connMysql ../CGImysql/connpool.cc connMysql.cc -lmysqlcppconn  -I/usr/include/cppconn    ../lock/locker.h ../log/log.cc -L/usr/lib  -lpthread 
-Wall
 */
