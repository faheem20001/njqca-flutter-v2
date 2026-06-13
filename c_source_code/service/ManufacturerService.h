#include <map>
#include <../model/ManufacturerModel.h>
#include <../dao/ManufacturerDAO.h>

using namespace std;

class ManufacturerService: public V2Service {
  
  private:
    ManufacturerModel manufacturerModel;
    ManufacturerDAO manufacturerDAO;

  //return first record from the database
  public: map<string, string> getManufacturerData() {
    map<int, map<string, string>> manufacturerDataList = getManufacturerList();
    return manufacturerDataList[0];
  }

  public: bool isFound(string manufacturerName){
    bool returnValue = false;
    std::map<std::string, string> manufacturerMap;
    whereMap["manufacturer_name"] = manufacturerName;
    manufacturerMap = getManufacturerData();
    if (manufacturerMap.size() > 0) {returnValue = true;}
    return returnValue;

  }

/*************  ✨ Windsurf Command ⭐  *************/
/**
 * Retrieves a list of manufacturers from the database.
 * 
 * This function sets the default query parameters and retrieves
 * the manufacturer data as a map, where each key is an integer
 * representing the manufacturer ID, and each value is another map
 * containing manufacturer details such as name and status.
 * 

/*******  b187018a-0851-413d-bc5a-116e6071b49c  *******/
  public: map<int, map<string, string>> getManufacturerList() {
    setDefault();
    return manufacturerDAO.getManufacturerList();
  }

  public: int updateManufacturer(ManufacturerModel manufacturerModel) {
    std::map<std::string, string> conditionMap;
    return updateManufacturer(manufacturerModel, conditionMap);
  }

  public: int updateManufacturer(ManufacturerModel manufacturerModel, std::map<std::string, string> conditionMap) {
    return manufacturerDAO.updateManufacturer(manufacturerModel, conditionMap);
  }

  private: void setDefault(){
    if (whereMap.size() != 0) {manufacturerDAO.whereMap = whereMap;}
    if (orderMap.size() != 0) {manufacturerDAO.orderMap = whereMap;}
    if (start != "") {manufacturerDAO.start = start;}
    if (length != "") {manufacturerDAO.length = length;}
  }
};