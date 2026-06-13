#include <iostream>
#include <string>
#include <list>
#include <map>

using namespace std;

class ItemBomDAO: public Database {
  TableName tableName;

  public: map<int, map<string, string>> getItemBomList() {
    setDefault();
    getDataList(tableName.ITEM_BOM + " as itemBom ");
    return returnData;
  }
  
  public: int updateItemBom(ItemBomModel itemBomModel, std::map<std::string, string> conditionMap) {
		int returnValue = 0;
    std::map<std::string, string> fieldDataMap;
    if (itemBomModel.getItemBomId() != 0) {fieldDataMap["item_bom_id"] = to_string(itemBomModel.getItemBomId());}
    if (itemBomModel.getItemId() != 0) {fieldDataMap["item_id"] = to_string(itemBomModel.getItemId());}
    if (itemBomModel.getItemPartId() != 0) {fieldDataMap["item_part_id"] = to_string(itemBomModel.getItemPartId());}
    if (itemBomModel.getIsItemCode() != "") {fieldDataMap["is_item_code"] = itemBomModel.getIsItemCode();}
    if (itemBomModel.getIsRemovable() != "") {fieldDataMap["is_removable"] = itemBomModel.getIsRemovable();}
    if (itemBomModel.getItemBomStatus() != "") {fieldDataMap["item_bom_status"] = itemBomModel.getItemBomStatus();}

		returnValue = update(tableName.ITEM_BOM, fieldDataMap);
		return returnValue;
	}

  private: void setDefault() {
    if (selectFieldNameList.size() == 0 ) {
      selectFieldNameList = {
        "itemBom.item_bom_id", "itemBom.item_id", "itemBom.item_part_id", "itemBom.is_item_code", "itemBom.is_removable", "itemBom.item_bom_status", 
          "item.item_name", "itemPart.exec_func", "itemPart.item_njhat_code",
          "itemPart.item_name as itemPartName"
      };
    }
    if (selectJoinList.size() == 0) {
      selectJoinList = {
        "left join " + tableName.ITEM + " as item on itembom.item_id = item.item_id",
        "left join " + tableName.ITEM + " as itemPart on itembom.item_part_id = itemPart.item_id"
      };

    }

    whereMap["item_bom_status"] = "Active";
  }
};