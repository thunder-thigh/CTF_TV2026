#include <stdio.h>
#include <sqlite3.h>
#include <iostream>

#define SQLITE_OPEN_READONLY         0x00000001  /* Ok for sqlite3_open_v2() */

using namespace std;

sqlite3* loadDB_write(){
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

sqlite3* loadDB_read(){
	sqlite3* db;
	char dbname[] = "users.db";
	int rc=sqlite3_open_v2(dbname, &db, SQLITE_OPEN_READONLY, nullptr);
	if(rc){
		fprintf(stderr, "Can't Open Database: %s\n", sqlite3_errmsg(db));
		return NULL;
	} else {
		return db;
	}
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	for(i=0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

int setup_db(sqlite3* DB_connection){
	const char* setup_users_table_query="CREATE TABLE IF NOT EXISTS auth_table (team_name TEXT NOT NULL UNIQUE, password TEXT NOT NULL);";
	char* zErrMsg=0;
	if(sqlite3_exec(DB_connection, setup_users_table_query, nullptr, 0, &zErrMsg)!=SQLITE_OK){
		std::cout<<"Error in setting up auth_table: "<<zErrMsg;
		sqlite3_free(zErrMsg);
		return -1;
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

int register_team(sqlite3* DB_write_connection, std::string& team_name, std::string& password_hashed, const char* statement){ 
    //const char* statement="INSERT INTO auth_table (team_name, password) VALUES (?,?);";
	sqlite3_stmt* PpStmt = nullptr;
	if (sqlite3_prepare_v2(
		DB_write_connection,			/* Database handle */
		statement,				/* SQL statement, UTF-8 encoded */
		-1,						/* Maximum length of zSql in bytes. */
		&PpStmt,	/* OUT: Statement handle */
		nullptr					/* OUT: Pointer to unused portion of zSql */
	)!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Preparation of SQL stmt failed: "<<sqlite3_errmsg(DB_write_connection)<<std::endl;
		return -1;
	}
	if ((sqlite3_bind_text(PpStmt, 1, team_name.c_str(), -1, SQLITE_TRANSIENT))!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Could not bind username to prepared string: "<<sqlite3_errmsg(DB_write_connection)<<std::endl;
		sqlite3_finalize(PpStmt);
		return -1;
	}
	if ((sqlite3_bind_text(PpStmt, 2, password_hashed.c_str(), -1, SQLITE_TRANSIENT))!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Could not bind pass to prepared string: "<<sqlite3_errmsg(DB_write_connection)<<std::endl;
		sqlite3_finalize(PpStmt);
		return -1;
	}
	if (sqlite3_step(PpStmt)!=SQLITE_DONE){
		std::cerr<<"SQLITE ERROR: Could not run query on DB: "<<sqlite3_errmsg(DB_write_connection)<<" Team name: "<<team_name<<std::endl;
		sqlite3_finalize(PpStmt);
		return -2;
	}
	sqlite3_finalize(PpStmt);
	std::cout<<"Account created successfully\n";
	return 0;
}

std::string fetch_single_string_from_DB(sqlite3* DB_read_connection, std::string column, const char* statement){ 
	//const char* statement = "SELECT ? FROM ? WHERE ? = ?;";
	sqlite3_stmt* PpStmt = nullptr;
	if (sqlite3_prepare_v2(
		DB_read_connection,			/* Database handle */
		statement,				/* SQL statement, UTF-8 encoded */
		-1,						/* Maximum length of zSql in bytes. */
		&PpStmt,				/* OUT: Statement handle */
		nullptr					/* OUT: Pointer to unused portion of zSql */
	)!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Preparation of SQL stmt failed: "<<sqlite3_errmsg(DB_read_connection)<<std::endl;
		//return -1;
	}
	if ((sqlite3_bind_text(PpStmt, 4, column.c_str(), -1, SQLITE_TRANSIENT))!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Could not bind constraint value to prepared string: "<<sqlite3_errmsg(DB_read_connection)<<std::endl;
		sqlite3_finalize(PpStmt);
		//return -1;
	}
	int rc=sqlite3_step(PpStmt);
	if (rc==SQLITE_ROW){
		const unsigned char* fetched_string=sqlite3_column_text(PpStmt,0);
		if (fetched_string){
			std::string result(reinterpret_cast<const char*>(fetched_string));
			std::cout<<"Fetched string from DB: "<<result<<endl;
			return result;
		} else {
			std::cout<<"Retrieved NULL value"<<endl;
		}
	} else if (rc==SQLITE_DONE){
		std::cout<<"Query returned no rows"<<endl;
	} else {
		std::cerr<<"SQL step error"<<sqlite3_errmsg(DB_read_connection)<<endl;
	}
	sqlite3_finalize(PpStmt);
	return column;
}

int check_password(sqlite3* DB_read_connection, std::string team_name, std::string password_hashed, const char* statement){
	//const char* statement="SELECT COUNT() FROM auth_table WHERE team_name=? AND password=?;";
	sqlite3_stmt* PpStmt = nullptr;
	if (sqlite3_prepare_v2(
		DB_read_connection,			/* Database handle */
		statement,				/* SQL statement, UTF-8 encoded */
		-1,						/* Maximum length of zSql in bytes. */
		&PpStmt,	/* OUT: Statement handle */
		nullptr					/* OUT: Pointer to unused portion of zSql */
	)!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Preparation of SQL stmt failed: "<<sqlite3_errmsg(DB_read_connection)<<std::endl;
		return -1;
	}
	if ((sqlite3_bind_text(PpStmt, 1, team_name.c_str(), -1, SQLITE_TRANSIENT))!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Could not bind username to prepared string: "<<sqlite3_errmsg(DB_read_connection)<<std::endl;
		sqlite3_finalize(PpStmt);
		return -1;
	}
	if ((sqlite3_bind_text(PpStmt, 2, password_hashed.c_str(), -1, SQLITE_TRANSIENT))!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Could not bind pass to prepared string: "<<sqlite3_errmsg(DB_read_connection)<<std::endl;
		sqlite3_finalize(PpStmt);
		return -1;
	}
	int rc=sqlite3_step(PpStmt);
	if (rc!=SQLITE_ROW){
		std::cerr<<"SQL step error"<<sqlite3_errmsg(DB_read_connection)<<endl;
	} else {
		const unsigned char* fetched_result=sqlite3_column_text(PpStmt,0);
		std::cout<<fetched_result<<endl;
		if (*fetched_result=='1'){
			sqlite3_finalize(PpStmt);
			return 1;
		} else {
			sqlite3_finalize(PpStmt);
			return 0;
		}
	}
	sqlite3_finalize(PpStmt);
	return -1;
}

int update_password(sqlite3* DB_write_connection, std::string team_name, std::string new_pass_hashed, const char* statement){
    //const char* reset_password_statement="UPDATE auth_table SET password=? WHERE team_name=?;";
	sqlite3_stmt* PpStmt = nullptr;
	if (sqlite3_prepare_v2(
		DB_write_connection,			/* Database handle */
		statement,				/* SQL statement, UTF-8 encoded */
		-1,						/* Maximum length of zSql in bytes. */
		&PpStmt,	/* OUT: Statement handle */
		nullptr					/* OUT: Pointer to unused portion of zSql */
	)!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Preparation of SQL stmt failed: "<<sqlite3_errmsg(DB_write_connection)<<std::endl;
		return -1;
	}
	if ((sqlite3_bind_text(PpStmt, 1, new_pass_hashed.c_str(), -1, SQLITE_TRANSIENT))!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Could not bind pass to prepared string: "<<sqlite3_errmsg(DB_write_connection)<<std::endl;
		sqlite3_finalize(PpStmt);
		return -1;
	}
	if ((sqlite3_bind_text(PpStmt, 2, team_name.c_str(), -1, SQLITE_TRANSIENT))!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Could not bind team_name to prepared string: "<<sqlite3_errmsg(DB_write_connection)<<std::endl;
		sqlite3_finalize(PpStmt);
		return -1;
	}
	if (sqlite3_step(PpStmt)!=SQLITE_DONE){
		std::cerr<<"SQLITE ERROR: Could not run query on DB: "<<sqlite3_errmsg(DB_write_connection)<<" Team name: "<<team_name<<std::endl;
		sqlite3_finalize(PpStmt);
		return -1;
	}
	sqlite3_finalize(PpStmt);
	return 1;
}

int delete_team(sqlite3* DB_write_connection, std::string team_name, const char* statement){
    //const char* reset_password_statement="DELETE FROM auth_table WHERE team_name="001";";
	sqlite3_stmt* PpStmt = nullptr;
	if (sqlite3_prepare_v2(
		DB_write_connection,			/* Database handle */
		statement,				/* SQL statement, UTF-8 encoded */
		-1,						/* Maximum length of zSql in bytes. */
		&PpStmt,	/* OUT: Statement handle */
		nullptr					/* OUT: Pointer to unused portion of zSql */
	)!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Preparation of SQL stmt failed: "<<sqlite3_errmsg(DB_write_connection)<<std::endl;
		return -1;
	}
	if ((sqlite3_bind_text(PpStmt, 1, team_name.c_str(), -1, SQLITE_TRANSIENT))!=SQLITE_OK){
		std::cerr<<"SQLITE ERROR: Could not bind pass to prepared string: "<<sqlite3_errmsg(DB_write_connection)<<std::endl;
		sqlite3_finalize(PpStmt);
		return -1;
	}
	if (sqlite3_step(PpStmt)!=SQLITE_DONE){
		std::cerr<<"SQLITE ERROR: Could not run query on DB: "<<sqlite3_errmsg(DB_write_connection)<<" Team name: "<<team_name<<std::endl;
		sqlite3_finalize(PpStmt);
		return -1;
	}
	sqlite3_finalize(PpStmt);
	return 1;
}
