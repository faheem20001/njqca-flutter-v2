#include <iostream>
#include <string>
#include <list>
#include <map>

using namespace std;

class PartResultDAO: public Database {
  TableName tableName;

  public: map<int, map<string, string>> getPartResultList() {
    setDefault();
    getDataList(tableName.PART_RESULT + " as partResult ");
    return returnData;
  }

  public: int updatePartResult(map<std::string, string> fieldDataMap) {
		int returnValue = 0;
    map<std::string, string> conditionMap;
		returnValue = updatePartResult(fieldDataMap, conditionMap);
		return returnValue;
	}
  
  public: int updatePartResult(map<std::string, string> fieldDataMap, map<std::string, string> conditionMap) {
		int returnValue = 0;
		returnValue = update(tableName.PART_RESULT, fieldDataMap);
		return returnValue;
	}

  private: void setDefault() {
    if (selectFieldNameList.size() == 0 ) {
      selectFieldNameList = {
           "partResult.part_config_id", "partResult.param as parameter","partResult.result as value","partResult.score as score","partResult.cpu_load as cpu_load","partResult.discharge_rate as discharge_rate","item.erp_item_name as item_group"

        // "partResult.part_result_id", "partResult.stock_info_id", "partResult.stock_part_id", "partResult.param", "partResult.paramResult.result", 
        //   "stockInfo.serial_no, "
        //   "item.item_name", "itemPart.item_njhat_code",
        //   "itemPart.item_name as itemPartName"
      };
    }
    if (selectJoinList.size() == 0) {
      selectJoinList = {
        "left join " + tableName.STOCK_INFO + " as stockInfo on partResult.stock_info_id = stockInfo.stock_info_id ",
        "left join " + tableName.ITEM + " as item on item.item_id = partResult.part_id"
    //     "left join " + tableName.ITEM + " as itemPart on item.item_id = partResult.item_part_id"
      };

    }
  }
};