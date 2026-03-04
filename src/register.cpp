#include "crow.h"
#include <string>
#include <format>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>

//This file handles registrations
//extern void run_and_print_DB_query(sqlite3* DB_connection, char* query);
std::string hash_password(std::string password);
int register_team(sqlite3* DB_connection, const std::string& username, const std::string& password_hashed, const char* statement);
//extern sqlite3* db_connection;
//extern char query[];

void setup_register_route(crow::SimpleApp& app, sqlite3* DB_connection){
    CROW_ROUTE(app, "/register")
        .methods("POST"_method)
        ([DB_connection](const crow::request& req){
            auto body=crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid JSON");
            if (!body.has("username") || !body.has("password")){
                return crow::response(400, "Missing usernsme or password");
            }
            std::string username=body["username"].s();
            std::string password=body["password"].s();
            //Hash password
            std::string password_hashed=hash_password(password);
//          std::string password_hashed(reinterpret_cast<char*>(hash), hash_len);
            //std::cout<<username<<" : "<<password<<" : "<<password_hashed<<"\n";
            //Store in DB
            const char* query_statement="INSERT INTO auth_table (team_name, password) VALUES (?,?);";
            int status=register_team(DB_connection, username, password_hashed, query_statement);
            if (status==-1){
                return crow::response(500, "Internet server error: Could not register user\n");
            } else if (status==-2){
                return crow::response(500, "Internet server error: Username already taken\n");
            }
            //UNSAFE
            //const char* query=("INSERT INTO users (username, password) VALUES (?, ?);");
            return crow::response(200, "Account created successfully\n");
        });
    printf("Registration Route UP :\n");
    printf("Usage: POST -d");
}
