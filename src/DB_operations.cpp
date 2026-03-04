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

int register_team(sqlite3* DB_connection, const std::string& username, const std::string& password_hashed, const char* statement){ 
	sqlite3_stmt* PpStmt = nullptr;
	if (sqlite3_prepare_v2(
		DB_connection,			/* Database handle */
		statement,				/* SQL statement, UTF-8 encoded */
		-1,						/* Maximum length of zSql in bytes. */
		&PpStmt,	/* OUT: Statement handle */
		nullptr					/* OUT: Pointer to unused portion of zSql */
	)!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Preparation of SQL stmt failed: "<<sqlite3_errmsg(DB_connection)<<std::endl;
		return -1;
	}
	if ((sqlite3_bind_text(PpStmt, 1, username.c_str(), -1, SQLITE_TRANSIENT))!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Could not bind username to prepared string: "<<sqlite3_errmsg(DB_connection)<<std::endl;
		sqlite3_finalize(PpStmt);
		return -1;
	}
	if ((sqlite3_bind_text(PpStmt, 2, password_hashed.c_str(), -1, SQLITE_TRANSIENT))!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Could not bind pass to prepared string: "<<sqlite3_errmsg(DB_connection)<<std::endl;
		sqlite3_finalize(PpStmt);
		return -1;
	}
	if (sqlite3_step(PpStmt)!=SQLITE_DONE){
		std::cerr<<"SQLITE ERROR: Could not run query on DB: "<<sqlite3_errmsg(DB_connection)<<" Username: "<<username<<std::endl;
		sqlite3_finalize(PpStmt);
		return -2;
	}
	sqlite3_finalize(PpStmt);
	std::cout<<"Account created successfully\n";
	return 0;
}

	int setup_db(sqlite3* db_connection){
		const char* setup_users_table_query="CREATE TABLE IF NOT EXISTS auth_table (team_name TEXT NOT NULL UNIQUE, password TEXT NOT NULL);";
		char* zErrMsg=0;
		if(sqlite3_exec(db_connection, setup_users_table_query, nullptr, 0, &zErrMsg)!=SQLITE_OK){
			std::cout<<"Error in setting up auth_table: "<<zErrMsg;
			sqlite3_free(zErrMsg);
			return -1;
		}
		return 0;
	}
