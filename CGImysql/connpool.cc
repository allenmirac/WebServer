#include "connpool.h"

namespace webserver
{

ConnPool* ConnPool::connPool = nullptr;

ConnPool::ConnPool()
    :curConn_(0), freeConn_(0){
    
}

ConnPool *ConnPool::GetInstance(){
    if(nullptr == connPool){
        connPool = new ConnPool();
    }
    return connPool;
}

void ConnPool::init(std::string url, std::string user, std::string password, std::string dataBaseName, int maxConn, int close_log) {
    url_=url;
    user_=user;
    password_=password;
    databaseName_=dataBaseName;
    close_log_=close_log;
    // try {
        this->driver = sql::mysql::get_mysql_driver_instance();
    // } catch (sql::SQLException& e) {
    //     perror("get driver error.\n");
    // }
    //  catch (std::runtime_error& e) {
    //     perror("[ConnPool] run time error.\n");
    // }
    // create (maxConn/2) connections.
    for(int i=0; i<maxConn/2; i++){
        sql::Connection *con = nullptr;
        con = this->CreateConnection();
        if(nullptr == con){
            LOG_ERROR("sql::Connection Error");
            exit(1);
        }
        if(nullptr == con){
            LOG_ERROR("sql::Connection Error");
            exit(1);
        }
        connList.push_back(con);
        ++freeConn_;
    }
    reserve = sem(freeConn_);
    maxConn_=freeConn_;
}

sql::Connection *ConnPool::GetConnection(){
    sql::Connection* con=nullptr;
    if(0 == connList.size()){
        return nullptr;
    }
    reserve.wait();

    lock.lock();
    con = connList.front();
    connList.pop_front();
    --freeConn_;
    ++curConn_;
    lock.unlock();

    return con;
}

bool ConnPool::ReleaseConnection(sql::Connection* con){
    if(nullptr == con){
        return false;
    }

    lock.lock();
    connList.push_back(con);// give back the con
    ++freeConn_;
    --curConn_;
    lock.unlock();

    reserve.post();
    return true;
}

int ConnPool::GetFreeConn(){
    return freeConn_;
}

void ConnPool::DestroyPool(){
    lock.lock();
    if(connList.size() > 0){
        for(auto it=connList.begin(); it!=connList.end(); it++){
            sql::Connection *con = *it;
            con->close();
        }
        curConn_=0;
        maxConn_=0;
        connList.clear();
    }
    lock.unlock();
}
sql::Connection *ConnPool::CreateConnection(){
    sql::Connection *conn;
    // try {
        conn = driver->connect(url_, user_, password_);
        conn->setSchema(databaseName_);
    // }catch(sql::SQLException& e){
    //     e.what();
    // } catch(sql::SQLException& e){
    //     e.what();
    // }
    return conn;
}
ConnPool::~ConnPool(){
    DestroyPool();
}

connectionRAII::connectionRAII(sql::Connection **SQL, ConnPool *pool) {
    *SQL = pool->GetConnection();

    connRAII = *SQL;
    poolRAII = pool;
}

connectionRAII::~connectionRAII(){
    poolRAII->ReleaseConnection(connRAII);
}

} // namespace webserver