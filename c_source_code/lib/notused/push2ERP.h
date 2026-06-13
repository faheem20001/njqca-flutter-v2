#include <iostream>
#include <map>
#include <string>
#include <freshweb/restAPI.h>

using namespace std;
using nlohmann::json;

class Push2ERP {
  using json = nlohmann::json;
	Database database;
  Constant constant;
  Config config;
  RestAPI restAPI;
  RestAPIModel restAPIModel;
  RestAPIModel restAPIModel1;
  
  public: 
    string postFields;
    string response;

  public: void push2ERP() {}
    
  public: bool rawData() {
    bool returnValue = true;
    
    string updateWhere;

    //Sync Raw Data
    map<string, string> whereData;
    map<string, string> orderByData;
    whereData["is_processed"] = "No";
    orderByData["raw_data_id"] = "asc";
    string sqlQuery =  "select raw_data_id, json_data from nj_raw_data ";
    database.whereMap = whereData;
	
    int sqlResult = database.executeSQL(sqlQuery);
    int i = 0;
    while (sqlite3_step(database.stmt) != SQLITE_DONE) {
      map<std::string, string> rowData = database.getRowData();
      json jsonObject = rowData;
      string response = api2ERP(to_string(jsonObject));
      if (response != "") {
        //update the row as updated in ERP
        if (updateWhere != "") {updateWhere += " or ";}
        updateWhere += " raw_data_id = '" + rowData["raw_data_id"] + "'";
        
      } else {returnValue = "false";}
      i++;
    }
    updateWhere = " (" + updateWhere + ") ";
    //update all the completed rows - Assuming that interuption has not happened
    updateAPICompleted(updateWhere);
    return returnValue;
  };

  //update the status after completing the syntax
  private: int updateAPICompleted(string whereData) {
    Database updateDatabase;
    std::map<std::string, string> fieldMap;
    fieldMap["is_processed"] = "Yes";
    updateDatabase.whereMap["#"] = whereData;
    return updateDatabase.update("nj_raw_data", fieldMap, true);
  }

  //Pusing to ERP - RawData
  private: string api2ERP(json jsonData) {
    map<std::string, string> header;
    header["Authorization"] = config.TOKEN;
    header["Content-Type"] = "application/json";
    header["Accept"] = "application/json";
    restAPIModel1.setApiURL(constant.API_CREATE_URL + "Raw%20Data");
    restAPIModel1.setPostFields(jsonData);
    restAPIModel1.setRequestJSON(true);
    restAPIModel1.setHeader(header);
    restAPI.postRequest(restAPIModel1, response);
    return response;
  }
};