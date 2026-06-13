#include <map>
#include <../dao/MasterFieldDAO.h>

using namespace std;

class MasterFieldService: public V2Service {
  
  private:
    MasterFieldDAO masterFieldDAO;

  public: map<int, map<string, string>> getMasterFieldList(string classificationId) {
    whereMap["classification_id"] = classificationId; 
    setDefault();
    return getMasterFieldList();
  }

  public: map<int, map<string, string>> getMasterFieldList() {
    setDefault();
    return masterFieldDAO.getMasterFieldList();
  }

  private: void setDefault(){
    if (whereMap.size() != 0) {masterFieldDAO.whereMap = whereMap;}
    if (orderMap.size() != 0) {masterFieldDAO.orderMap = orderMap;}
    if (start != "") {masterFieldDAO.start = start;}
    if (length != "") {masterFieldDAO.length = length;}
  }
};