#include <iostream>
#include <map>
#include <string>
//#include <../service/sync2ERP/IQCService.h>

using namespace std;

class AllSyncService  {
	private:
    Sync2ERPService sync2ERPService;
    IQCService iQCService;
    CommonView commonView;
    
  public: void allSync() {

    // sync2ERPService.rawData();
    map<int, map<string, string>> msgERP = iQCService.dataSync();
    list<string> columnList {"No","Item Group","Brand","Model","Make","BIOS Serial No","Created By","Status"};
    commonView.columnList = columnList;
    commonView.view(msgERP);

  }; 
};