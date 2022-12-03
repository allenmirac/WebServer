/*
 * @Author: miracle
 * @Date: 2022-12-1 16.58
 * @LastEditTime: 2022-12-1
 * @LastEditors: miracle
 */

#ifndef WEBSERVER_SQLCONNECTIONPOOL_H
#define WEBSERVER_SQLCONNECTIONPOOL_H

#include <list>
#include <error.h>
#include <mysql/mysql.h>
#include <string>
#include "../lock/locker.h"
#include "../log/log.h"

namespace webserver
{
class connection_pool{
public:
    MYSQL *GetConnection();
    bool ReleaseConnection(MYSQL *conn);
    int GetFreeConn();
    void DestroyPool();
    static connection_pool *GetInstance();
    void init(std::string url, std::string user, std::string password, std::string dataBaseName, int port, int maxConn, int close_log);
private:
    connection_pool();
    ~connection_pool();
private:
    int m_maxConn;  /*max num*/
    int m_curConn;  /*current index*/
    int m_freeConn; /*rest conn num*/
    locker lock;    /*lock*/
    std::list<MYSQL*> connList; /*pool*/
    sem reserve;
public:
    std::string m_url;          /*host address*/
    std::string m_user;         /*user name*/
    std::string m_password;     /*password*/
    std::string m_databaseName; /*database name*/
    std::string m_port;         /*MYSQL port*/
    int m_close_log;            /*use log or not*/
};
class connectionRAII{
public:
    connectionRAII(MYSQL** con, connection_pool *connPool);
    ~connectionRAII();
private:
    MYSQL *connRAII;
    connection_pool *poolRAII;
};

} // namespace webserver


#endif // WEBSERVER_SQLCONNECTIONPOOL_H