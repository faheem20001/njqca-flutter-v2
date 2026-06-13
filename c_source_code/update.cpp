
#include <iostream>
#include <fstream>
#include<thread>
#include <vector>
#include <string>
#include <sqlite3.h>
#include <curl/curl.h>
#include <chrono>
// Include Libs & config
#include <config/constant.h>
#include <nlohmann/json.hpp>
#include <freshweb/Util.h>
#include <freshweb/database.h>
#include <freshweb/log.h>
#include <freshweb/model/restAPIModel.h>
#include <freshweb/restAPI.h>
#include <freshweb/Encryption.h>

#include <../view/CommonView.h>
#include <../view/AuthenticationView.h>
#include <../view/PurchaseReceiptView.h>

//#include <../service/Sync2ERPService.h>
//#include <../service/SettingService.h>
#include <../service/V2Service.h>
// #include  "service/check/CompleteBatteryCheck.h"
#include <../service/SettingService.h>
#include <../service/UserService.h>
#include <../service/SystemInfoService.h>
#include <../service/PurchaseReceiptService.h>
#include <../service/AuthenticationService.h>
#include <../service/Sync2ERPService.h>
#include <../service/sync2ERP/AllSyncService.h>
#include <../service/HardwareInfoService.h>

// #include <../service/Sync4ERPService.h>
// #include "../service/check/CompleteBatteryCheck.h"
//#include <../service/PartTestService.h>
// #include <../service/sync2ERP/AllSyncService.h>

// #include "../service/NJsettinsService.h"
// #include <../service/check/BatteryCheck.h>
//#include  "../service/check/wifiCheck.h"
//#include "../service/check/LANchek.h"
// #include "../service/check/USBcheck.h"
//#include "../service/check/StorageCheck.h"

using namespace std;

int main(int argc, char* argv[]) {

   // cout<<"hello world..!"<<endl;

   Constant constant;
   Util util;
   Encryption encryption;
   // BatteryCheck batteryCheck;

   AuthenticationView authenticationView;
   CommonView commonView;

   UserService userService;
   // PurchaseReceiptService purchaseReceiptService;
   AuthenticationService authenticationService;
   SystemInfoService systemInfoService;
   HardwareInfoService hardwareInfoService;
   // Sync2ERPService sync2ERPService;
   // Sync4ERPService sync4ERPService;
   // AllSyncService allSyncService;
   // PartTestService partTestService;
   PartConfigService partConfigService;
   // PurchaseReceiptView purchaseReceiptView;
   SystemCheck systemcheck;
   SettingService Settingservice;
   IQCService IQCservice;
   LanCheck lancheck;
   USBcheck usbcheck;
   BatteryCheck batteryCheck;
   GetFromERPService getFromERPService;
   HDDSentinelCheck hddsentinelcheck;
   CompleteBatteryCheck completebatterytest;
   PerformanceCheck performance;
   HDDSentinelCheck hddsentinel;
   Password passwordcheck;
   NjsettingsService njsettings;
   Database db;
   // CPUFanSpeedCheck fanspeed;
   // BluetoothCheck bluetooth;   
   auto begin = std::chrono::high_resolution_clock::now();
   
   string usr;
   string pwd;
   string hostName;
   hostName = Settingservice.getSettingParam("NJQCA", "hostName");
   // cout<<"\n"<<hostName<<endl;
   bool DO_CBC=false;
   // /*Parse the input parameters*/
   string param;
   bool userReset = false;
   bool doTest = true;
   for (int i = 0; i < argc; ++i) {
      param = argv[i];
      if (param == "-u" || param == "-U") {userReset = true;}
      if (param == "-cbc" || param == "-CBC") {DO_CBC=true;}
      if (param == "-e" || param == "-E") {
         hostName = argv[i + 1];
      }
      if (param == "-r" || param == "-R") {
         doTest = false;
      }
      
   }
   // constant.apiBaseURL = hostName;

   /*Start - Login Autentication Need to move to another file*/
   string userName = "";
   map<string,string> authInputMap;

   map<string, string> userData;
   map<string, string> orderBy;

   // string copycommand="robocopy \"wbem\" \"C:\\Windows\\System32\\wbem\" /e /xc /xn /xo";


   // util.executeTerminal(copycommand);

   if(!userReset){
   userData=passwordcheck.readPasswordFromUSB();
   if(!userData.empty()){
      authInputMap["username"] = userData["username"];
      authInputMap["password"] = userData["password"];
   }
   } 


   string storedUserName = userName;
   while (userName == "") {
      if (userData.empty()) {authInputMap = authenticationView.authentication();}

      userName = authenticationService.authenticate(authInputMap["username"], authInputMap["password"]);
      if (userName == "-1") {
         string confirm;
         std::cout << "Not able to connect to the NJERP server. Are you sure that you want to login using default login Yes / No ? ";
         std::cin >> confirm;
         std::map<string, string> validConfirmInfo;
         validConfirmInfo["0", "Yes"]; validConfirmInfo["1", "yes"]; validConfirmInfo["2", "YES"]; validConfirmInfo["3", "y"]; validConfirmInfo["4", "Y"];
         if (! util.findMap(validConfirmInfo,confirm)) {
            std::cout << "\nConnect after you are able to connect to the NJERP server\n\n";
            exit(0);
         }
         userName = storedUserName; //Use the store username for processing the information
      }
      if (userName == "") {
         userData.clear();
         std::cout << "The username or password is incorrect. Please retry again..." << endl;
      }
   }


   // }
   usr=  authInputMap["username"];
   pwd=  authInputMap["password"];
   string userDepartment= getFromERPService.SetToken(authInputMap["username"],authInputMap["password"]);
   
   string windowsNJQCA_url ;
   NjsettingsService njset;
   json  njsetting_result=njset.getNjSettingsvalues();
   for (const auto& entry :  njsetting_result) {
      
         
      if (entry["parameter"]=="WindowsNJQCA_URL") {windowsNJQCA_url=entry["value"].get<string>();
         cout<<windowsNJQCA_url<<endl;
         break;
      }
      
   }
   windowsNJQCA_url=lancheck.urlEncode(windowsNJQCA_url);
   lancheck.downloadFile(windowsNJQCA_url,"WindowsNJQCA.exe");
   
   string update_command="Temp_Data/WindowsNJQCA.exe ";

   auto end = std::chrono::high_resolution_clock::now();
   auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);

   printf("\nTime measured: %.2f seconds.\n", elapsed.count() * 1e-9);
   exit(0);
};

// https://uat.newjaisa.biz/api/method/nj_features.nj_features.api.login.login?usr=postman@mail.com&pwd=3b 04 01 07 74 04 0b 03&doctype_url=njqca/NJQCA-Lenovo-ThinkPad X1 Carbon 6th-PF1ET55S-01