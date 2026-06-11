#include <map>
#include <freshweb/UtilTable.h>

class CommonView {
  public: list<string> columnList;

  public: string view(map<int,string> viewMap) {
    string idx;
    bool isIdxFound = false;
    TextTable t( '-', '|', '+' );
    
    /*t.add( "idx" );
    t.add( "Item Code" );
    t.add( "Item Name" );
    t.add( "Description" );
    t.endOfRow();*/

    for (int i = 1; i <= viewMap.size(); ++i) {
      t.add(viewMap[i]);
      t.endOfRow();
    }
    
    std::cout << t;
    return idx;
  }

  public: string view(map<int, map<string, string>> viewMapList) {
    string idx;
    bool isIdxFound = false;
    TextTable t( '-', '|', '+' );

    for (auto column : columnList) {
      t.add( column );
    }
    t.endOfRow();
    
    /*t.add( "idx" );
    t.add( "Item Code" );
    t.add( "Item Name" );
    t.add( "Description" );
    t.endOfRow();*/
    
    
    for (auto viewMap : viewMapList) {
      int slNo = viewMap.first + 1;
    
      for(string fieldNameByRow : columnList) {
      //for (auto viewData : viewMap.second) {
        t.add(viewMap.second[fieldNameByRow]);
      }
      t.endOfRow();
    }
    std::cout << t;
    return idx;
  }
};