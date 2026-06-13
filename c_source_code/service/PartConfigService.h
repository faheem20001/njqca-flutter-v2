#include <map>
#include <../service/MasterFieldService.h>
#include <../dao/PartConfigDAO.h>

using namespace std;

class PartConfigService: public V2Service {
  
  private:
    PartConfigDAO partConfigDAO;

  //return first record from the database
  public: map<string, string> getPartConfigData() {
    std::map<int, map<string, string>> partConfigList = getPartConfigList();
    return partConfigList[0];
  }

  public: std::map<int, map<std::string, std::string>> getPartConfigList(string stockInfoId) {
    std::map<int, map<std::string, std::string>> returnMapList;
    //std::map<std::string, std::string> whereMap;
    whereMap["stock.stock_info_id"] = stockInfoId;
    setDefault();
    returnMapList = getPartConfigList();
    return returnMapList;
  }

  public: map<int, map<string, string>> getPartConfigList() {
     std::map<int, map<std::string, std::string>> returnMapList;
    setDefault();
    returnMapList = partConfigDAO.getPartConfigList();
    return returnMapList;
  }

  public: map<int, map<string,string>>getPartConfigbyID(string partconfigID){
     std::map<int, map<std::string, std::string>> returnMapList;
    //std::map<std::string, std::string> whereMap;
    whereMap["partConfig.part_config_id"] = partconfigID;
    setDefault();
    returnMapList = getPartConfigList();
    return returnMapList;
  }

  /*Map the JSON result with the mapfield and update in the database*/
  public: int updatePartConfig(string stockInfoId, map<int, map<string, string>> itemPartConfigMap) {
    int return_value=0;
    for (int i = 0; i < itemPartConfigMap.size(); ++i) {
      MasterFieldService masterFieldService;
      PartConfigService partConfigService;
      map<int, map<std::string, std::string>> partFieldMap = masterFieldService.getMasterFieldList(itemPartConfigMap[i]["part_id"]);
      map<std::string, std::string> partConfigMap;
      for (int j = 0; j < partFieldMap.size(); ++j) {
        map<std::string, std::string> partFieldRowMap = partFieldMap[j];
        string jsonKey = partFieldRowMap["field_label"];
        // transform(jsonKey.begin(), jsonKey.end(), jsonKey.begin(), ::tolower);
        string value = convertFieldValue(partFieldRowMap, itemPartConfigMap[i][jsonKey]);
        partConfigMap[partFieldRowMap["field_name"]] = value;
      }
      partConfigMap["stock_part_id"] = itemPartConfigMap[i]["part_id"];
      partConfigMap["stock_info_id"] = stockInfoId;
      return_value= partConfigService.updatePartConfig(partConfigMap);
    }
    return return_value;
  }

  private: string convertFieldValue(map<std::string, std::string> partFieldRowMap, string value) {
    string suffix = ""; string prefix = ""; string returnValue = "";
    if (value != "") {
      //if (partFieldRowMap["classification_id"] == "4") {std::cout << partFieldRowMap["prefix"];}//util.printMap(partFieldRowMap);}
      if (partFieldRowMap["prefix"] != "") {prefix = partFieldRowMap["prefix"] + " ";}
      if (partFieldRowMap["suffix"] != "") {suffix = " " + partFieldRowMap["suffix"];}
      value = execFunction(partFieldRowMap["exec_function"], value);
      value = prefix + value + suffix;
      //if (partFieldRowMap["classification_id"] == "4") {std::cout << returnValue << "\n";}
      transform(value.begin(), value.end(), value.begin(), ::toupper);
    }
    return value;
  }

  private: string execFunction(string execFunction, string value) {
    if (execFunction != "") {
      map<int, std::string> execFunctionSplitArray =  util.splitString(execFunction, "::");
      if (execFunctionSplitArray[0] == "divide" && value != "") {
        double intValue = stod(value);
        value = to_string(static_cast<int>(intValue / stod(execFunctionSplitArray[1])));
      }
      if (execFunctionSplitArray[0] == "stringReplace") {value = util.removeString(value, execFunctionSplitArray[1]);}
    }
    return value;
  }

  public:
	  int updatePartConfig(std::map<std::string, string> fieldDataMap) {
    std::map<std::string, string> conditionMap;
    return updatePartConfig(fieldDataMap, conditionMap);
  }

  public:
	int updatePartConfig(std::map<std::string, string> fieldDataMap, std::map<std::string, string> conditionMap) {
    return partConfigDAO.updatePartConfig(fieldDataMap, conditionMap);
  }

  public: map<int, map<string, string>> getPartFieldMappingList() {
    MasterFieldService masterFieldService;
    setDefault();
    //masterFieldService.whereMap["classification_id"] = "3";
    masterFieldService.length = "-1";
    map<int, map<string, string>> fieldMappingList = masterFieldService.getMasterFieldList();
    map<int, map<string, string>> partMappingList;
    for (auto fieldInfo : fieldMappingList) {
      map<string, string> fieldData = fieldInfo.second;
      //if (fieldData["classification_id"] == "3") {util.printMap(fieldInfo.second);}
      partMappingList[stoi(fieldData["classification_id"])][fieldData["field_name"]] = fieldData["erp_field_name"];
    }
    //util.printMap(partMappingList[3]);
    return partMappingList;
  }

  private: void setDefault(){
    if (whereMap.size() != 0) {partConfigDAO.whereMap = whereMap;}
    if (orderMap.size() != 0) {partConfigDAO.orderMap = orderMap;}
    if (start != "") {partConfigDAO.start = start;}
    if (length != "") {partConfigDAO.length = length;}
  }
};