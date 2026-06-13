#include <map>
#include <../dao/UserDAO.h>

using namespace std;

class UserService: public V2Service {
  
  private:
    UserModel userModel;
    UserDAO userDAO;

  //return first record from the database
  public: map<string, string> getUserById(string userId) {
    whereMap["user_id"] = userId;
    map<string, string> userData = getUserData();
    return userData;
  }
  //return first record from the database
  public: map<string, string> getUserData() {
    map<int, map<string, string>> userDataList = getUserDataList();
    return userDataList[0];
  }

  public: map<int, map<string, string>> getUserDataList() {
    setDefault();
    return userDAO.getUserDataList();
  }

  public: int updateUser(UserModel userModel) {
    std::map<std::string, string> conditionMap;
    return updateUser(userModel, conditionMap);
  }

  public: int updateUser(UserModel userModel, std::map<std::string, string> conditionMap) {
    if (whereMap.size() != 0) {userDAO.whereMap = whereMap;}
    return userDAO.updateUser(userModel, conditionMap);
  }

  private: void setDefault() {
    if (whereMap.size() != 0) {userDAO.whereMap = whereMap;}
    if (orderMap.size() != 0) {userDAO.orderMap = orderMap;}
    if (start != "") {userDAO.start = start;}
    if (length != "") {userDAO.length = length;}
  }
};