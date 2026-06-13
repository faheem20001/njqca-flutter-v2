#include <iostream>
#include <map>
#include <string>
#include <../service/PartResultService.h>
// #include <../service/PartTestService.h>
//#include <../service/Sync2ERPService.h>

#include <fstream>
 #include <cstdio>

using namespace std;
using namespace config;


class IQCService: public V2Service { // public Sync2ERPService 
  using json = nlohmann::json;
	Database database;
  Constant constant;
  RestAPI restAPI;
  RestAPIModel restAPIModel;
  RestAPIModel restAPIModel1;
  DocType docType;
  PartResultService partresultservice;
  static string draft_name;
  
  
  public: 
    string postFields;
    string response;
    TableName tableName;
    static string config_updated_name;
    static int current_stock_info_id;
    static string model_name;
    static string bios_serial_no;
    static float njqca_version;
    static string item_group;
    static bool isNJQCAStored;
    static string loggedUser;
    static string selected_inspection_type;
  const string CMD_NJPULSE="powershell -Command \"Start-Process -FilePath \"chrome.exe\" -ArgumentList \"https://njpulse.newjaisa.com/\"\"";
  // const string CMD_NJPULSE="powershell -Command \"Start-Process -FilePath \"chrome.exe\" -ArgumentList \"http://3.110.131.250/\"\"";
  // const string CMD_ERP="powershell -Command \"Start-Process -FilePath \"chrome.exe\" -ArgumentList \""+constant.apiBaseURL+config_updated_name+"\"";
  
    
  public: 
    string readNJQCANameFromFile(const std::string& currentUser = "") {
        std::ifstream file("njqca_name.txt");
        std::string line;

        if (file.is_open()) {
            getline(file, line);
            file.close();
        }

        if (line.empty()) return "";

        size_t delimPos = line.find('|');
        if (delimPos == std::string::npos) {
            cout << "[INFO] Old format njqca_name.txt found. Treating as fresh." << endl;
            deleteNJQCAFile();
            return "";
        }

        std::string savedNJQCA = line.substr(0, delimPos);
        std::string savedUser  = line.substr(delimPos + 1);

        if (!currentUser.empty() && savedUser != currentUser) {
            cout << "[INFO] NJQCA was created by '" << savedUser << "' but current user is '" << currentUser << "'. Treating as fresh." << endl;
            deleteNJQCAFile();
            return "";
        }

        return savedNJQCA;
    }

    void saveNJQCANameToFile(const std::string& njqcaName, const std::string& userName = "") {
        std::ofstream file("njqca_name.txt");
        if (file.is_open()) {
          // cout << "[DEBUG] Saving NJQCA: " << njqcaName << " | User: " << userName << " | loggedUser: " << loggedUser << endl;
            file << njqcaName << "|" << userName;
            file.close();
            std::cout << "NJQCA name saved to file\n";
        } else {
            std::cout << "NJQCA name saved to file: [" << njqcaName << "] user: [" << userName << "]\n";
        }
    }
    void deleteNJQCAFile() {
        if (std::remove("njqca_name.txt") == 0) {
            std::cout << "njqca_name.txt deleted successfully\n";
        } else {
            std::cout << "njqca_name.txt not found or unable to delete\n";
        }
    }
    string readInspectionTypeFromFile(const std::string& currentUser = "") {
    std::ifstream file("selected_inspection_type.txt");
    std::string line;

    if (file.is_open()) {
        getline(file, line);
        file.close();
    }

    if (line.empty()) return "";

    size_t delimPos = line.find('|');
    if (delimPos == std::string::npos) {
        std::cout << "[INFO] Invalid inspection type file format. Treating as fresh." << std::endl;
        deleteInspectionTypeFile();
        return "";
    }

    std::string savedUser = line.substr(0, delimPos);
    std::string savedInspectionType = line.substr(delimPos + 1);

    if (!currentUser.empty() && savedUser != currentUser) {
        std::cout << "[INFO] Inspection type was saved for user '" << savedUser
                  << "' but current user is '" << currentUser
                  << "'. Treating as fresh." << std::endl;
        deleteInspectionTypeFile();
        return "";
    }

    return savedInspectionType;
}

void saveInspectionTypeToFile(const std::string& inspectionType, const std::string& userName = "") {
    std::ofstream file("selected_inspection_type.txt");
    if (file.is_open()) {
        file << userName << "|" << inspectionType;
        file.close();
        std::cout << "Inspection type saved to file\n";
    } else {
        std::cout << "Unable to save inspection type file\n";
    }
}

void deleteInspectionTypeFile() {
    if (std::remove("selected_inspection_type.txt") == 0) {
        std::cout << "selected_inspection_type.txt deleted successfully\n";
      } else {
          std::cout << "selected_inspection_type.txt not found or unable to delete\n";
      }
  }
  bool canReuseSavedInspectionType(const std::string& currentUser = "") {
    std::string existingNJQCA = readNJQCANameFromFile(currentUser);

    if (existingNJQCA.empty()) {
        deleteInspectionTypeFile();
        return false;
    }

    if (isNJQCASubmitted(existingNJQCA)) {
        std::cout << "[INFO] Saved NJQCA is already submitted. Will not reuse inspection type." << std::endl;
        deleteNJQCAFile();
        deleteInspectionTypeFile();
        return false;
    }

    return true;
}
    bool isNJQCASubmitted(const string& njqcaName) {
    if (njqcaName.empty()) return false;
    
    RestAPI checkAPI;
    RestAPIModel checkModel;
    string checkResponse = "";
    
    map<std::string, string> header;
    header["Authorization"] = config::TOKEN;
    header["Accept"] = "application/json";
    header["charset"] = "utf-8";
    
    // encode name to handle spaces like "HP Probook 430 G6"
    string encodedName = util.encodeURL(njqcaName);
    
    map<string, string> checkRequestMap;
    checkRequestMap["filters"] = "[[\"name\",\"=\",\"" + encodedName + "\"],[\"docstatus\",\"=\",\"1\"]]";
    
    checkModel.setApiURL(constant.API_DOC_URL + docType.IQC);
    checkModel.setRequestJSON(true);
    checkModel.setHeader(header);
    checkModel.setRequestMethod("GET");
    checkModel.setPostFields("");
    checkModel.setRequestKeyPairMap(checkRequestMap);
    checkAPI.getRequest(checkModel, checkResponse);
    
    cout << "[SUBMIT CHECK] checkResponse: " << checkResponse << endl;
    
    if (checkResponse.empty()) return false;
    
    try {
        json j = json::parse(checkResponse);
        for (const auto& item : j.items()) {
            for (const auto& rec : item.value().items()) {
                cout << "[CHECK] NJQCA '" << njqcaName << "' is already SUBMITTED." << endl;
                return true;
            }
        }
    } catch (...) {}
    
    return false;
}

  map<int, map<string, string>> dataSync() {
    logMsg("Started to update iQC information to ERP...");
    cout << "[DEBUG] dataSync loggedUser = '" << loggedUser << "'" << endl;
    bool returnValue = false;
    StockInfoService stockInfoService;
    PartConfigService partConfigService;

    map<int, string> primaryKeyMap;
    map <int, map<string, string>> msgERP;
    int count = 0;
    int slNo = 1;
    
    partConfigService.length = "";
    map<int, std::map<std::string, std::string>> PARTFIELDMAPlIST = partConfigService.getPartFieldMappingList();
    logMsg("Started reading local stock information...")
    //Sync Raw Data
    map<string, string> whereData;
    map<string, string> orderByData;
    stockInfoService.whereMap["is_processed"] = "No";
    stockInfoService.orderMap["stock_info_id"] = "asc";
    //stockInfoService.length = "1";
    std::map<int, std::map<std::string, std::string>> stockInfoList = stockInfoService.getStockInfoList(true);
    for (auto stockInfo : stockInfoList) {
      json jsonObject = stockInfo.second;
      model_name=jsonObject["model"];
      bios_serial_no=jsonObject["bios_serialno"];
      jsonObject["manufacturer_model"] = jsonObject["model"];
      if(njqca_version!=0){
        jsonObject["njqca_version"] = to_string(njqca_version);
      }
      jsonObject["model"] = "";
      if(selected_inspection_type != ""){
          jsonObject["inspection_type"] = selected_inspection_type;
      }
      if(item_group!="Laptops"){
        jsonObject["item_group"] = item_group;
      }
      // for (auto stockInfoData : stockInfo.second) {
        //partConfigService.whereMap["stock_part_id"] = "6";
        //partConfigService.length = "2";
        std::map<int, std::map<std::string, std::string>> partConfigList = partConfigService.getPartConfigList(stockInfo.second["stock_info_id"]);
        json jsonPartArray = json::array();
        for (auto partConfigInfo: partConfigList) {
          std::map<std::string, std::string> partMapping;
          try{
          for (auto partConfigData : partConfigInfo.second) {
            string partMapKey = PARTFIELDMAPlIST[stoi(partConfigInfo.second["stock_part_id"])][partConfigData.first];
            // cout<<partConfigData.first<<endl;
            if (partMapKey == "") {partMapKey = partConfigData.first;}
            partMapping[partMapKey] =sanitize_utf8(partConfigData.second);
          } 
          }catch(const exception & e){
            cout<<"ERROR While maping the results"<<e.what()<<endl;
          }
          json jsonPartObject = partMapping; //partConfigInfo.second;
          jsonPartArray.insert(jsonPartArray.end(), jsonPartObject);
           
        }
        jsonObject["njqca_item_component"] = jsonPartArray;
        jsonObject["power_on"] = "Yes";
        //jsonObject["created"] = "it@mail.com";
        // parttestservice.joinbatteryThread();
        std::map<int, std::map<std::string, std::string>> partResultList = partresultservice.getPartResultList(stockInfo.second["stock_info_id"]);
        json jsonResultArray = json::array();
        for(const auto&[key, result]: partResultList){
            std::map<std::string, std::string> ResultMapping;
            string partConfigId = result.at("part_config_id");
            for (size_t i = 0; i < jsonPartArray.size(); ++i) {
            if (jsonPartArray[i]["part_config_id"] == partConfigId) {
              ResultMapping=result;
              // ResultMapping["config"]=to_string(i+1);
            }
        }
        if(!ResultMapping.empty()){
         json jsonResultObject=ResultMapping;
        jsonResultArray.insert(jsonResultArray.end(),jsonResultObject);}
        }
        jsonObject["njqca_result_parameters"]=jsonResultArray;
    // }
       
      std::cout << jsonObject.dump(2) << std::endl;

      string method = "post";
      string recordPkey = "";
      //Check for draft data and get the same
      map<string, string> requestMap;
      //Check whether the IQC doctype has draft data
      requestMap["filters"] = "[[\"serial_no\",\"=\",\"" + stockInfoList[count]["bios_serialno"] + "\"],[\"docstatus\",\"=\",\"0\"]]";
      //requestMap["limit_page_length"] = "1";
      std::cout << constant.API_DOC_URL << docType.IQC<<endl;
      
      // string response = api2ERP(constant.API_DOC_URL + docType.IQC, "get", requestMap, "");
      // logInfo(response);
      // if (response != "") {
      //   method = "PUT";
      //   recordPkey = "/" + util.encodeURL(response);
      // }


      // read NJQCA from file — only reuse if same user created it
        string existingNJQCA = readNJQCANameFromFile(loggedUser);

        // check if saved NJQCA is already submitted in ERP → if yes delete file and start fresh
        if (!existingNJQCA.empty() && isNJQCASubmitted(existingNJQCA)) {
            cout << "[INFO] NJQCA '" << existingNJQCA << "' already submitted. Creating fresh." << endl;
            deleteNJQCAFile();
            existingNJQCA = "";
            isNJQCAStored = false;
        }

        if(isNJQCAStored){
          // decide POST or PUT
          if(existingNJQCA != ""){
              // NJQCA exists, same user, still DRAFT → update same
              config_updated_name = existingNJQCA;
              recordPkey = "/" + util.encodeURL(existingNJQCA);
              method = "PUT";
              cout << "Using existing NJQCA: " << existingNJQCA << endl;
          }
          else{
              // No NJQCA or different user or submitted → create new
              method = "post";
              cout << "Creating new NJQCA..." << endl;
          }
      }

      //update stock and part informatoin to IQC in ERP
      response = api2ERP(constant.API_DOC_URL + docType.IQC + recordPkey, method, requestMap, to_string(jsonObject));
  
      // cout<<"response from ERP 185:------------"<<response<<endl;
      logInfo(response);
      if(response != "" && existingNJQCA == ""){
          config_updated_name = response;
          saveNJQCANameToFile(config_updated_name, loggedUser);
      }

      if(response.empty()){
        cout<<"\nConfig is Not updated in ERP..."<<endl;
        cout<<"\nPlease Check the ERP Connection..."<<endl;
        exit(0);
      }
      config_updated_name=response;
      // draft_name=config_updated_name;
      draft_name=util.encodeURL(config_updated_name);
      string CMD_ERP="powershell -Command \"Start-Process -FilePath \"chrome.exe\" -ArgumentList \""+constant.apiBaseURL+"app/njqca/"+draft_name+"\"";
     
      // CMD_ERP=util.encodeURL(CMD_ERP);
      cout<<"\nConfig Updated Name in ERP:"<<config_updated_name<<endl;
      saveNJQCANameToFile(config_updated_name, loggedUser);
      // util.executeTerminal(CMD_NJPULSE);
      // // cout<<CMD_ERP<<endl;
      // util.executeTerminal(CMD_ERP);
      map <string, string> msgERPInside;
      msgERPInside["No"] = to_string(slNo);
      if(jsonObject.contains("item_group")){msgERPInside["Item Group"] = jsonObject["item_group"];}
      if(jsonObject.contains("brand")){msgERPInside["Brand"] = jsonObject["brand"];}
      if(jsonObject.contains("model")){msgERPInside["Model"] = jsonObject["manufacturer_model"];}
     if(jsonObject.contains("make")){ msgERPInside["Make"] = jsonObject["make"];}
     if(jsonObject.contains("bios_serialno")) {msgERPInside["BIOS Serial No"] = jsonObject["bios_serialno"];}
      if(jsonObject.contains("created")) {msgERPInside["Created By"] = jsonObject["created"];}
      msgERPInside["Status"] = "Failed";

      if (response != "") {
        returnValue = "true";
        StockInfoModel stockInfoModel;
        stockInfoModel.setErpIqcName(response);
        stockInfoModel.setRawDataId(0);
        stockInfoModel.setStockInfoId(stoi(stockInfo.second["stock_info_id"]));
        stockInfoModel.setIsProcessed("Yes"); 
        stockInfoService.updateStockInfo(stockInfoModel);
        count++;
        msgERPInside["Status"] = "Synced to ERP";
      }
      else {logError("Not able to communicate to ERP server");}
      slNo++;
      msgERP[stockInfo.first] = msgERPInside;
    }
    logMsg("Completed iQC information " +  to_string(count) + " to ERP...");
    return msgERP;
  };


  public:void ChecksResultSync(std::string CheckName,string partconfigID="", json jsonData={}){
    using json = nlohmann::json;
    StockInfoService stockinfoservice;
    PartConfigService partconfigservice;
    json jsonresultObject;
    
    if(partconfigID!=""){
      map<int, std::map<std::string, std::string>> PARTFIELDMAPlIST = partconfigservice.getPartFieldMappingList();
      std::map<int, std::map<std::string, std::string>> partConfigList = partconfigservice.getPartConfigList(to_string(current_stock_info_id));
      json jsonPartArray = json::array();
      for (auto partConfigInfo: partConfigList) {
          std::map<std::string, std::string> partMapping;
          for (auto partConfigData : partConfigInfo.second) {
            string partMapKey = PARTFIELDMAPlIST[stoi(partConfigInfo.second["stock_part_id"])][partConfigData.first];
            if (partMapKey == "") {partMapKey = partConfigData.first;}
            partMapping[partMapKey] = partConfigData.second;
          } 
          json jsonPartObject = partMapping; //partConfigInfo.second;
          jsonPartArray.insert(jsonPartArray.end(), jsonPartObject);
           
      }
      jsonresultObject["njqca_item_component"]=jsonPartArray;
      // cout<<jsonresultObject.dump(2)<<endl;
    }
    if(!jsonData.empty()){
      for(auto & [key,value]:jsonData.items()){
        jsonresultObject[key]=value;
      }
    }
    // cout<<"\nStaring updateing "<<CheckName<<" Results to ERP..."<<endl;
    
    std::map<int, std::map<std::string, std::string>> partResultList = partresultservice.getPartResultList(to_string(current_stock_info_id));
    json jsonResultArray = json::array();
    for(const auto&[key, result]: partResultList){
       json resultObject=result;
       jsonResultArray.push_back(resultObject);
      }
    jsonresultObject["njqca_result_parameters"]=jsonResultArray;
    // cout<<jsonresultObject.dump(2)<<endl;

    map<string,string>requestMap;
    if(config_updated_name!=""){
      string updated_name="/"+util.encodeURL(config_updated_name);
      // cout<<updated_name<<endl;
      string response = api2ERP(constant.API_DOC_URL + docType.IQC + updated_name , "PUT", requestMap, to_string(jsonresultObject));
      logInfo(response);
      if(response!=""){
        // StockInfoModel stockInfoModel;
        // map<string,string>wheremap;
        // wheremap["stock_info_id"]=to_string(current_stock_info_id);
        // stockInfoModel.setStockInfoId(current_stock_info_id);
        // stockInfoModel.setIsProcessed("Yes"); 
        // stockinfoservice.updateStockInfobyID(stockInfoModel,wheremap);
        cout<<"\n"<<CheckName<<" Results are synced to ERP..."<<endl;
      }else{
        // StockInfoModel stockInfoModel;
        // // stockInfoModel.setErpIqcName(config_updated_name);
        // // stockInfoModel.setRawDataId(0);
        // map<string,string>wheremap;
        // wheremap["stock_info_id"]=to_string(current_stock_info_id);
        // stockInfoModel.setStockInfoId(current_stock_info_id);
        // stockInfoModel.setIsProcessed("No"); 
        // stockinfoservice.updateStockInfobyID(stockInfoModel,wheremap);
        cout<<"\n"<<CheckName<<" Results are  Not synced to ERP..."<<endl;
      }
    }
    else{
      cout<<"\nConfig is not Updated to ERP. Please try after update of configuration... "<<std::flush<<endl;
      // ChecksResultSync(CheckName,"push to ERP",jsonData);
    }
   
  }

  public:bool ResultSync(std::string CheckName,json results){
    using json = nlohmann::json;
    json inputJson;
    inputJson["data"] = to_string(results);
    // cout<<inputJson.dump(2)<<endl;
    PartConfigService partconfigservice;

    map<string,string>requestMap;
    if(config_updated_name!=""){
      // cout<<updated_name<<endl;
      string response = getParameterList(constant.API_resultsync_URL , "post", requestMap, to_string(inputJson));
      // cout<<response<<endl;
      logInfo(response);
      if(response!=""){
      try{
        json responseJson = json::parse(response);
        if(responseJson["message"]["name"]!=""){
          cout<<"\n"<<CheckName<<" Results are synced to ERP..."<<endl;
          return true;
        }else{
          cout<<"\n"<<CheckName<<" Results are  Not synced to ERP..."<<endl;
          return false;
        }
      }
      catch(const std::exception& e){
        cout<<"\n"<<CheckName<<" Results are  Not synced to ERP..."<<endl;
        return false;
      }
    }else{
      cout<<"\n"<<CheckName<<" Results are  Not synced to ERP..."<<endl;
      return false;
    }
    }
    else{
      cout<<"\nConfig is not Updated to ERP. Please try after update of configuration... "<<std::flush<<endl;
    }
    return false;
  }
  //update the status after completing the syntax
  private: int updateAPICompleted(string tableName, string primaryKeyName, map<int, string> primaryKeyMap) {
    Database updateDatabase;
    string updateWhereData;
    std::map<std::string, string> fieldMap;
    for (auto i = primaryKeyMap.begin(); i != primaryKeyMap.end(); i++) {
      if (updateWhereData != "") {updateWhereData += " or ";}
      updateWhereData += primaryKeyName + "= '" + i -> second + "'";
    }
    if (updateWhereData != "") {updateWhereData = " (" + updateWhereData + ") ";}

    fieldMap["is_processed"] = "Yes";
    updateDatabase.whereMap["#"] = updateWhereData;
    return updateDatabase.update(tableName, fieldMap, true);
  }

  public: string getNJNo(string serialNo) {
    bool isFound = false;
    bool isNJFound = false;
    string njSerialNo = "";
    map<string, string> requestMap;
    string method = "POST";
    json jsonObject;
    requestMap["filters"] = "[[\"serial_no\",\"=\",\"" + serialNo + "\"]]"; //,[\"docstatus\",\"=\",\"0\"]
    response = api2ERP(constant.API_DOC_URL + docType.SERIAL_NO, method, requestMap, to_string(jsonObject));
    logInfo(response);
    if (response == "") {
      
      while (! isNJFound) {
          std::cout << "Bios Number " << serialNo  <<" not found in NJERP. Plesae enter the NJ number / Bar code number: ";
          std::cin >> njSerialNo;
          requestMap["filters"] = "[[\"barcode\",\"=\",\"" + njSerialNo + "\"]]"; //,[\"docstatus\",\"=\",\"0\"]
          response = api2ERP(constant.API_DOC_URL + docType.SERIAL_NO, method, requestMap, to_string(jsonObject));
          if (response != "") {isNJFound = true;} else {std::cout << "Incorrect NJ number / Bar code number. Please enter the again.\n";}
      }
    }
    return njSerialNo;
  }

  //Creating the data in ERP
  public: string api2ERP(string url, string method, map<string, string> requestMap, string postData) {
    RestAPI restAPI;
    RestAPIModel restAPIModel1;
    response = "";
    map<std::string, string> header;
    header["Authorization"] = config::TOKEN;
    // header["Content-Type"] = "application/json";
    header["Accept"] = "application/json";
    header["charset"] = "utf-8";
    restAPIModel1.setApiURL(url);
    restAPIModel1.setRequestJSON(true);
    restAPIModel1.setHeader(header);
    restAPIModel1.setRequestMethod(method);

    if (method == "post" || method == "PUT") {
      restAPIModel1.setPostFields(postData);
      restAPI.postRequest(restAPIModel1, response);
      logInfo(response);
    }
    else {
      restAPIModel1.setPostFields("");
      restAPIModel1.setRequestKeyPairMap(requestMap);
      restAPI.getRequest(restAPIModel1, response);
      logInfo(response);
    }
    // cout<<"response from ERP:"<<response<<endl;
    if (response != "") {
       try {
        // try to use an array index that is not a number
        json jsonObject = json::parse(response);
        //logInfo(jsonObject.dump(2));
        if (jsonObject.contains("exc_type")) {
          logError("ERP Error Message:" + jsonObject["_server_messages"].dump(2));
          response = "";
        }
        else {
          //logInfo(method);
          if (method == "post" || method == "PUT") {
            //logInfo("entered...");
            //logInfo(jsonObject["data"]["name"].dump(2));
            response = util.json2String(jsonObject["data"]["name"]);
            // cout<<"response after put operation:"<<response<<endl;
          } 
          else {
            response = "";
            for (const auto& item : jsonObject.items()) {
              for (const auto& itemName : item.value().items()) {
                response = itemName.value()["name"];
              }
            }
          }
        }
      }
      catch (const json::parse_error& e) {
        std::cout << "Parse Error in the ERP response:"<<e.what() << '\n';
      }
      catch(const std::exception& e) {
        std::cerr << "Error in the ERP response: " << e.what() << '\n';
      }
    }
    else {logError("Not able to communicate to ERP server");}
    return response;
  }
  public :string configname(){
    return draft_name;
  }

   public: string getParameterList(string url, string method, map<string, string> requestMap, string postData) {
    RestAPI restAPI;
    RestAPIModel restAPIModel1;
    response = "";
    map<std::string, string> header;
    header["Authorization"] = config::TOKEN;
    header["Content-Type"] = "application/json";
    header["Accept"] = "application/json";
    header["charset"] = "utf-8";
    // cout<<url<<endl;
    restAPIModel1.setApiURL(url);
    restAPIModel1.setRequestJSON(true);
    restAPIModel1.setHeader(header);
    restAPIModel1.setRequestMethod(method);

    if (method == "post" || method == "PUT") {
      restAPIModel1.setPostFields(postData);
      restAPI.postRequest(restAPIModel1, response);
      logInfo(response);
    }
    else {
      restAPIModel1.setPostFields("");
      restAPIModel1.setRequestKeyPairMap(requestMap);
      restAPI.getRequest(restAPIModel1, response);
      logInfo(response);
    }
    return response;
   }
    std::string sanitize_utf8(const std::string& input) {
    std::string output;
    for (unsigned char c : input) {
        if ((c >= 0x20 && c <= 0x7E) || c == '\n' || c == '\r' || c == '\t') {
            output += c;  
        }     
    }
    return output;
   }
};
int IQCService::current_stock_info_id ;
string IQCService::draft_name ;
string IQCService::config_updated_name ;
string IQCService::model_name;
string IQCService::bios_serial_no="";
string IQCService::item_group;
float IQCService::njqca_version=0.0;
bool IQCService::isNJQCAStored = false;
string IQCService::loggedUser = "";
string IQCService::selected_inspection_type = "";