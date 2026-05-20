#include "crow.h"
#include <sqlite3.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include <ctime>

//This file handles main server start/stop

extern void setup_register_route(crow::SimpleApp& app, sqlite3* DB_connection, std::unordered_map<std::string, std::time_t>& ip_table);
extern void setup_password_update_route(crow::SimpleApp& app, sqlite3* DB_write_connection,  sqlite3* DB_read_connection, std::unordered_map<std::string, std::time_t>& ip_table);
extern void setup_team_delete_route(crow::SimpleApp& app, sqlite3* DB_write_connection,  sqlite3* DB_read_connection, std::unordered_map<std::string, std::time_t>& ip_table);
//extern void setup_details_route(crow::SimpleApp& app, sqlite3* DB_connection);
extern sqlite3* loadDB_write();
extern sqlite3* loadDB_read();
extern int setup_db(sqlite3* DB_connection);
std::unordered_map<std::string, std::time_t> ip_table;

int main(){
    crow::SimpleApp app;
    sqlite3* db_write_connection=loadDB_write();
    sqlite3* db_read_connection=loadDB_read();
    setup_db(db_write_connection);
    //Register port.
    printf("\n");
    setup_register_route(app, db_write_connection, ip_table);     // Use <ip>:8000/register
    setup_password_update_route(app, db_write_connection, db_read_connection, ip_table);		// Use <ip>:8000/pass_reset
    setup_team_delete_route(app, db_write_connection, db_read_connection, ip_table);		// Use <ip>:8000/pass_reset
//    setup_details_route(app, db_write_connection, db_read_connection, ip_table);     // Use <ip>:8000/details
    app.port(8000).multithreaded().run();
}
