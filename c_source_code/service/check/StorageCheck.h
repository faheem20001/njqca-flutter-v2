#include <iostream>
#include <string>
#include <map>
// #include <../service/PartResultService.h>

using namespace std;
using json = nlohmann::json;

class StorageCheck: public V2Service {

  private:
    const string CMD_STORAGE = "smartctl -A -j /dev/nvme0n1";

    Util util;
    PartResultService partResultService;

  /*
  * Gets configuration of the system. 
  */

  public: int getTestResult(int stockInfoId, int itemId, int partConfigId, int partId) {
    int returnValue = 0;
    logMsg("Storage Test - StockInfo Id:  ");
    map<std::string, std::string> partResultMap;
    json jsonObject = util.executeTerminal(CMD_STORAGE, true);

  //convertJSON2Map (jsonObject);

    //returnValue = partResultService.updateBulkPartResult(partResultMap);
    return returnValue;
  }

  /*@Todo: Need to clean up and move to Util*/
  private: map<std::string, std::string> convertJSON2Map (json jsonObject) {
    std::map<std::string, std::string> partResultMap;
    for (const auto& item : jsonObject.items()) {
      if (item.value().is_object()) {
        map<std::string, std::string> partResultSubMap = getJsonObjectInfo(item.key(), item.value());
        partResultMap.insert(partResultSubMap.begin(),partResultSubMap.end());
      }
      if (item.value().is_array()) {
        map<std::string, std::string> partResultSubMap = getJsonObjectInfo1(item.key(), item.value());
        partResultMap.insert(partResultSubMap.begin(),partResultSubMap.end());
      }
    }
    return partResultMap;
  }

  private: map<std::string, std::string> getJsonObjectInfo(string objectKey, json jsonSubObject) {
    map<std::string, std::string> partSubMap;
    for (const auto& item : jsonSubObject.items()) {
      /*if (item.value().is_array) {
        map<std::string, std::string> partSubMap1;
        partSubMap1 = getJsonObjectInfo1(item.key() + "-" + objectKey, item.value().dump(0));
        partSubMap.insert(partSubMap1.begin(),partSubMap1.end());
      }*/
      partSubMap[objectKey + "-" + item.key()] = item.value().dump(0);

    }
    return partSubMap;
  }

  private: map<std::string, std::string> getJsonObjectInfo1(string arrayKey, json jsonSubArray) {
    map<std::string, std::string> partSubMap;
    for (int i = 0; i < jsonSubArray.size(); i++) {
      partSubMap[arrayKey + "-" + to_string(i)] = to_string(jsonSubArray[i]);
    }
    return partSubMap;
  }
};