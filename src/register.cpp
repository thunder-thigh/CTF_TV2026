#include "crow.h"
#include <string>
//This file handles registrations
void setup_register_route(crow::SimpleApp& app){
    CROW_ROUTE(app, "/register")
        .methods("POST"_method)
        ([](const crow::request& req){
            auto body=crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid JSON");
            if (!body.has("username") || !body.has("password")){
                return crow::response(400, "Missing usernsme or password");
            }
            std::string username=body["username"].s();
            std::string password=body["password"].s();
            //std::cout<<username<<password<<"\n";
            //Hash pass
            //Store in DB
            return crow::response(200, "Account created successfully\n");
        });
    printf("Registration Route UP :\n");
}
