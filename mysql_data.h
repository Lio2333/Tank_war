#include <iostream>
#include <string>
#include <mysql/mysql.h>

class MysqlDB
{
private:
	MYSQL mysql;
	MYSQL_ROW row;
	MYSQL_RES *result;
	MYSQL_FIELD *field;

public:
	MysqlDB();
	~MysqlDB();
	void connect(std::string host, std::string user, std::string passwd, std::string database);
	bool check(std::string name, std::string passwd);
	int  checkuid(std::string name,std::string passwd);
};

