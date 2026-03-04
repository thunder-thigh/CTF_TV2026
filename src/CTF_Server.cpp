#include "crow.h"
#include <sqlite3.h>
//This file handles main server start/stop

extern void setup_register_route(crow::SimpleApp& app, sqlite3* DB_connection);
extern void run_and_print_DB_query(sqlite3* DB_connection, char* query);
extern sqlite3* loadDB();
int setup_db(sqlite3* DB_connection);

int main(){
    crow::SimpleApp app;
    sqlite3* db_signup_connection=loadDB();
    setup_db(db_signup_connection);
//    char query[]="SELECT * FROM users";
//    char* query="SELECT * FROM users";
//    run_and_print_DB_query(db_connection, query);
    setup_register_route(app, db_signup_connection);
    app.port(8000).multithreaded().run();
}
