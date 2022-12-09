#include "httpconn.h"

namespace webserver
{
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";

void HttpConn::init_mysql_result(ConnPool *connPool){
    sql::Connection *con = connPool->GetConnection();
    connectionRAII mysqlcon(&mysql, connPool);
    std::string sql = "select name,password from user";
    sql::Statement *stat = mysql->createStatement();
    sql::ResultSet *rs = stat->executeQuery(sql);
    if(rs->getFetchSize()==0){
        LOG_ERROR("select error:%s\n");
    }
    while(rs->next()){
        std::string name = rs->getString(1);
        std::string password = rs->getString(2);
        users_[name] = password;
    }

    delete stat;
    delete rs;
}



} // namespace webserver
