#include <iostream>
#include <string>
#include <list>
#include <sqlite3.h>
#include <freshweb/dateUtil.h>

#include <map>

using namespace std;
using namespace config;
using json = nlohmann::json;

class Database {
	protected: TableName tableName;
		DateUtil dateUtil;
	
	//Config config;
  sqlite3 *db;
  char *szErrMsg = 0;

  public:
		sqlite3_stmt *stmt;
		std::list<string> selectFieldNameList;
		std::list<string> selectJoinList;
		std::map<std::string, string> whereMap;
		std::map<std::string, string> orderMap;
		string start = "0";
		string length = "35";
		map<int, map<string, string>> returnData;
		json jsonReturnData = json::array();
		json null;

  //Connect to the database
  int getDbConnection() {
    int sqlResult = 0;
    sqlResult = sqlite3_open("db/njhat.db", &db);
    if (sqlResult) {std::cout << "Cannot open database";} //else {std::cout << "Database Connected";}
    return sqlResult;
  }

	public: int executeSQL(string sqlQuery) {
		getDbConnection();
    int sqlResult = 0;
		sqlQuery += buildWhere() + buildOrder() + buildLimit();
		// std::cout <<"\n\n"<<sqlQuery << endl;
  	if (config::DEBUG) {std::cout << "execute SQL: " << sqlQuery << "\n";}
    sqlResult = sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, 0);
    if(sqlResult != SQLITE_OK) {
    	std::cout << "SQL Error: "  << sqlQuery << std::endl;
			exit(2);
    }
		resetQueryParam();
		return sqlResult;
	}

	private: void resetQueryParam() {
		whereMap.clear();
		orderMap.clear();
		selectFieldNameList.clear();
		selectJoinList.clear();
	}

	public: std::map<string, string> getRowData() {
		int columnCount = sqlite3_column_count(stmt);
		map<std::string, string> rowData;
		for (int col=0; col< columnCount; col++) {
			if (sqlite3_column_type(stmt, col) != SQLITE_NULL) {
				const char *columnName = sqlite3_column_name(stmt, col);
				const unsigned char *columnValue = sqlite3_column_text(stmt, col);
				std::string sValue((char*) columnValue);
				std::string sName((char*) columnName);
				if (sqlite3_column_type(stmt, 0) == SQLITE_NULL) {sValue = "";}
				rowData[sName] = sValue;
			}
		}
		return rowData;
	}

	public: int getDataList(string tableName) {
		int sqlResult = 0;
		string sqlQuery = buildSQL (tableName);
		// std:cout << sqlQuery;
		sqlResult = executeSQL(sqlQuery);
		int rowCount = 0;
		jsonReturnData = json::array();
		while (sqlite3_step(stmt) != SQLITE_DONE) {
    	map<std::string, string> rowData = getRowData();
			jsonReturnData.push_back({rowCount, json(rowData)});
    	returnData[rowCount] = rowData;
    	rowCount++;
    }
		sqlite3_finalize(stmt);
    closeDb();
    return sqlResult;
	}

	/*@deprecated
	get data from database*/
	public: int getData(string sqlQuery){
		return getSelect(sqlQuery);
	}

  /*@deprecated
		get data from database*/ 
  int getSelect(string sqlQuery) {
		int sqlResult = 0;
		sqlResult = executeSQL(sqlQuery);
		jsonReturnData = json::array();
    int rowCount = 0;
    while (sqlite3_step(stmt) != SQLITE_DONE) {
    	map<std::string, string> rowData = getRowData();
			jsonReturnData.push_back({rowCount, rowData});
    	returnData[rowCount] = rowData;
    	rowCount++;
    }
    sqlite3_finalize(stmt);
    closeDb();
    return sqlResult;
  }

  string getField(string sqlQuery, string returnField = "fieldData") {
  	string returnValue = "";
		int sqlResult = executeSQL(sqlQuery);
  	map<std::string, string> rowData = returnData[0];
  	std::map<std::string, string>::iterator it = rowData.begin();
  	while (it != rowData.end()) {
  		if (returnField == "fieldData") {returnValue = it->second;} else {returnValue = it->first;}
  		++it;
  	}
  	return returnValue;
  }

	//update / Insert -- all the other update can be decrepicated. 
	public: int update(string tableName, std::map<std::string, string> fieldMap) {
		
		int sqlResult = 0;
		string updateFieldName;
		string fieldName;
		string fieldValue;
		string sqlQuery;

		//find whethere wheremp is defined. Not working
		if (whereMap.size() == 0) {
			//Check whether primary is there in the Map for update
			string pkSQL = "select name from pragma_table_info('" + tableName + "') where pk";
			string pkName = getField(pkSQL, "fieldName");
			// cout<<"\n pk name is"<<pkName<<endl;
			// cout<<"feild name:"<<fieldMap[pkName]<<endl;
			if (fieldMap.find(pkName) != fieldMap.end()) {whereMap[pkName] = fieldMap[pkName];}
		}
		getDbConnection();
		std::map<std::string, string>::iterator it = fieldMap.begin();
		fieldName = " created_on "; fieldValue = "'" + dateUtil.getGMTDateTime() + "'"; updateFieldName = " modified_on = " + fieldValue;
		while (it != fieldMap.end()) {
			if (fieldName != "") {fieldName += ",";fieldValue += ",";}
			if (updateFieldName != "") {updateFieldName += ",";}
			fieldName += it->first;
			fieldValue += "'" + it->second + "'";
			updateFieldName += it->first + " = '" + it->second + "'";
			++it;
		}
		
		if (whereMap.size() != 0) {
			fieldMap["modified_on"] = dateUtil.getGMTDateTime();
			sqlQuery = "update " + tableName + " set " + updateFieldName + buildWhere();
			// cout<<"\nupdate query is:"<<sqlQuery<<endl;
			if (config::DEBUG) {cout << "update Query - " << tableName << ": "<< sqlQuery << "\n";}
		}
		else {
			fieldMap["created_on"] = dateUtil.getGMTDateTime();
			sqlQuery = "insert into " + tableName + "(" + fieldName + ") values (" + fieldValue + ")";
			// std::cout << sqlQuery;
			if (config::DEBUG) {cout << "Insert SQL Query - " <<  tableName <<": " << sqlQuery;}
		}
		sqlResult = sqlite3_exec(db, sqlQuery.c_str(), 0, 0, &szErrMsg);
  	if(sqlResult != SQLITE_OK) {
  		std::cout << "SQL Error: " << szErrMsg << " -- " << sqlQuery << std::endl << "\n";
  		sqlite3_free(szErrMsg);
  	}
		sqlResult = sqlite3_last_insert_rowid(db);
		closeDb();
		whereMap.clear();
		return sqlResult;
	}

	//@Depreciated
	//Update with prepared statement, need to do insert also.
	public: int update(string tableName, std::map<std::string, string> fieldMap, bool update) {
		getDbConnection();
		int sqlResult = 0;
		string fieldName;
		fieldMap["modified_on"] = dateUtil.getGMTDateTime();
		std::map<std::string, string>::iterator it = fieldMap.begin();
  	while (it != fieldMap.end()) {
  		if (fieldName != "") {fieldName += ",";}
  		fieldName += it->first + " = '" + it->second + "'";
  		++it;
  	}
		string sqlQuery = "update " + tableName + " set " + fieldName + buildWhere();
		if (config::DEBUG) {cout << "update Query - " << tableName << ": "<< sqlQuery << "\n";}
		sqlResult = sqlite3_exec(db, sqlQuery.c_str(), 0, 0, &szErrMsg);
  	if(sqlResult != SQLITE_OK) {
  		std::cout << "SQL Error: " << szErrMsg << std::endl;
  		sqlite3_free(szErrMsg);
  	}
		closeDb();
		return sqlResult;
	}

	//Works only with insert need to be removed for prepared statement
  /*public: int update(string tableName, std::map<std::string, string> fieldData) {
  	getDbConnection();
  	int sqlResult = 0;
  	string fieldName;
  	string fieldValue;
		fieldData["created_on"] = dateUtil.getGMTDateTime();

  	std::map<std::string, string>::iterator it = fieldData.begin();
  	while (it != fieldData.end()) {
  		if (fieldName != "") {fieldName += ",";fieldValue += ",";}
  		fieldName += it->first;
  		fieldValue += "'" + it->second + "'";
  		++it;
  	}
		
  	string sqlQuery = "insert into " + tableName + "(" + fieldName + ") values (" + fieldValue + ")";
		if (config::DEBUG) {cout << "Insert SQL Query - " <<  tableName <<": " << sqlQuery;}
  	sqlResult = sqlite3_exec(db, sqlQuery.c_str(), 0, 0, &szErrMsg);
  	if(sqlResult != SQLITE_OK) {
  		std::cout << "SQL Error: " << szErrMsg << std::endl;
  		sqlite3_free(szErrMsg);
  	}
  	sqlResult = sqlite3_last_insert_rowid(db);
  	closeDb();
  	return sqlResult;
  }*/

  //Close DB Connection
  void closeDb() {
    if(db) {sqlite3_close(db);}
  }

	private: string buildSQL (string tableName) {
    string sqlQuery = "select ";
    for(string fieldNameByRow : selectFieldNameList) {
      if (sqlQuery != "select ") {sqlQuery += ", ";}
      sqlQuery += fieldNameByRow;
    }
    sqlQuery += " from " + tableName;
		for(string joinByRow : selectJoinList) {sqlQuery = sqlQuery + " " + joinByRow + " ";}
    return sqlQuery;
  }

  private:
  string buildWhere() {
		string whereData = "";
  	std::map<std::string, string>::iterator it = whereMap.begin();
  	while (it != whereMap.end()) {
  		if (whereData != "") {whereData += " and ";}
			if (it->first == "#") {whereData += it->second;} else {whereData += it->first + " = '" + it->second + "'";}
  		++it;
  	}
  	if (whereData != "") {whereData = " where " + whereData;}
		return whereData;
  }

	private:
  string buildOrder() {
		string orderBy = "";
  	std::map<std::string, string>::iterator it = orderMap.begin();
  	while (it != orderMap.end()) {
  		if (orderBy!= "") {orderBy += " , ";}
  		orderBy += it->first + " " + it->second;
  		++it;
  	}
  	if (orderBy != "") {orderBy = " order by " + orderBy;}
		return orderBy;
  }

	private:
	string buildLimit(){
		string returnValue = " limit ";
		if (start != "") {returnValue = returnValue + start;}
		if (length != "") {returnValue = returnValue + ", " + length;}
		if (returnValue == " limit " ) {returnValue = "";}
		if (length == "-1") {returnValue = "";}
		return returnValue;
	}

	/*private: string changeCamelCase2TableCase(string fieldName) {
    string dbFileName = "";
    for (int i=0; i<strlen(fieldName.c_str); i++) {
        if (isupper(fieldName[i])) {dbFileName = "_" fieldName[i] + 32;} else {dbFileName = fieldName[i];} 
    }
    return dbFileName;
  }*/

  static int callback(void *NotUsed, int argc, char **argv, char **szColName) {
  	map<string, string> rowData;
  	for(int i = 0; i < argc; i++) {
  		rowData[szColName[i]] = argv[i];
      std::cout << szColName[i] << " = " << argv[i] << std::endl;
    }
  	std::cout << "\n";
  	return 0;
  }
  public: int deleteData(const string& deleteQuery) {
    int sqlResult = 0;

    // Open the database connection
    sqlResult = getDbConnection();
    if (sqlResult != SQLITE_OK) {
        std::cerr << "Failed to open database connection." << std::endl;
        return sqlResult;
    }

    // Execute the DELETE query
    sqlResult = sqlite3_exec(db, deleteQuery.c_str(), 0, 0, &szErrMsg);
    if (sqlResult != SQLITE_OK) {
        std::cerr << "SQL Error: " << szErrMsg << " -- " << deleteQuery << std::endl;
        sqlite3_free(szErrMsg); // Free the error message
    } // } else {
    //     std::cout << "Delete operation successful." << std::endl;
    // }

    // Close the database connection
    closeDb();
    return sqlResult;
}
};
