#include <iostream>
#include <string>
#include <list>
#include <map>
#include <../model/ModelModel.h>
//#include <../lib/freshweb/database.h>

using namespace std;

class ModelDAO: public Database {
  TableName tableName;

  public: map<int, map<string, string>> getModelDataList() {
    setDefault();
    getDataList(tableName.MODEL + " as model ");
    return returnData;
  }
  
  public: int updateModel(ModelModel modelModel, std::map<std::string, string> conditionMap) {
		int returnValue = 0;
    std::map<std::string, string> fieldDataMap;
    if (modelModel.getModelId() != 0) {fieldDataMap["model_id"] = to_string(modelModel.getModelId());}
    if (modelModel.getManufacturerId() != 0) {fieldDataMap["manufacturer_id"] = to_string(modelModel.getManufacturerId());}
    if (modelModel.getModelName() != "") {fieldDataMap["model_name"] = modelModel.getModelName();}
    if (modelModel.getStatus() != "") {fieldDataMap["status"] = modelModel.getStatus();}

    std::cout << "Model: " << modelModel.getModelName();

		returnValue = update(tableName.MODEL, fieldDataMap);
		return returnValue;
	}

  private: void setDefault() {
    if (selectFieldNameList.size() == 0 ) {
      selectFieldNameList = {
        "model.model_id", "model.model_name", "model.manufacturer_id", "model.status",
        "manufacturer.manufacturer_name"
      };
    }
    if (selectJoinList.size() == 0)
      {selectJoinList = {"left join " + tableName.MANUFACTURER + " as manufacturer on model.manufacturer_id = manufacturer.manufacturer_id"};}
  }
};