#include <map>
#include <freshweb/UtilTable.h>

class PurchaseReceiptView {
  public: string prView(map<int, map<string,string>> prItemList) {
    string idx;
    bool isIdxFound = false;
    TextTable t( '-', '|', '+' );
    
    t.add( "idx" );
    t.add( "Item Code" );
    t.add( "Item Name" );
    t.add( "Description" );
    t.endOfRow();

    for (int i = 1; i <= prItemList.size(); ++i) {
      t.add( prItemList[i]["idx"] );
      t.add( prItemList[i]["itemCode"] );
      t.add( prItemList[i]["itemName"] );
      t.add( prItemList[i]["description"] );
      t.endOfRow();
    }
    //t.setAlignment( 2, TextTable::Alignment::RIGHT );
    std::cout << t;
    while (! isIdxFound) {
      std::cout << "Enter the No. ";
      std::cin >> idx;
      //if (isdigit(stoi(idx))) {
      if (prItemList[stoi(idx)].size() > 0) {isIdxFound = true;}
      //}
      //std::cout << "entered" << "::" << idx;
      cin.clear();
    }
    return idx;
  }
};