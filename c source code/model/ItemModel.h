#include <iostream>
#include <string>

using namespace std;

class ItemModel {
  private: int itemId = 0;
  private: string itemName;
  private: string itemCode;
  private: string itemNjhatCode;
  private: int itemTypeId = 0;
  private: string itemStatus;

  public: int getItemId() {return itemId;}
  public: void setItemId(int itemIdLocal) {itemId = itemIdLocal;}

  public: string getItemName() {return itemName;}
  public: void setItemName(string itemNameLocal) {itemName = itemNameLocal;}

  public: string getItemCode() {return itemCode;}
  public: void setItemCode(string itemCodeLocal) {itemCode = itemCodeLocal;}

  public: string getItemNjhatCode() {return itemNjhatCode;}
  public: void setItemNjhatCode(string itemNjhatCodeLocal) {itemNjhatCode = itemNjhatCodeLocal;}

   public: int getItemTypeId() {return itemTypeId;}
  public: void setItemTypeId(int itemTypeIdLocal) {itemTypeId = itemTypeIdLocal;}

  public: string getItemStatus() {return itemStatus;}
  public: void setItemStatus(string itemStatusLocal) {itemStatus = itemStatusLocal;}
};