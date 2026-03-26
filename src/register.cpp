#include "crow.h"
#include <string>
#include <format>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>

//This file handles registrations
std::string hash_password(std::string password);
//args[4]. 0 = column to fetch, 1 = table name, 2 = constraint column, 3 = constraint value
int register_team(sqlite3* DB_write_connection, std::string& username, std::string& password_hashed, const char* statement);
int check_password(sqlite3* DB_read_connection, std::string team_name, std::string password_hashed, const char* statement);
int reset_password(sqlite3* DB_write_connection, std::string team_name, std::string new_pass_hashed, const char* statement);
bool check_rate_limit(std::unordered_map<std::string, std::time_t>& ip_table, std::string IP);

/*
void verify_password(sqlite3* DB_connection, std::string password, std::string team_name){
    std::string hash=hash_password(password);
    std::string arguments[]={"password", "auth_table", "team_name", team_name};
    if (fetch_single_string_from_DB(DB_connection, arguments)==password){
        std::cout<<"True"<<std::endl;
    } else std::cout<<"False"<<std::endl;
}
*/
void setup_register_route(crow::SimpleApp& app, sqlite3* DB_write_connection, std::unordered_map<std::string, std::time_t>& ip_table){
    CROW_ROUTE(app, "/register")
        .methods("POST"_method)
        ([DB_write_connection, &ip_table](const crow::request& req){
			/*
			if(check_rate_limit(ip_table, req.remote_ip_address)==false){
				return crow::response(429, "Too many requests, Account creation failed\n");
			}
			*/
            auto body=crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid JSON");
            if (!body.has("team_name") || !body.has("password")){
                return crow::response(400, "Missing team_name or password");
            }
            std::string team_name=body["team_name"].s();
            std::string password=body["password"].s();
            //Hash password
            std::string password_hashed=hash_password(password);
//          std::string password_hashed(reinterpret_cast<char*>(hash), hash_len);
            //std::cout<<username<<" : "<<password<<" : "<<password_hashed<<"\n";
            //Store in DB
            const char* register_statement="INSERT INTO auth_table (team_name, password) VALUES (?,?);";
            int status=register_team(DB_write_connection, team_name, password_hashed, register_statement);
            if (status==-1){
                return crow::response(500, "Internet server error: Could not register user\n");
            } else if (status==-2){
                return crow::response(500, "Internet server error: Username already taken\n");
            }
            //UNSAFE
            //const char* query=("INSERT INTO users (username, password) VALUES (?, ?);");
            return crow::response(200, "Account created successfully\n");
        });
    printf("[Registration Route]: UP\n");
    printf("Usage: POST <ip>/register -d '{\"team_name\":\"xyz\",\"password\":\"sicrit\"}'\n\n");
}

void setup_password_reset_route(crow::SimpleApp& app, sqlite3* DB_write_connection,  sqlite3* DB_read_connection, std::unordered_map<std::string, std::time_t>& ip_table){
    CROW_ROUTE(app, "/pass_reset")
        .methods("POST"_method)
        ([DB_write_connection, DB_read_connection, &ip_table](const crow::request& req){
            auto body=crow::json::load(req.body);
            if(check_rate_limit(ip_table, req.remote_ip_address)==false){
				return crow::response(429, "Too many requests, slow down\n");
			}
			if (!body) return crow::response(400, "Invalid JSON");
            if (!body.has("team_name") || !body.has("password") || !body.has("new_password")){
                return crow::response(400, "Missing team_name/password or new_password\n");
            }
            std::string team_name=body["team_name"].s();
            std::string password=body["password"].s();
            //Hash password and verify
            std::string password_hashed=hash_password(password);
			const char* verify_password_statement="SELECT COUNT() FROM auth_table WHERE team_name=? AND password=?;";
            int pass_state=check_password(DB_read_connection, team_name, password_hashed, verify_password_statement);
			if (pass_state==-1){
				return crow::response(500, "Internal Server Error: Password could not be verified.\n");
			} else if (pass_state==0){
				return crow::response(401, "Wrong credentials\n");
			}
			//Push new pass to DB
            std::string new_pass=body["new_password"].s();
            std::string new_pass_hashed=hash_password(new_pass);
            const char* reset_password_statement="UPDATE auth_table SET password=? WHERE team_name=?;";
            int status=reset_password(DB_write_connection, team_name, new_pass_hashed, reset_password_statement);
            if (status==-1){
                return crow::response(500, "Internet server error: Verified, but could not update password\n");
            }
            //UNSAFE
            //const char* query=("INSERT INTO users (username, password) VALUES (?, ?);");
            return crow::response(200, "Account created successfully\n");
        });
    printf("[Pass Reset Route]: UP\n");
    printf("Usage: POST <ip>/pass_reset -d '{\"team_name\":\"xyz\",\"password\":\"sicrit\",\"new_password\":\"top_sicrit\"}'\n\n");
}
