/*
 * @Author: miracle
 * @Date: 2022-12-1 16.58
 * @LastEditTime: 2022-12-7
 * @LastEditors: miracle
 */

#ifndef WEBSERVER_SQLCONNECTIONPOOL_H
#define WEBSERVER_SQLCONNECTIONPOOL_H

#include <list>
#include <error.h>
#include <mysql_connection.h>
#include <string>
#include <mysql_driver.h>
#include "../lock/locker.h"
#include "../log/log.h"

namespace webserver
{
class ConnPool{
public:
    sql::Connection *GetConnection();
    bool ReleaseConnection(sql::Connection *conn);
    int GetFreeConn();
    void DestroyPool();
    static ConnPool *GetInstance();
    void init(std::string url, std::string user, std::string password, std::string dataBaseName, int maxConn, int close_log);
private:
    ConnPool();
    ~ConnPool();
private:
    int maxConn_;       /*max num*/
    int curConn_;       /*current index*/
    int freeConn_;      /*rest conn num*/
    locker lock;        /*lock*/
    sql::Driver* driver;/* driver */
    std::list<sql::Connection *> connList; /*pool*/
    static ConnPool* connPool;
    sql::Connection* CreateConnection();
    sem reserve;
public:
    std::string url_;          /*eg:"tcp://127.0.0.1:3306"*/
    std::string user_;         /*user name*/
    std::string password_;     /*password*/
    std::string databaseName_; /*database name*/
    int close_log_;            /*use log or not*/
};

class connectionRAII{
public:
    connectionRAII(sql::Connection ** con, ConnPool *connPool);
    ~connectionRAII();
private:
    sql::Connection *connRAII;
    ConnPool *poolRAII;
};

} // namespace webserver


#endif // WEBSERVER_SQLCONNECTIONPOOL_H