#include "sql_connection_pool.h"
using namespace webserver;

connection_pool::connection_pool()
    :m_curConn(0), m_freeConn(0){
    
}

connection_pool *connection_pool::GetInstance(){
    static connection_pool connPool;
    return &connPool;
}

void connection_pool::init(std::string url, std::string user, std::string password, std::string dataBaseName, int port, int maxConn, int close_log) {
    
}