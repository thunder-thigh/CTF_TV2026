#include <stdio.h>
#include <sqlite3.h>
#include <iostream>

using namespace std;

sqlite3* loadDB(){
	sqlite3* db;
	char dbname[] = "users.db";
	int rc=sqlite3_open(dbname, &db);
	if(rc){
		fprintf(stderr, "Can't Open Database: %s\n", sqlite3_errmsg(db));
		return NULL;
	} else {
		return db;
	}
}

int callback(void *data, int argc, char** argv, char**azColName){
	for(int i=0; i<argc; i++){
		printf("%s : %s\n", azColName[i], argv[i]?argv[i]:"NULL");
	}
	return 0;
}

void run_and_print_DB_query(sqlite3* DB_connection, char* query){
	char* messageError = nullptr;
	const char* data;
//	int exit_code=sqlite3_exec(DB_connection, query, callback, (void*)data, &messageError);
	int exit_code=sqlite3_exec(DB_connection, query, callback, nullptr, &messageError);
	if (exit_code!=SQLITE_OK){
		cerr<<"Error in running query"<<messageError<<endl;
		sqlite3_free(messageError);
	} else {
		printf("Query successful\n");
	}
}

void register_team(sqlite3* DB_connection, const std::string username, const std::string email, const std::string password){
	
