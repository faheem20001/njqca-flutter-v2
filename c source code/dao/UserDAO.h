#include <iostream>
#include <string>
#include <list>
#include <map>
#include <../model/UserModel.h>

using namespace std;

class UserDAO: public Database {
  TableName tableName;

  public: map<int, map<string, string>> getUserDataList() {
    setDefault();
    getDataList(tableName.USER + " as user ");
    return returnData;
  }
  
  public: int updateUser(UserModel userModel, std::map<std::string, string> conditionMap) {
		int returnValue = 0;
    std::map<std::string, string> fieldDataMap;
    if (userModel.getUserId() != 0) {fieldDataMap["user_id"] = to_string(userModel.getUserId());}
    if (userModel.getUserName() != "") {fieldDataMap["user_name"] = userModel.getUserName();}
    if (userModel.getPassword() != "") {fieldDataMap["password"] = userModel.getPassword();}
    if (userModel.getStatus() != "") {fieldDataMap["status"] = userModel.getStatus();}

    // std::cout << "User: " << userModel.getUserName();

		returnValue = update(tableName.USER, fieldDataMap);
		return returnValue;
	}

  private: void setDefault() {
    if (selectFieldNameList.size() == 0 ) {
      selectFieldNameList = {
        "user.user_id", "user.user_name", "user.password", "user.status"
      };
    }
    if (selectJoinList.size() == 0)
      {/*selectJoinList = {"left join " + tableName.MANUFACTURER + " as manufacturer on model.manufacturer_id = manufacturer.manufacturer_id"};*/}
  }
};