#include "crow.h"
//This file handles main server start/stop

void setup_register_route(crow::SimpleApp& app);

int main(){
    crow::SimpleApp app;
    setup_register_route(app);
    app.port(8000).multithreaded().run();
}
