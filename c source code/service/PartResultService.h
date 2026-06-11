#include <map>
#include <../dao/PartResultDAO.h>

using namespace std;

class PartResultService: public V2Service {
  
  private:
    PartResultDAO partResultDAO;
  public:
  static json score_map;
  static json inspection_score_map;
  //return first record from the database
  public: map<string, string> getPartResultData() {
    std::map<int, map<string, string>> partResultList = getPartResultList();
    return partResultList[0];
  }

  public: map<int, map<string, string>> getPartResultList() {
    setDefault();
    return partResultDAO.getPartResultList();
  }

  public: map<int, map<string, string>> getPartResultList(string stock_info_id) {
    whereMap["partResult.stock_info_id"]=stock_info_id;
    length="300";
    setDefault();
    return partResultDAO.getPartResultList();
  }

  public: int updateBulkPartResult(std::map<std::string, string> keyValueMap, map<string, string>FieldIDmap) {
    int returnValue = 0;
    std::map<std::string, string>::iterator it = keyValueMap.begin();
    
    while (it != keyValueMap.end()) {
      std::map<std::string, string> partResult;
      partResult["param"] = it->first;
      partResult["result"] = it->second;
      partResult["score"] = get_score_for_parameter_inspection(it->first,it->second);
      if(partResult["score"] == ""){
        partResult["score"] = get_score_for_parameter(it->first,it->second);
      }
      if(partResult["score"] == ""){        
        if(partResult["result"] == "PASS" || partResult["result"] == "Working" || partResult["result"] == "Charging" || partResult["result"] == "Discharging"){
            partResult["score"]="0";
        }else if(partResult["result"] == "FAIL" || partResult["result"] == "Not Working" || partResult["result"] == "Not charging" || partResult["result"] == "Not Discharging" || partResult["result"]=="scrap"){
          partResult["score"]="41";
        }else if(partResult["result"] == "Not Available"){
          partResult["score"]="1";
        }else if(partResult["result"] == "Need to run Full BDT"){
          partResult["score"]="-1";
        }
      }
      partResult["stock_info_id"]=FieldIDmap["stock_info_id"];
      partResult["part_config_id"]=FieldIDmap["part_config_id"];
      partResult["part_id"]=FieldIDmap["part_id"];
      partResult["item_id"]=FieldIDmap["item_id"];
      partResult["created_by"]=FieldIDmap["created_by"];
      returnValue = updatePartResult(partResult);
      it++; 
    } 
    return returnValue;
  }

  public: string get_score_for_parameter(string parameter,string value){
    string score="";
    std::transform(parameter.begin(), parameter.end(), parameter.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    for(int i=0;i<score_map.size();i++){
      string param=score_map[i]["parameter"];
      string nj_value=score_map[i]["value"];
      std::transform(param.begin(), param.end(), param.begin(),
                   [](unsigned char c) { return std::tolower(c); });

      std::transform(nj_value.begin(), nj_value.end(), nj_value.begin(),
                   [](unsigned char c) { return std::tolower(c); });           
      if(param==parameter){
        if(score_map[i]["operater"]=="Equals to"){
          if(nj_value==value){
            score=score_map[i]["score"];
            return score;
          }
        }
      }
    }
    return score;
  }
  public: string get_score_for_parameter_inspection(string parameter,string value){
    string score="";
    std::transform(parameter.begin(), parameter.end(), parameter.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    for(int i=0;i<inspection_score_map.size();i++){
      string param=inspection_score_map[i]["parameter"];
      string nj_value=inspection_score_map[i]["value"];
      std::transform(param.begin(), param.end(), param.begin(),
                   [](unsigned char c) { return std::tolower(c); });

      std::transform(nj_value.begin(), nj_value.end(), nj_value.begin(),
                   [](unsigned char c) { return std::tolower(c); });           
      if(param==parameter){
        if(inspection_score_map[i]["operater"]=="Equals to"){
          if(nj_value==value){
            score=inspection_score_map[i]["score"];
            return score;
          }
        }
      }
    }
    return score;
  }

  public: int updatePartResult(std::map<std::string, string> fieldDataMap) {
    std::map<std::string, string> conditionMap;
    return updatePartResult(fieldDataMap, conditionMap);
  }

  public: int updatePartResult(std::map<std::string, string> fieldDataMap, std::map<std::string, string> conditionMap) {
    return partResultDAO.updatePartResult(fieldDataMap, conditionMap);
  }

  private: void setDefault(){
    if (whereMap.size() != 0) {partResultDAO.whereMap = whereMap;}
    if (orderMap.size() != 0) {partResultDAO.orderMap = orderMap;}
    if (start != "") {partResultDAO.start = start;}
    if (length != "") {partResultDAO.length = length;}
  }
};
json PartResultService:: score_map ;
json PartResultService:: inspection_score_map;