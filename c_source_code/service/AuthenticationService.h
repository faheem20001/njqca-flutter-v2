#include <map>
//#include <../service/GetFromERPService.h>

using namespace std;

class AuthenticationService: public V2Service {

  private: 
    Constant constant;
    DocType docType;
    GetFromERPService getFromERPservice;
  
  public: string authenticate(string userName, string password) {
    string response = getFromERPservice.authoriseERP(userName, password);
    //update in DB for future automatic login
    if (response != "") {
    }
    return response; //userame if login successfull
  };
};