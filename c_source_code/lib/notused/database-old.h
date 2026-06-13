#include <iostream>
#include <string>
#include <sqlite3.h>

using namespace std;

class Database {
  sqlite3 *db;
  char *szErrMsg = 0;
  public:
  //Connect to the database
  int getDbConnection() {
    int sqlResult = 0;
    sqlResult = sqlite3_open("src/db/njhat.db", &db);
    if (sqlResult) {
        std::cout << "Cannot open database";
    }
    else {
        std::cout << "Database Connected";
    }
    return sqlResult;
  }

  //get data from database
  int getSelect() {
    int result = getDbConnection();
    std::cout << "DB Result from class" + result;

    int sqlResult = 0;
    const char *sqlQuery = "select * from tbltemp";
    sqlResult = sqlite3_exec(db,sqlQuery, 0, 0, &szErrMsg);
    std::cout << sqlResult;
    if(sqlResult != SQLITE_OK)
    {
      std::cout << "SQL Error: " << szErrMsg << std::endl;
      sqlite3_free(szErrMsg);
    }
    closeDb();
    return sqlResult;
  }

  //Close DB Connection
  void closeDb() {
    if(db) {sqlite3_close(db);}
  }
};
