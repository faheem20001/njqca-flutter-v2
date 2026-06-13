#include <map>
#include <../service/GetFromERPService.h>

using namespace std;

class PurchaseReceiptService: public V2Service {

  private: 
    Constant constant;
    DocType docType;
    GetFromERPService getFromERPservice;
  
  public: bool isPurchaseReceipt(string prNo) {
    bool isFound = false;
    std::map<std::string, string> requestMap;
    requestMap["filters"] = "[[\"name\",\"=\",\"" + prNo + "\"]]";
    string response = getFromERPservice.api2ERP(constant.API_DOC_URL + docType.PURCHASE_RECEIPT, requestMap);
    logInfo(response);
    json jsonData = json::parse(response);
    //std::cout << jsonData["data"].size();
    if (jsonData["data"].size() > 0) {isFound = true;}
    return isFound;
  };

  public: map<int, map<string,string>> getItemList(string prNo) {
    std::map<std::string, string> requestMap;
    map<int, map<string,string>> returnData;
    string response = getFromERPservice.api2ERP(constant.API_DOC_URL + docType.PURCHASE_RECEIPT + "/" + prNo, requestMap);
    json jsonData = json::parse(response);
    for (const auto& item : jsonData["data"]["items"]) {
      std::map<std::string, string> itemDataMap;
      itemDataMap["purchaseOrder"] = item["purchase_order"];
      itemDataMap["itemGroup"] = item["item_group"];
      itemDataMap["itemCode"] = item["item_code"];
      itemDataMap["itemName"] = item["item_name"];
      itemDataMap["description"] = item["description"];
      itemDataMap["idx"] = item["idx"].dump(0);
      int idx = stoi(item["idx"].dump(0));
      returnData[idx] = itemDataMap; 
      //util.printMap(itemDataMap);
      //std::cout << item["item_code"] << "\n";
    }

    return returnData;
  };

};