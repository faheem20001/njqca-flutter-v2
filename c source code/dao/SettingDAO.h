#include <iostream>
#include <string>
#include <list>
#include <map>
#include <../model/SettingModel.h>

using namespace std;

class SettingDAO: public Database {
  TableName tableName;

  public: map<int, map<string, string>> getSettingList() {
    setDefault();
    getDataList(tableName.SETTING + " as setting ");
    return returnData;
  }
  
  /*public: int updateSetting(settingModel settingModel, std::map<std::string, string> conditionMap) {
		int returnValue = 0;
    std::map<std::string, string> fieldDataMap;
    if (settingModel.getSettingId() != 0) {fieldDataMap["setting_id"] = to_string(userModel.getSettingId());}
    if (settingModel.getUserName() != "") {fieldDataMap["user_name"] = userModel.getUserName();}
    if (settingModel.getPassword() != "") {fieldDataMap["password"] = userModel.getPassword();}
    if (settingModel.getStatus() != "") {fieldDataMap["status"] = userModel.getStatus();}

    //std::cout << "User: " << userModel.getUserName();

		returnValue = update(tableName.USER, fieldDataMap);
		return returnValue;
	}*/

  private: void setDefault() {
    if (selectFieldNameList.size() == 0 ) {
      selectFieldNameList = {
        "settingValue.setting_value"
      };
    }
    if (selectJoinList.size() == 0)
      {selectJoinList = {"left join " + tableName.SETTING_VALUE + " as settingValue on settingValue.setting_id = setting.setting_id"};}
  }
};