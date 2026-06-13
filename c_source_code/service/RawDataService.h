#include <map>
#include <../dao/RawDataDAO.h>

using namespace std;

class RawDataService {
  RawDataModel rawDataModel;
  RawDataDAO rawDataDAO;

  //converts the Map to rawdata Object
  public: int updateRawData(std::map<string, string> systemInfo) {
    rawDataModel.setManufacturerName(systemInfo["vendor"]);
		rawDataModel.setModelName(systemInfo["version"]);
		rawDataModel.setSerialNo(systemInfo["serialNo"]);
		rawDataModel.setJsonData(systemInfo["jsonData"]);
		return updateRawData(rawDataModel);
  }

  public: int updateRawData(RawDataModel rawDataModel) {
    std::map<std::string, string> conditionMap;
    return rawDataDAO.updateRawData(rawDataModel, conditionMap);
  }

  public: int updateRawData(RawDataModel rawDataModel, std::map<std::string, string> conditionMap) {
    return rawDataDAO.updateRawData(rawDataModel, conditionMap);
  }
};