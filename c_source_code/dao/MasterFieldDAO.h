#include <iostream>
#include <string>
#include <list>
#include <map>

using namespace std;

class MasterFieldDAO: public Database {
  TableName tableName;

  public: map<int, map<string, string>> getMasterFieldList() {
    setDefault();
    getDataList(tableName.MASTER_FIELD + " as masterField ");
    return returnData;
  }

  private: void setDefault() {
    if (selectFieldNameList.size() == 0 ) {
      selectFieldNameList = {
        "masterField.master_field_id", "masterField.classification_id", "masterField.classification", "masterField.field_label", "masterField.field_name", "masterField.erp_field_name",  
            "masterField.prefix", "masterField.suffix", "masterField.exec_function", "masterField.field_type", "masterField.field_data", "masterField.mandatory", "masterField.display_order", "masterField.status"
      };
    }
    /*if (selectJoinList.size() == 0) {
      selectJoinList = {};

    }*/
    whereMap["status"] = "Active";
  }
};