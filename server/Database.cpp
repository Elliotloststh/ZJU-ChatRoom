#include "Database.hpp"
#include <string>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <mariadb/mysql.h>

using std::string;
using std::cerr;
using std::endl;
using std::cout;
using namespace boost::property_tree;

DB_Connection::DB_Connection():conn(nullptr), res(nullptr){}

DB_Connection::~DB_Connection(){
    mysql_free_result(res);
    mysql_close(conn);
}
bool DB_Connection::DB_init(const string& filename){
    conn = mysql_init(NULL);
    if (!conn){
        cerr << mysql_error(conn) << endl;
        return false;
    }
    ptree root;
    read_json(filename, root);
    auto host = root.get<string>("Host");  
    auto user = root.get<string>("User");
    auto passwd = root.get<string>("Password");
    auto database = root.get<string>("Database");
    auto socket = root.get<string>("Socket");
    auto r = mysql_real_connect(
        conn,
        host.data(),
        user.data(),
        passwd.data(),
        database.data(),
        root.get<unsigned int>("Port"),
        socket.data(),
        root.get<unsigned long>("ClientFlag")
    );
    
    if (!r){
        cerr << mysql_error(conn) << endl;
        return false;
    }
    return true;
}

return_rows DB_Connection::query(const string& s){
    return_rows rr;
    mysql_query(conn, s.data());
    res = mysql_store_result(conn);
    if (res == NULL){
        cout << mysql_error(conn) << endl;
        return rr;
    }
    rr.fields = mysql_num_fields(res);
    rr.length = mysql_num_rows(res);
    rr.rows = new MYSQL_ROW[rr.length];
    for (int i = 0; i < rr.length; ++i)
        rr.rows[i] = mysql_fetch_row(res);
    return rr;
}

bool DB_Connection::insert(const string& s){
    auto res = mysql_query(conn, s.data());
    if (!res){
        cerr << mysql_error(conn) << endl;
    }
    return res == 0 ? true : false;
}

bool DB_Connection::update(const string& s){
    return insert(s);
}

bool DB_Connection::del(const string& s){
    return insert(s);
}

// #include <cassert>
// int main(){
//     //test function
//     DB_Connection conn;
//     auto r = conn.DB_init("DB_config.json");
//     assert(r == true);
//     auto res = conn.query("select userid from User where userid = 'testid1';");
//     for (auto i = 0; i < res.length; ++i){
//         for (auto j = 0; j < res.fields; ++j){
//             cout << res.rows[i][j] << " | ";
//         }
//         cout << endl;
//     }
// }
