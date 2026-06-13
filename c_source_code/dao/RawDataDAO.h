#include <iostream>
#include <string>
#include <map>

using namespace std;

class RawDataDAO: public Database {
  public: int updateRawData(RawDataModel rawDataModel, std::map<std::string, string> conditionMap) {
		int returnValue = 0;
    std::map<std::string, string> fieldDataMap;
    if (rawDataModel.getRawDataId() != 0) {fieldDataMap["raw_data_id"] = to_string(rawDataModel.getRawDataId());}
    if (rawDataModel.getManufacturerId() != 0) {fieldDataMap["manufacturer_id"] = to_string(rawDataModel.getManufacturerId());}
    if (rawDataModel.getManufacturerName() != "") {fieldDataMap["manufacturer_name"] = rawDataModel.getManufacturerName();}
    if (rawDataModel.getModelId() != 0) {fieldDataMap["model_id"] = to_string(rawDataModel.getModelId());}
    if (rawDataModel.getModelName() != "") {fieldDataMap["model_name"] = rawDataModel.getModelName();}
    if (rawDataModel.getSerialNo() != "") {fieldDataMap["serial_no"] = rawDataModel.getSerialNo();}
    if (rawDataModel.getJsonData() != "") {fieldDataMap["json_data"] = rawDataModel.getJsonData();}
    if (rawDataModel.getIsProcessed() != "") {fieldDataMap["is_processed"] = rawDataModel.getIsProcessed();}

		returnValue = updateRawData(fieldDataMap, conditionMap);
		return returnValue;
	}

  public: int updateRawData(std::map<std::string, string> fieldDataMap, std::map<std::string, string> conditionMap) {
		return update(tableName.RAW_DATA, fieldDataMap);
	}
};