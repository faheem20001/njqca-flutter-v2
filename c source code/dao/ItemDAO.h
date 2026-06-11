#include <iostream>
#include <string>
#include <list>
#include <map>

using namespace std;

class ItemDAO: public Database {
  TableName tableName;

  public: std::map<int, map<string, string>> getItemList() {
    setDefault();
    getDataList(tableName.ITEM + " as item ");
    return returnData;
  }
  
  public:
	int updateItem(ItemModel itemModel, std::map<std::string, string> conditionMap) {
		int returnValue = 0;
    std::map<std::string, string> fieldDataMap;
    if (itemModel.getItemId() != 0) {fieldDataMap["item_id"] = to_string(itemModel.getItemId());}
    if (itemModel.getItemTypeId() != 0) {fieldDataMap["item_id"] = to_string(itemModel.getItemTypeId());}
    if (itemModel.getItemName() != "") {fieldDataMap["item_name"] = itemModel.getItemName();}
    if (itemModel.getItemCode() != "") {fieldDataMap["item_code"] = itemModel.getItemCode();}
    if (itemModel.getItemNjhatCode() != "") {fieldDataMap["item_njhat_code"] = itemModel.getItemNjhatCode();}
    if (itemModel.getItemStatus() != "") {fieldDataMap["item_status"] = itemModel.getItemStatus();}

		returnValue = update(tableName.ITEM, fieldDataMap);
		return returnValue;
	}

  private: void setDefault() {
    if (selectFieldNameList.size() == 0 ) {
      selectFieldNameList = {
        "item.item_id", "item.item_name", "item.item_type_id", "item.item_code", "item.item_njhat_code", "item.exec_func", "item.item_status"
      };
    }
    /*if (selectJoinList.size() == 0)
      {selectJoinList = {"left join " + tableName.MANUFACTURER + " as manufacturer on model.manufacturer_id = manufacturer.manufacturer_id"};}*/
  }
};