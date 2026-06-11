#include <iostream>
#include <string>

using namespace std;

class SettingModel {
  private: int settingId = 0;
  private: int parentId = 0;
  private: string parameter;
  private: string module;
  private: string moduleType;
  private: string displayOrder;
  private: string settingValue;

  public: int getSettingId() {return settingId;}
  public: void setSettingId(int settingIdLocal) {settingId = settingIdLocal;}

  public: string getParameter() {return parameter;}
  public: void setParameter(string parameterLocal) {parameter = parameterLocal;}

  public: string getModule() {return module;}
  public: void setModule(string moduleLocal) {module = moduleLocal;}

  public: string getModuleType() {return moduleType;}
  public: void setModuleType(string moduleTypeLocal) {module = moduleTypeLocal;}

  public: string getDisplayOrder() {return displayOrder;}
  public: void setDisplayOrder(string displayOrderLocal) {module = displayOrderLocal;}

  public: string getSettingValue() {return settingValue;}
  public: void setSettingValue(string settingValueLocal) {module = settingValueLocal;}
};