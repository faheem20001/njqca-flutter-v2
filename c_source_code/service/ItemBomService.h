#include <map>
#include <../dao/ItemBomDAO.h>

using namespace std;

class ItemBomService: public V2Service {
  
  private:
    ItemBomModel itembomModel;
    ItemBomDAO itemBomDAO;

  public: map<int, map<string, string>> getItemPartList(string itemId) {
    whereMap["item.item_id"] = itemId;
    std::map<int, map<string, string>> itemBomList = getItemBomList();
    return itemBomList;
  }

  //return first record from the database
  public: map<string, string> getItemBomData() {
    std::map<int, map<string, string>> itemBomList = getItemBomList();
    return itemBomList[0];
  }

  public: map<int, map<string, string>> getItemBomList() {
    setDefault();
    return itemBomDAO.getItemBomList();
  }

  public: int updateItemBom(ItemBomModel itemBomModel) {
    std::map<std::string, string> conditionMap;
    return updateItemBom(itemBomModel, conditionMap);
  }

  public: int updateItemBom(ItemBomModel itemBomModel, std::map<std::string, string> conditionMap) {
    return itemBomDAO.updateItemBom(itemBomModel, conditionMap);
  }

  private: void setDefault(){
    if (whereMap.size() != 0) {itemBomDAO.whereMap = whereMap;}
    if (orderMap.size() != 0) {itemBomDAO.orderMap = orderMap;}
    if (start != "") {itemBomDAO.start = start;}
    if (length != "") {itemBomDAO.length = length;}
  }
};