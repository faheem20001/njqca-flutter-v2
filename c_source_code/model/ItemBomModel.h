#include <iostream>
#include <string>

using namespace std;

class ItemBomModel {
  private: int itemBomId = 0;
  private: int itemId = 0;
  private: int itemPartId = 0;
  private: string isItemCode;
  private: string isRemovable;
  private: string itemBomStatus;
  private: string itemName;
  private: string itemPartName;


  public: int getItemBomId() {return itemBomId;}
  public: void setItemBomId(int itemBomIdLocal) {itemBomId = itemBomIdLocal;}

  public: int getItemId() {return itemId;}
  public: void setItemId(int itemIdLocal) {itemId = itemIdLocal;}

  public: int getItemPartId() {return itemPartId;}
  public: void setItemPartId(int itemPartIdLocal) {itemPartId = itemPartIdLocal;}

  public: string getIsItemCode() {return isItemCode;}
  public: void setIsItemCode(string isItemCodeLocal) {isItemCode = isItemCodeLocal;}

  public: string getIsRemovable() {return isRemovable;}
  public: void setIsRemovable(string isRemovableLocal) {isRemovable = isRemovableLocal;}

  public: string getItemName() {return itemName;}
  public: void setItemName(string itemNameLocal) {itemName = itemNameLocal;}

  public: string getItemBomStatus() {return itemBomStatus;}
  public: void setItemBomStatus(string itemBomStatusLocal) {itemBomStatus = itemBomStatusLocal;}

  public: string getItemPartStatus() {return itemPartName;}
  public: void setItemPartStatus(string itemPartNameLocal) {itemPartName = itemPartNameLocal;}
};