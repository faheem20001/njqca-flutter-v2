#include <map>
#include <../dao/SettingDAO.h>

using namespace std;

class SettingService: public V2Service {
  
  private:
    SettingModel settingModel;
    SettingDAO settingDAO;

  //return one parameter from the given module and parameter.
  public: string getSettingParam(string module, string param) {
    map<string, string> settingData = getSettingModule(module);
    return settingData["setting_value"];
  }

  //return first record from the database
  public: map<string, string> getSettingModule(string module) {
    whereMap["module"] = module;
    map<int, map<string, string>> settingList = getSettingList();
    return settingList[0];
  }

  public: map<int, map<string, string>> getSettingList() {
    setDefault();
    return settingDAO.getSettingList();
  }

  private: void setDefault() {
    if (whereMap.size() != 0) {settingDAO.whereMap = whereMap;}
    if (orderMap.size() != 0) {settingDAO.orderMap = orderMap;}
    if (start != "") {settingDAO.start = start;}
    if (length != "") {settingDAO.length = length;}
  }
};