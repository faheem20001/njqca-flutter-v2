#include <iostream>
#include <string>
#include <list>
#include <map>


using namespace std;
using json = nlohmann::json;

class PartConfigDAO: public Database {
  TableName tableName;

  public: map<int, map<string, string>> getPartConfigList() {
    setDefault();
    getDataList(tableName.PART_CONFIG + " as partConfig ");
    return returnData;
  }
  
  public: int updatePartConfig(map<std::string, string> fieldDataMap, map<std::string, string> conditionMap) {
		int returnValue = 0;
		returnValue = update(tableName.PART_CONFIG, fieldDataMap);
		return returnValue;
	}

  private: void setDefault() {
    if (selectFieldNameList.size() == 0 ) {
      selectFieldNameList = {
        "partConfig.part_config_id", "partConfig.stock_part_id, partConfig.serial_no, partConfig.attr_01, partConfig.attr_02, partConfig.attr_03, partConfig.attr_04, partConfig.attr_05, partConfig.attr_06, partConfig.attr_07, partConfig.attr_08, partConfig.attr_09, partConfig.attr_10, partConfig.attr_11, partConfig.attr_12, partConfig.attr_13, partConfig.attr_14, partConfig.attr_15, partconfig.attr_16, "
        "item.item_name as njhat_item_name, erp_item_name as item_group ","partConfig.stock_info_id", "stock.item_id","stock.created_by"
      };
    }
    if (selectJoinList.size() == 0) {
      selectJoinList = {
        "left join " + tableName.ITEM + " as item on item.item_id = partConfig.stock_part_id " ,
        "left join " + tableName.STOCK_INFO + " as stock on partConfig.stock_info_id = stock.stock_info_id"
      };

    }
  }
};