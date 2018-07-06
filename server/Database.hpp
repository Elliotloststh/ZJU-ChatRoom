#ifndef __DATABASE_H
#define __DATABASE_H
#include <mariadb/mysql.h>
#include <string>
using std::string;

struct return_rows{
    size_t length;
    size_t fields;
    MYSQL_ROW *rows;
};

class DB_Connection{
    public:
        DB_Connection();
        // read from config file
        bool DB_init(const string&);
        return_rows query(const string&);
        bool insert(const string&);
        bool update(const string&);
        bool del(const string&);
        ~DB_Connection();

    private:
        MYSQL *conn;
        MYSQL_RES *res;
};


#endif