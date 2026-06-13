#include <iostream>
#include <map>
#include <string>

using namespace std;
using namespace config;
using nlohmann::json;

class Sync4ERPService: public V2Service {
  using json = nlohmann::json;
  private:
    Constant constant;
    DocType docType;
    TableFieldMapping tableFieldMapping;
    Database database;

  public: 
    string response;
    TableName tableName;
    
  public: bool getData() {
    bool returnValue = false;
    std::map<std::string, string> requestMap;

    for (auto docType : tableFieldMapping.tableMapping) {
      int inserted = 0;
      string docTypeName = docType.first;
      map<std::string, std::string> resultMap;
    
      std::map<std::string, std::string> fieldMapping = tableFieldMapping.tableFieldMapping[docTypeName];
      std::map<std::string, std::string> tableMap = tableFieldMapping.tableMapping[docTypeName];
      util.printMap(tableMap);
      string tableName = tableMap.begin()->first;
      string tableAPIURL = tableMap.begin()->second;

      vector<string> fieldList{};
      for (auto fieldName : fieldMapping) {
        fieldList.insert(fieldList.end(), fieldName.first);
      }
    
      json jsonArray = fieldList;
      requestMap["fields"] = jsonArray.dump();
      requestMap["limit"] = ERP_PAGE_LENGTH;
      string response = api2ERP(getURL(tableAPIURL, docTypeName), requestMap);
      logInfo(response);
      //if (getErrorMessage() != "") {
        
        json jsonData = json::parse(response);
        if (jsonData["exc_type"] == "DataError") {logMsg("Not able to Sync Doc Type : " + docTypeName);}
        if (jsonData["data"].size() == 0) {jsonData = jsonData["message"];} else {jsonData = jsonData["data"];}
        for (int i = 0; i < jsonData.size(); ++i) {
          for (const auto& item : jsonData[i].items()) {
            if (fieldMapping[item.key()] != "") {resultMap[fieldMapping[item.key()]] = util.json2String(item.value());}
          }
          //Check for insert / update
          string pkey = database.getField("select count(*) from `" + tableName + "` where name = '" + resultMap["name"] + "'");
          if (pkey != "0") {database.whereMap["name"] = resultMap["name"];}
          int result = database.update(tableName, resultMap);
          if (result == 0) {inserted++;}
        }
        logMsg("Inserted / Updated Records for " + tableName +  " : " + to_string(inserted));
      /*}
      else {
        logMsg(getErrorMessage());
      }*/
    
    }
    exit(0);
    return returnValue;
  };

  private: string getURL(string tableAPIURL, string docTypeName) {
    logInfo(tableAPIURL + "::" + docTypeName );
    string url;
    if (tableAPIURL != "") {url = constant.API_CUSTOM_URL + tableAPIURL;} else {url = constant.API_DOC_URL;}
    url += util.encodeURL(docTypeName);
    logInfo(url);
    return url;
  }

  //Fetching the data from ERP
  protected: string api2ERP(string url, std::map<std::string, string> requestKeyPairMap) {
    RestAPI restAPI;
    RestAPIModel restAPIModel;
    map<std::string, string> header;
    header["Authorization"] = config::TOKEN;
    //header["Content-Type"] = "application/json";
    header["Accept"] = "application/json";
    restAPIModel.setApiURL(url);
    restAPIModel.setRequestJSON(false);
    restAPIModel.setHeader(header);
    restAPIModel.setRequestKeyPairMap(requestKeyPairMap);
    string response;
    restAPI.getRequest(restAPIModel, response);
    logInfo(response);
    return response;
  }
};