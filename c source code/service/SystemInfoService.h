#include <map>
#include <cctype>
#include <iostream>
#include <string>
#include <list>
#include <any>
#include <typeinfo>

#include <../model/RawDataModel.h>
#include <../service/RawDataService.h>
#include <../service/StockInfoService.h>
#include <../service/sync2ERP/IQCService.h>

using namespace std;
using json = nlohmann::json;

class SystemInfoService: public V2Service {
  private:
  const string CMD_SYSTEM= "powershell -Command \" Get-WmiObject -Class Win32_computersystemproduct\"";

    Util util;
    RawDataService rawDataService;
    StockInfoService stockInfoService;
    IQCService iqcService;

  /*
  * Gets vital system information. 
  */
  public: int getSystemInfo(string userName) { //string poNo, string prNo, string itemCode, string idx
    logMsg("Starting to get the configuration 1/2 ....");
    map<std::string, std::string> systemInfoMap;
    json jsonObject  = util.executeTerminalJSON(CMD_SYSTEM,":");
    // systemInfoMap["jsonData"] = sysResultJSON;
    // json jsonObject = json::parse(sysResultJSON);

    for (const auto& item : jsonObject.items()) {
      //Get product details
			if (item.key() == "Vendor") {systemInfoMap["vendor"] = item.value();}
			if (item.key() == "Name") {systemInfoMap["Name"] = item.value();}
      // if (item.key() == "description") {systemInfoMap["ProductType"] = item.value();}
			if (item.key() == "IdentifyingNumber") {systemInfoMap["serialNo"] = item.value();}
			if (item.key() == "Version") {systemInfoMap["version"] = item.value();}
      // if (item.key() == "product") {systemInfoMap["product"] = item.value();}
     
      
      // logDebug("version:" + systemInfoMap["version"] + "::" "product: " + systemInfoMap["product"]);
    }
    
    if (systemInfoMap["vendor"] !="LENOVO") {
      // map <int,string> productMap = util.splitString(systemInfoMap["product"],"(");
      systemInfoMap["version"] = systemInfoMap["Name"] ;
    }

    // map <int,string> productMap = util.splitString(systemInfoMap["product"],"(");
    // systemInfoMap["make"] = util.trim(productMap[0]);
    // if (systemInfoMap["vendor"] == "Dell Inc."||systemInfoMap["vendor"]=="HP" || systemInfoMap["vendor"]=="Hewlett-Packard") {
    //   string make=util.trim(productMap[1]);
    //   make.erase(std::remove(make.begin(), make.end(), ')'), make.end());
    //   systemInfoMap["make"]=make;
    //   if(systemInfoMap["make"].empty()){ systemInfoMap["make"]=" ";}
    //   systemInfoMap["version"] = util.trim(productMap[0]);//systemInfoMap["make"];
    // }
    
    // logDebug("version:" + systemInfoMap["version"] + "::" "product: " + systemInfoMap["product"]);

    //Check whether serial number is available in the NJERP
    string njSerialNo = ""; //iqcService.getNJNo(systemInfoMap["serialNo"]);

    logMsg("Completed  configuration 1/2 and saved locally...");
    //Update the raw JSON data to the database
    // int rawDataId = rawDataService.updateRawData(systemInfoMap);
    //update the stock information with minimum required data
    // systemInfoMap["rawDataId"] =  std::to_string(rawDataId);
    systemInfoMap["createdBy"] = userName;
    //systemInfoMap["purchaseOrderNo"] = poNo;
    //systemInfoMap["purchaseReceiptNo"] = prNo;
    //systemInfoMap["itemCode"] = itemCode;
    //systemInfoMap["idx"] = idx;
    systemInfoMap["uno"] = njSerialNo;
    //util.printMap(systemInfoMap);
    // for(auto item:systemInfoMap){
    //   cout<<item.first<<"=="<<item.second<<endl;
    // }
    int stockInfoId =  stockInfoService.updateStockInfo(systemInfoMap);
    logMsg("Configuration 1/2 saved locally...");
    return stockInfoId;
  }
};