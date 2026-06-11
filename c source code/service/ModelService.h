#include <map>
#include <../dao/ModelDAO.h>

using namespace std;

class ModelService: public V2Service {
  
  private:
    ModelModel modelModel;
    ModelDAO modelDAO;


  //return first record from the database
  public: map<string, string> getModelById(string modelId) {
    whereMap["model_id"] = modelId;
    map<string, string> modelData = getModelData();
    return modelData;
  }
  //return first record from the database
  public: map<string, string> getModelData() {
    map<int, map<string, string>> modelDataList = getModelDataList();
    return modelDataList[0];
  }

  public: map<int, map<string, string>> getModelDataList() {
    setDefault();
    return modelDAO.getModelDataList();
  }

  public: int updateModel(ModelModel modelModel) {
    std::map<std::string, string> conditionMap;
    return updateModel(modelModel, conditionMap);
  }

  public: int updateModel(ModelModel modelModel, std::map<std::string, string> conditionMap) {
    return modelDAO.updateModel(modelModel, conditionMap);
  }

  private: void setDefault(){
    if (whereMap.size() != 0) {modelDAO.whereMap = whereMap;}
    if (orderMap.size() != 0) {modelDAO.orderMap = whereMap;}
    if (start != "") {modelDAO.start = start;}
    if (length != "") {modelDAO.length = length;}
  }
};