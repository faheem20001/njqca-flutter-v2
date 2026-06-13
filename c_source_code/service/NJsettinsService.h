#include <map>
//#include <../service/GetFromERPService.h>
//#include "config/constant.h"

using namespace std;

class NjsettingsService: public V2Service {

  private: 
    Constant constant;
    DocType docType;
    GetFromERPService getFromERPservice;


  public: json getNjSettingsvalues(){
    json resultArray=json::array();
    std::map<std::string, string> requestMap;
    //requestMap["filters"] ="[[\"module\",\"=\",\"Stock\"]]";
    requestMap["filters"] ="[[\"module\",\"=\",\"NJ%20Features\"],[\"doc_type\",\"=\",\"NJQCA\"]]"; //[[\"module\",\"=\",\"NJQCA\"]]";
    // requestMap["filters"]="[[\"department\",\"=\",\"IT - NJ\"]]";
    requestMap["fields"] = "[\"department\",\"parameter\",\"value\"]";
    requestMap["limit"]="80";
    string data_response = getFromERPservice.api2ERP(constant.API_DOC_URL + docType.NJ_SETTINGS, requestMap);
    // cout<<data_response<<"\n"<<endl;
    if(data_response!=""){
      json jsonData = json::parse(data_response);
      // cout<<jsonData.dump(0)<<endl;    
      auto dataArray=jsonData["data"];
      for (const auto& item : dataArray) {
          json filteredItem = {
            {"department", item["department"]},
            {"parameter", item["parameter"]},
            {"value", item["value"]}
            // {"role", item["role"]},
            // {"role_profile", item["role_profile"]},
            // {"user", item["user"]}
          };
          resultArray.push_back(filteredItem);
        
      }
    }
    return resultArray;
  }

};