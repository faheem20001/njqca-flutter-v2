#include <iostream>
#include <string>
#include <map>

using namespace std;

class ManufacturerDAO: public Database {
  //Database database;
  TableName tableName;

  public: map<int, map<string, string>> getManufacturerList() {
    setDefault();
    getDataList(tableName.MANUFACTURER + " as manufacturer ");
    return returnData;
  }

  public: int updateManufacturer(ManufacturerModel manufacturerModel) {
    std::map<std::string, string> conditionMap;
    return updateManufacturer(manufacturerModel, conditionMap);
  }
  
  public: int updateManufacturer(ManufacturerModel manufacturerModel, std::map<std::string, string> conditionMap) {
		int returnValue = 0;
    std::map<std::string, string> fieldDataMap;
    if (manufacturerModel.getManufacturerId() != 0) {fieldDataMap["manufacturer_id"] = to_string(manufacturerModel.getManufacturerId());}
    if (manufacturerModel.getManufacturerName() != "") {fieldDataMap["manufacturer_name"] = manufacturerModel.getManufacturerName();}
    if (manufacturerModel.getStatus() != "") {fieldDataMap["status"] = manufacturerModel.getStatus();}

		returnValue = update(tableName.MANUFACTURER, fieldDataMap);
		return returnValue;
	}

  private: void setDefault() {
    if (selectFieldNameList.size() == 0 ) {
      selectFieldNameList = {
        "manufacturer.manufacturer_id", "manufacturer.manufacturer_name", "manufacturer.status"
      };
    }
  }
};