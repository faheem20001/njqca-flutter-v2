#include <map>
#include <../model/ItemModel.h>
#include <../dao/ItemDAO.h>

using namespace std;

class ItemService: public V2Service {
  
  private:
    ItemModel itemModel;
    ItemDAO itemDAO;

  //return first record from the database
  public: map<string, string> getItemData() {
    std::map<int, map<string, string>> itemList = getItemList();
    return itemList[0];
  }

  public: map<int, map<string, string>> getItemList() {
    setDefault();
    return itemDAO.getItemList();
  }

  public:
	int updateItem(ItemModel itemModel) {
    std::map<std::string, string> conditionMap;
    return updateItem(itemModel, conditionMap);
  }

  public:
	int updateItem(ItemModel itemModel, std::map<std::string, string> conditionMap) {
    return itemDAO.updateItem(itemModel, conditionMap);
  }

  private: void setDefault(){
    if (whereMap.size() != 0) {itemDAO.whereMap = whereMap;}
    if (orderMap.size() != 0) {itemDAO.orderMap = orderMap;}
    if (start != "") {itemDAO.start = start;}
    if (length != "") {itemDAO.length = length;}
  }
};