#include <iostream>
#include <mysql/mysql.h>

using namespace std;

int main(int argc, char *argv[])
{
	MYSQL conn;
	mysql_init(&conn);

	if (!mysql_real_connect(&conn, "localhost", "root", "", "user", 0, NULL, 0))
	{
		cout << "mysql connect failed" << endl;
		exit(-1);
	}
	cout << "mysql conenct success" << endl;
	mysql_close(&conn);
	return 0;
}
/* 
g++ -o connMysql connMysql.cc -I /usr/include/mysql -lmysqlclient
 */