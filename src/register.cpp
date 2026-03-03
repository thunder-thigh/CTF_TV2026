#include "crow.h"
#include <string>
#include <format>
#include <sqlite3.h>

//This file handles registrations

extern void run_and_print_DB_query(sqlite3* DB_connection, char* query);
//extern sqlite3* db_connection;
//extern char query[];

void setup_register_route(crow::SimpleApp& app, sqlite3* DB_connection){
    CROW_ROUTE(app, "/register")
        .methods("POST"_method)
        ([&DB_connection](const crow::request& req){
            auto body=crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid JSON");
            if (!body.has("username") || !body.has("password")){
                return crow::response(400, "Missing usernsme or password");
            }
            std::string username=body["username"].s();
            std::string password=body["password"].s();
            const char* query=("INSERT INTO users (username, password) VALUES (?, ?);"  
            //std::cout<<username<<password<<"\n";
            //Hash pass
            //Store in DB
            return crow::response(200, "Account created successfully\n");
        });
    printf("Registration Route UP :\n");
    printf("Usage: POST -d");
}
