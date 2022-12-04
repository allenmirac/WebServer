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
    m_url=url;
    m_user=user;
    m_password=password;
    m_databaseName=dataBaseName;
    m_port=port;
    m_close_log=close_log;

    for(int i=0; i<maxConn; i++){
        MYSQL *con = nullptr;
        con = mysql_init(con);
        if(nullptr == con){
            LOG_ERROR("Mysql Error");
            exit(1);
        }
        con = mysql_real_connect(con, url.c_str(), user.c_str(), password.c_str(), dataBaseName.c_str(), port, NULL, 0);
        if(nullptr == con){
            LOG_ERROR("Mysql Error");
            exit(1);
        }
        connList.push_back(con);
        ++m_freeConn;
    }
    reserve = sem(m_freeConn);
    m_maxConn=m_freeConn;
}

MYSQL *connection_pool::GetConnection(){
    MYSQL* con=nullptr;
    if(0 == connList.size()){
        return nullptr;
    }
    reserve.wait();

    lock.lock();
    con = connList.front();
    connList.pop_front();
    --m_freeConn;
    ++m_curConn;
    lock.unlock();

    return con;
}

bool connection_pool::ReleaseConnection(MYSQL* con){
    if(nullptr == con){
        return false;
    }

    lock.lock();
    connList.push_back(con);// the size of connList can't be change.
    ++m_freeConn;
    --m_curConn;
    lock.unlock();

    reserve.post();
    return true;
}

void connection_pool::DestroyPool(){
    lock.lock();
    if(connList.size() > 0){
        for(auto it=connList.begin(); it!=connList.end(); it++){
            MYSQL *con = *it;
            mysql_close(con);
        }
        m_curConn=0;
        m_maxConn=0;
        connList.clear();
    }
    lock.unlock();
}

connection_pool::~connection_pool(){
    DestroyPool();
}

connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *pool) {
    *SQL = pool->GetConnection();

    connRAII = *SQL;
    poolRAII = pool;
}

connectionRAII::~connectionRAII(){
    poolRAII->ReleaseConnection(connRAII);
}