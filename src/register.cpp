#include "crow.h"
#include <string>
#include <format>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>
#include <unordered_set>

//This file handles registrations, password rests, team deletion
std::string hash_password(std::string password);
int register_team(sqlite3* DB_write_connection, std::string& team_name, std::string& password_hashed/*, const char* statement*/);
int register_teammate(sqlite3* DB_write_connection, std::string& team_name, int mem_number, std::string& member_name, std::string& member_enrolment/*, const char* statement*/);
int check_password(sqlite3* DB_read_connection, std::string& team_name, std::string& password_hashed/*, const char* statement*/);
int update_password(sqlite3* DB_write_connection, std::string& team_name, std::string& new_pass_hashed/*, const char* statement*/);
int delete_team(sqlite3* DB_write_connection, std::string& team_name/*, const char* delete_team_statement*/);
int delete_teammate(sqlite3* DB_write_connection, std::string& team_name, int mem_number/*, const char* delete_team_statement*/);
bool check_rate_limit(std::unordered_map<std::string, std::time_t>& ip_table, std::string IP);
const std::unordered_set<int> allowed_team_members {0,1,2,3};

void setup_register_route(crow::SimpleApp& app, sqlite3* DB_write_connection, std::unordered_map<std::string, std::time_t>& ip_table){
    CROW_ROUTE(app, "/register")
        .methods("POST"_method)
        ([DB_write_connection, &ip_table](const crow::request& req){
			if(check_rate_limit(ip_table, req.remote_ip_address)==false){
				return crow::response(429, "Too many requests, Account creation failed\n");
			}
            auto body=crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid JSON\n");
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
            //const char* register_statement="BEGIN TRANSACTION;
											//INSERT INTO auth_table (team_name, password) VALUES (?,?);
											//INSERT INTO team_details (team_name) VALUES (?)";
											//COMMIT TRANSACTION;"
            int status=register_team(DB_write_connection, team_name, password_hashed/*, register_statement*/);
            if (status==-1){
                return crow::response(401, "Internet server error: Could not register user\n");
            } else if (status==-2){
                return crow::response(401, "Internet server error: Username already taken\n");
            }
            //UNSAFE
            //const char* query=("INSERT INTO users (username, password) VALUES (?, ?);");
            return crow::response(200, "Account created successfully\n");
        });
    printf("[Registration Route]: UP\n");
    printf("Usage: POST curl 127.0.0.1:8000/register -d '{\"team_name\":\"xyz\",\"password\":\"sicrit\"}'\n\n");
}

void setup_password_update_route(crow::SimpleApp& app, sqlite3* DB_write_connection,  sqlite3* DB_read_connection, std::unordered_map<std::string, std::time_t>& ip_table){
    CROW_ROUTE(app, "/register")
        .methods("PATCH"_method)
        ([DB_write_connection, DB_read_connection, &ip_table](const crow::request& req){
            auto body=crow::json::load(req.body);
            if(check_rate_limit(ip_table, req.remote_ip_address)==false){
				return crow::response(429, "Too many requests, slow down\n");
			}
			if (!body) return crow::response(400, "Invalid JSON\n");
            if (!body.has("team_name") || !body.has("password") || !body.has("new_password")){
                return crow::response(400, "Missing team_name/password or new_password\n");
            }
            std::string team_name=body["team_name"].s();
            std::string password=body["password"].s();
            //Hash password and verify
            std::string password_hashed=hash_password(password);
			//const char* verify_password_statement="SELECT COUNT() FROM auth_table WHERE team_name=? AND password=?;";
            int pass_state=check_password(DB_read_connection, team_name, password_hashed/*, verify_password_statement*/);
			if (pass_state==-1){
				return crow::response(500, "Internal Server Error: Password could not be verified.\n");
			} else if (pass_state==0){
				return crow::response(401, "Wrong credentials OR team does not exist\n");
			}
			//Push new pass to DB
            std::string new_pass=body["new_password"].s();
            std::string new_pass_hashed=hash_password(new_pass);
            //const char* update_password_statement="UPDATE auth_table SET password=? WHERE team_name=?;";
            int status=update_password(DB_write_connection, team_name, new_pass_hashed/*, update_password_statement*/);
            if (status==-1){
                return crow::response(500, "Internet server error: Verified, but could not update password\n");
            }
            //UNSAFE
            //const char* query=("INSERT INTO users (username, password) VALUES (?, ?);");
            return crow::response(200, "Password updated successfully\n");
        });
    printf("[Pass Reset Route]: UP\n");
    printf("Usage: PATCH curl 127.0.0.1:8000/register -d '{\"team_name\":\"xyz\",\"password\":\"sicrit\",\"new_password\":\"top_sicrit\"}'\n\n");
}

void setup_team_delete_route(crow::SimpleApp& app, sqlite3* DB_write_connection,  sqlite3* DB_read_connection, std::unordered_map<std::string, std::time_t>& ip_table){
    CROW_ROUTE(app, "/register")
        .methods("DELETE"_method)
        ([DB_write_connection, DB_read_connection, &ip_table](const crow::request& req){
            auto body=crow::json::load(req.body);
            if(check_rate_limit(ip_table, req.remote_ip_address)==false){
				return crow::response(429, "Too many requests, slow down\n");
			}
			if (!body) return crow::response(400, "Invalid JSON\n");
            if (!body.has("team_name") || !body.has("password")){
                return crow::response(400, "Missing team_name/password or new_password\n");
            }
            std::string team_name=body["team_name"].s();
            std::string password=body["password"].s();
            //Hash password and verify
            std::string password_hashed=hash_password(password);
			//const char* verify_password_statement="SELECT COUNT() FROM auth_table WHERE team_name=? AND password=?;";
            int pass_state=check_password(DB_read_connection, team_name, password_hashed/*, verify_password_statement*/);
			if (pass_state==-1){
				return crow::response(500, "Internal Server Error: Password could not be verified.\n");
			} else if (pass_state==0){
				return crow::response(401, "Wrong credentials OR team does not exist\n");
			}
            //const char* delete_team_statement="DELETE FROM auth_table WHERE team_name=?;";
            int status=delete_team(DB_write_connection, team_name/*, delete_team_statement*/);
            if (status==-1){
                return crow::response(500, "Internet server error: Verified, but could delete team\n");
            }
            //UNSAFE
            //const char* query=("INSERT INTO users (username, password) VALUES (?, ?);");
            return crow::response(200, "Team deleted successfully\nSorry to see you go ;(\n");
        });
    printf("[Team Delete Route]: UP\n");
    printf("Usage: DELETE curl 127.0.0.1:8000/register -d '{\"team_name\":\"xyz\",\"password\":\"sicrit\"}'\n\n");
}

void setup_team_details_route(crow:: SimpleApp& app, sqlite3* DB_write_connection, sqlite3* DB_read_connection, std::unordered_map<std::string, std::time_t>& ip_table){
	CROW_ROUTE(app, "/team/member/<int>")
		.methods("PATCH"_method)
		([DB_write_connection, DB_read_connection, &ip_table](const crow::request& req, int mem_num){
			if (!allowed_team_members.count(mem_num)){
				return crow::response(400, "Resource not found, allowed members are: 0,1,2,3");
			}
			auto body=crow::json::load(req.body);
			if(check_rate_limit(ip_table, req.remote_ip_address)==false){
				return crow::response(429, "Too many requests, slow down\n");
			}
			if (!body) return crow::response(400, "Invalid JSON\n");
			if (!body.has("team_name") || !body.has("password")){
				return crow::response(400, "Missing team_name/password\n");
			}
			if (!body.has("mem_name") || !body.has("mem_enrol")){
				return crow::response(400, "Missing member_name/member_enrollment\n");
			}
			std::string team_name = body["team_name"].s();
			std::string password = body["password"].s();
			std::string mem_name = body["mem_name"].s();
			std::string mem_enrollment = body["mem_enrol"].s();
            std::string password_hashed=hash_password(password);
			int pass_state=check_password(DB_read_connection, team_name, password_hashed/*, verify_password_statement*/);
			if (pass_state==-1){
				return crow::response(500, "Internal Server Error: Password could not be verified.\n");
			} else if (pass_state==0){
				return crow::response(401, "Wrong credentials OR team does not exist\n");
			}
			//const char* add_detail_statement="UPDATE team_details SET memX_name=?, memX_enrol=? WHERE team_name=?;"
			int status=register_teammate(DB_write_connection, team_name, mem_num, mem_name, mem_enrollment);
			if (status==-1){
				return crow::response(500, "Password verified but could not add teammate. Internal Server Error\n");
			}
			return crow::response(200, "Team member added\n");
		});
    printf("[Member Details Route]: UP\n");
    printf("Usage: PATCH curl 127.0.0.1:8000/team/member/<int:0/1/2/3> -d '{\"team_name\":\"xyz\",\"password\":\"sicrit\"}',\"mem_name\":\"aashu\",\"mem_enrol\":\"0108CS241123\"}'\n\n");
}

void setup_team_details_delete_route(crow::SimpleApp& app, sqlite3* DB_write_connection, sqlite3* DB_read_connection, std::unordered_map<std::string, std::time_t>& ip_table){
	CROW_ROUTE(app, "/team/member/<int>")
		.methods("DELETE"_method)
		([DB_write_connection, DB_read_connection, &ip_table](const crow::request& req, int mem_num){
			if (!allowed_team_members.count(mem_num)){
				return crow::response(400, "Resource not found, allowed members are: 0,1,2,3");
			}
			auto body=crow::json::load(req.body);
			if(check_rate_limit(ip_table, req.remote_ip_address)==false){
				return crow::response(429, "Too many requests, slow down\n");
			}
			if (!body) return crow::response(400, "Invalid JSON\n");
			if (!body.has("team_name") || !body.has("password")){
				return crow::response(400, "Missing team_name/password\n");
			}
			std::string team_name = body["team_name"].s();
			std::string password = body["password"].s();
            std::string password_hashed=hash_password(password);
			int pass_state=check_password(DB_read_connection, team_name, password_hashed/*, verify_password_statement*/);
			if (pass_state==-1){
				return crow::response(500, "Internal Server Error: Password could not be verified.\n");
			} else if (pass_state==0){
				return crow::response(401, "Wrong credentials OR team does not exist\n");
			}
			//const char* add_detail_statement="UPDATE team_details SET memX_name=NULL, memX_enrol=NULL WHERE team_name=?;"
			int status=delete_teammate(DB_write_connection, team_name, mem_num);
			if (status==-1){
				return crow::response(500, "Password verified but could teammate could not be deleted. Internal Server Error\n");
			}
			return crow::response(200, "Team member deleted\n");
		});
	printf("[Member Delete Route]: UP\n");
	printf("Usage: DELETE curl 127.0.0.1:8000/team/member/<int:0/1/2/3> -d '{\"team_name\":\"xyz\",\"password\":\"sicrit\"}'\n\n");
}
