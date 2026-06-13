#include <iostream>
#include <map>
#include <string>
//#include <freshweb/restAPI.h>

using namespace std;
using namespace config;
using nlohmann::json;

class Sync2ERPService: public V2Service  {
  using json = nlohmann::json;
	Database database;
  Constant constant;
  RestAPI restAPI;
  RestAPIModel restAPIModel;
  RestAPIModel restAPIModel1;
  DocType docType;
  
  public: 
    string postFields;
    string response;
    TableName tableName;

  //public: Sync2ERP() {}
    
  public: bool rawData() {
    logMsg("Started to push the configuration information to ERP...");
    bool returnValue = false;
    map<int, string> primaryKeyMap;
    int count=0;

    //Sync Raw Data
    map<string, string> whereData;
    map<string, string> orderByData;
    whereData["is_processed"] = "No";
    orderByData["raw_data_id"] = "asc";
    string sqlQuery =  "select raw_data_id, json_data from nj_raw_data ";
    database.whereMap = whereData;
	
    int sqlResult = database.executeSQL(sqlQuery);
    //logInfo("sqlquery: " + sqlQuery);
    int i = 0;
    while (sqlite3_step(database.stmt) != SQLITE_DONE) {
      map<std::string, string> rowData = database.getRowData();
      json jsonObject = rowData;
      string response = api2ERP(constant.API_DOC_URL + docType.RAW_DATA, to_string(jsonObject));
      //logInfo("Response: " + response);
      if (response != "") {
        returnValue = "true";
        primaryKeyMap[count] = rowData["raw_data_id"];
        count++;
      }
       else {logError("Not able to communicate to ERP server");}
      i++;
      logMsg("Updated " + to_string(i) + " information... ");
    }
    if (returnValue) {
      updateAPICompleted(tableName.RAW_DATA, "raw_data_id", primaryKeyMap);
    }
    else {logError("Not able to get any data for processing to ERP");}
    logMsg("Update to ERP is successfull...");
    return returnValue;
  };

  //update the status after completing the syntax
  protected: int updateAPICompleted(string tableName, string primaryKeyName, map<int, string> primaryKeyMap) {
    Database updateDatabase;
     string updateWhereData;
    std::map<std::string, string> fieldMap;
    for (auto i = primaryKeyMap.begin(); i != primaryKeyMap.end(); i++) {
      if (updateWhereData != "") {updateWhereData += " or ";}
      updateWhereData += primaryKeyName + "= '" + i -> second + "'";
    }
    if (updateWhereData != "") {updateWhereData = " (" + updateWhereData + ") ";}
    logInfo("update Where: " + updateWhereData);

    fieldMap["is_processed"] = "Yes";
    updateDatabase.whereMap["#"] = updateWhereData;
    return updateDatabase.update(tableName, fieldMap, true);
  }

  //Creating the data in ERP
  protected: string api2ERP(string url, json jsonData) {
    map<std::string, string> header;
    header["Authorization"] = config::TOKEN;
    header["Content-Type"] = "application/json";
    header["Accept"] = "application/json";
    restAPIModel1.setApiURL(url);
    restAPIModel1.setPostFields(jsonData);
    restAPIModel1.setRequestJSON(true);
    restAPIModel1.setHeader(header);
    restAPI.postRequest(restAPIModel1, response);
    return response;
  }
};