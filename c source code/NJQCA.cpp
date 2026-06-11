
#include <iostream>
#include <fstream>
#include<thread>
#include <vector>
#include <string>
#include <sqlite3.h>
#include <curl/curl.h>
#include <chrono>
#include <cctype>
#include <windows.h>
#include <filesystem>
#include <cstdlib>
// Include Libs & config
#include <config/constant.h>
#include <nlohmann/json.hpp>
#include <freshweb/Util.h>
#include <freshweb/database.h>
#include <freshweb/log.h>
#include <freshweb/model/restAPIModel.h>
#include <freshweb/restAPI.h>
#include <freshweb/Encryption.h>
#include <freshweb/emit.h>        // Flutter event bridge

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
namespace fs = std::filesystem;
string trimString(string str) {
   size_t first = str.find_first_not_of(" \n\r\t");
   if (first == string::npos) {
      return "";
   }

   size_t last = str.find_last_not_of(" \n\r\t");
   return str.substr(first, last - first + 1);
}

int getCurrentBatteryChargePercentage(Util& util) {
   string command =
      "powershell -NoProfile -Command \""
      "(Get-CimInstance -ClassName Win32_Battery | "
      "Select-Object -First 1 -ExpandProperty EstimatedChargeRemaining)"
      "\"";

   string result = util.executeTerminal(command);
   result = trimString(result);

   cout << " Raw battery percentage response: " << result << endl;

   if (result.empty()) {
      return -1;
   }

   string number = "";

   for (char ch : result) {
      if (isdigit(static_cast<unsigned char>(ch))) {
         number += ch;
      } else if (!number.empty()) {
         break;
      }
   }

   if (number.empty()) {
      return -1;
   }

   return stoi(number);
}
const std::string NJQCA_TEMP_DIR = "C:\\WindowsNJQCA\\Temp_Data";
const std::string NJQCA_SLEEP_BACKUP_FILE = NJQCA_TEMP_DIR + "\\njqca_sleep_backup.csv";

void runPowerShellScript(const std::string& script, const std::string& scriptPath) {
   fs::create_directories(NJQCA_TEMP_DIR);

   std::ofstream file(scriptPath);
   file << script;
   file.close();

   std::string command =
      "powershell -NoProfile -ExecutionPolicy Bypass -File \"" + scriptPath + "\"";

   system(command.c_str());
}

void backupSleepSettingsOnce() {
   fs::create_directories(NJQCA_TEMP_DIR);

   // Important: after restart, do not overwrite original sleep setting.
   if (fs::exists(NJQCA_SLEEP_BACKUP_FILE)) {
      cout << " Sleep setting backup already exists. Not overwriting." << endl;
      return;
   }

   std::string script = R"PS(
$ErrorActionPreference = "Stop"

$tempDir = "C:\WindowsNJQCA\Temp_Data"
$backupPath = "C:\WindowsNJQCA\Temp_Data\njqca_sleep_backup.csv"

New-Item -ItemType Directory -Force -Path $tempDir | Out-Null

$query = powercfg /query SCHEME_CURRENT SUB_SLEEP STANDBYIDLE

$acLine = $query | Select-String "Current AC Power Setting Index:" | Select-Object -First 1
$dcLine = $query | Select-String "Current DC Power Setting Index:" | Select-Object -First 1

if (-not $acLine -or -not $dcLine) {
   throw "Unable to read current Windows sleep settings."
}

$acHex = ($acLine.ToString().Split(":")[-1]).Trim()
$dcHex = ($dcLine.ToString().Split(":")[-1]).Trim()

$acValue = [Convert]::ToInt32($acHex.Replace("0x", ""), 16)
$dcValue = [Convert]::ToInt32($dcHex.Replace("0x", ""), 16)

"$acValue,$dcValue" | Set-Content -Encoding ASCII -Path $backupPath
)PS";

   runPowerShellScript(script, NJQCA_TEMP_DIR + "\\backup_sleep_settings.ps1");

   cout << " Original Windows sleep settings backed up." << endl;
}

void enableNJQCANoSleepMode() {
   // cout << "\n Enabling NJQCA No-Sleep Mode..." << endl;

   backupSleepSettingsOnce();

   // Prevent system sleep while current NJQCA process is running.
   SetThreadExecutionState(
      ES_CONTINUOUS |
      ES_SYSTEM_REQUIRED
   );

   // Disable only actual Windows sleep.
   system("powercfg /change standby-timeout-ac 0");
   system("powercfg /change standby-timeout-dc 0");
   system("powercfg /setactive SCHEME_CURRENT");

   cout << " NJQCA No-Sleep Mode enabled." << endl;
}

void restoreNJQCASleepSettings() {
   // cout << "\n Restoring Windows sleep settings..." << endl;

   SetThreadExecutionState(ES_CONTINUOUS);

   if (!fs::exists(NJQCA_SLEEP_BACKUP_FILE)) {
      // cout << " No sleep backup found. Restore skipped." << endl;
      return;
   }

   std::string script = R"PS(
$backupPath = "C:\WindowsNJQCA\Temp_Data\njqca_sleep_backup.csv"

if (Test-Path $backupPath) {
   $values = (Get-Content $backupPath -Raw).Trim().Split(",")

   if ($values.Count -ge 2) {
      $acValue = [int]$values[0]
      $dcValue = [int]$values[1]

      powercfg /setacvalueindex SCHEME_CURRENT SUB_SLEEP STANDBYIDLE $acValue | Out-Null
      powercfg /setdcvalueindex SCHEME_CURRENT SUB_SLEEP STANDBYIDLE $dcValue | Out-Null
      powercfg /setactive SCHEME_CURRENT | Out-Null
   }

   try {
      Remove-Item $backupPath -Force -ErrorAction Stop
   } catch {
      Write-Output " Could not delete sleep backup file. Close it if it is opened."
   }
}
)PS";

   runPowerShellScript(script, NJQCA_TEMP_DIR + "\\restore_sleep_settings.ps1");

   // cout << " Windows sleep settings restored." << endl;
}
int main(int argc, char* argv[]) {

   // cout<<"hello world..!"<<endl;

   Constant constant;
   Util util;
   restoreNJQCASleepSettings();
   std::atexit(restoreNJQCASleepSettings);
   enableNJQCANoSleepMode();
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
   // //njsettingsService njset;
   WifiCheck wificheck;
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
   AdapterCheck adaptertest;
   DriverCheck driver;
   HDMI_VGA_PortCheck port_check;
   RAMspeedCheck ramcheck;
   TypeC_chargerCheck typec_charger_check;
   SpeakerTest speakertest;
   VibrationCheck vibrationcheck;
   PartResultService partresultservice;
   GeekBenchCheck geekbenchcheck;
   Database db;
   LidCheck lidcheck;
   // CPUFanSpeedCheck fanspeed;
   // BluetoothCheck bluetooth;   
   auto begin = std::chrono::high_resolution_clock::now();
   db.deleteData("DELETE FROM nj_stock_info;");
   db.deleteData("DELETE FROM nj_raw_data;");
   db.deleteData("DELETE FROM nj_part_config;");
   db.deleteData("DELETE FROM nj_part_result;");

// std::string Script ="fnlockinps.exe";
//   system(Script.c_str());
// exit(0);
   /*Check for root user*/
   // if (getuid()) {
   //    string rootPassword;
   //    printf("%s", "You are not root. Use sudo -s to login as root\n");
   //    exit(0);
   //    //std::cout << "Enter the root passsword:";
   //    //std::cin >> rootPassword;
   //    //system("sudo");
   // }

   //Get ERP host URL
   // string LAN_connection_status=util.executeTerminal("nmcli dev status|grep 'ethernet' | grep 'connected'");
   // string wifi_connection_status=util.executeTerminal("nmcli dev status|grep 'wifi' | grep 'connected'");

   // if(LAN_connection_status.empty() && wifi_connection_status.empty()){
   //    system("echo  '\\e[31merror: Please Connect to Network\\e[0m'");
   //    exit(0);
   // }else{
   //    string Network_status=util.executeTerminal("timeout 5 ping google.com");
   //    if(Network_status.find("failure")==std::string::npos){
   //       system("echo  '\\e[31merror: Please Check Your Network Connection...\\e[0m'");
   //       exit(0);
   //    }
   string usr;
   string pwd;
   string hostName;
   std::thread VibrationThread;
   hostName = Settingservice.getSettingParam("NJQCA", "hostName");
   // cout<<"\n"<<hostName<<endl;
   bool DO_CBC=false;
   bool DO_adapter=false;
   bool DO_ram=false;
   bool Do_sync=false;
   // /*Parse the input parameters*/
   string param;
   bool userReset = false;
   bool doTest = true;
   for (int i = 0; i < argc; ++i) {
      param = argv[i];
      if (param == "-u" || param == "-U") {userReset = true;}
      if (param == "-cbc" || param == "-CBC") {DO_CBC=true;}
      if (param == "-lac" || param == "-LAC") {DO_adapter=true;}
      if (param == "-lrc" || param == "-LRC") {DO_ram=true;}
      if (param == "-sync" || param == "-SYNC") {Do_sync=true;DO_CBC=true;}

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

      // 1. Try Flutter-provided credentials first
      ifstream fa("Temp_Data\\flutter_auth.txt");
      if(fa.good()){
         string fu, fp;
         getline(fa, fu);
         getline(fa, fp);
         fa.close();
         remove("Temp_Data\\flutter_auth.txt");
         if(!fu.empty() && !fp.empty()){
            userData["username"] = fu;
            userData["password"] = fp;
         }
      }

      // 2. USB scan only if Flutter didn't provide credentials
      if(userData.empty()){
         userData=passwordcheck.readPasswordFromUSB();
      }

      if(!userData.empty()){
         authInputMap["username"] = userData["username"];
         authInputMap["password"] = userData["password"];
      }
   } 

   // if(userData.empty()){
   // orderBy["modified_on"] = "desc";
   // userService.orderMap = orderBy;
   // userService.length = "1";
   // userData = userService.getUserData();
   // if (! userData.empty() && ! userReset) {
   //    authInputMap["username"] = userData["user_name"];
   //    authInputMap["password"] = encryption.decrypt(userData["password"],constant.ENCRYPTION_KEY);
   //    std::map <int, string> messageMap;
   //    messageMap[1] = "Auto login as " + userData["user_name"] + " in " + constant.apiBaseURL;
   //    messageMap[2] = "To do force login as different user enter as ./njhat -u";
   //    commonView.view(messageMap);
   // }
   // }

   //Login Authentication
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
  
   usr=  authInputMap["username"];
   // cout << "[DEBUG] usr = '" << usr << "'" << endl;
   pwd=  authInputMap["password"];
   string userDepartment= getFromERPService.SetToken(authInputMap["username"],authInputMap["password"]);
   // Write department to file for PowerShell scripts to read
   string deptFilePath = "Temp_Data\\department.txt";
   // Delete existing file if present
   if (fs::exists(deptFilePath)) {
      fs::remove(deptFilePath);
   }
   std::ofstream deptFile(deptFilePath);
   if (deptFile.is_open()) {
      deptFile << userDepartment;
      deptFile.close();
      cout << "Department written to file: " << userDepartment << endl;
   } else {
      cout << "Warning: Could not write department.txt" << endl;
   }
   float version=8.8;
   float NJSettings_Version;
   NjsettingsService njset;
   // cout<<userDepartment<<endl;
   json  njsetting_result=njset.getNjSettingsvalues();
   // cout<<"223"<<endl;
   // cout<<njsetting_result.dump(2)<<endl;
   for (const auto& entry :  njsetting_result) {
      
         
      if (entry["parameter"]=="WindowsNJQCA") {NJSettings_Version=stof(entry["value"].get<string>());
      
         break;
      }
      
   }
   // cout<<"version"<<version<<endl;
   // cout<<"NJsettings "<<NJSettings_Version<<endl;
   string updatecommand="powershell.exe -Command \".\\update.exe\"";
   if(NJSettings_Version){
   if(version<NJSettings_Version){
      cout<<"current version is "<<version<<" Nj settings version is "<<NJSettings_Version<<endl;
      // string update=util.executeTerminal("zenity --question --text=\"Update Available for NJQCA. Do you want to update now?\" --ok-label=\"Yes\" --cancel-label=\"No\" --width=300 --height=200 && echo \"1\" || echo \"0\"");
      
      
      int returncode=system(updatecommand.c_str());
      if(returncode==0){
         // version=NJSettings_Version;
         cout<<"\033[1;33m\nNJQCA is updated successfully to version "<<NJSettings_Version<<"\033[0m"<<endl;
         cout<<"\033[1;33m\nPlease re-run the Application\033[0m"<<endl;
      }      

      exit(0);
      
   }
}
   json input_user_data_inspection_types;
   string inspection_type="";
   vector<string> inspection_type_list;
   map<string,string> requestMap;
   input_user_data_inspection_types["user"]=usr;
   std::string role_inspection_types = IQCservice.getParameterList(constant.API_role_inspection_types, "post", requestMap, to_string(input_user_data_inspection_types));
   try{
      json role_inspection_types_json=json::parse(role_inspection_types);
      if(role_inspection_types_json["message"]!=""){
        inspection_type_list =role_inspection_types_json["message"];
      }
   }catch(const exception& e){
      cout<<"Error while getting the inspection type"<<e.what()<<role_inspection_types<<endl;
   }
   
   if (inspection_type_list.size()==0){
      util.generatePopupredok("No Active Inspection Type Mapped to user role","Please contact your TL/ML");
      exit(0);
   }
   bool can_reuse_saved_inspection_type = IQCservice.canReuseSavedInspectionType(usr);

string saved_inspection_type = "";
bool saved_type_valid = false;

if (can_reuse_saved_inspection_type) {
   saved_inspection_type = IQCservice.readInspectionTypeFromFile(usr);

   if (!saved_inspection_type.empty()) {
      for (const auto& type : inspection_type_list) {
         if (type == saved_inspection_type) {
            saved_type_valid = true;
            break;
         }
      }
   }
} else {
   IQCservice.deleteInspectionTypeFile();
}
// ── Read Flutter-provided inspection type ─────────────────────
{
    ifstream fi("Temp_Data\\flutter_inspection_type.txt");
    if (fi.good()) {
        string ft;
        getline(fi, ft);
        fi.close();
        remove("Temp_Data\\flutter_inspection_type.txt");
        for (const auto& t : inspection_type_list) {
            if (t == ft) {
                inspection_type = ft;
                IQCservice.saveInspectionTypeToFile(inspection_type, usr);
                break;
            }
        }
    }
}
// ──────────────────────────────────────────────────────────────
if (inspection_type.empty()) {
if (inspection_type_list.size() > 1) {
   if (saved_type_valid) {
      inspection_type = saved_inspection_type;
      cout << "[INFO] Reusing saved inspection type: " << inspection_type << endl;
   } else {
      inspection_type = util.selected_option(inspection_type_list);
      while (inspection_type.empty()) {
         inspection_type = util.selected_option(inspection_type_list);
      }
      IQCservice.saveInspectionTypeToFile(inspection_type, usr);
   }
} else {
   inspection_type = inspection_type_list[0];
   IQCservice.saveInspectionTypeToFile(inspection_type, usr);
}
} // end if (inspection_type.empty())

   IQCservice.selected_inspection_type = inspection_type;
   int required_system_min_charge_percentage = 0;

   json charge_input;
   charge_input["inspection_type"] = inspection_type;

   map<string, string> chargeRequestMap;

   std::string charge_config_response = IQCservice.getParameterList(
      constant.API_system_min_charge_percentage,
      "post",
      chargeRequestMap,
      to_string(charge_input)
   );

   cout << "[DEBUG] charge_config_response: " << charge_config_response << endl;

   try {
      json charge_config_json = json::parse(charge_config_response);

      if (
         charge_config_json.contains("message") &&
         charge_config_json["message"].contains("system_min_charge_percentage")
      ) {
         required_system_min_charge_percentage =
            charge_config_json["message"]["system_min_charge_percentage"].get<int>();
      }
   } catch (const exception& e) {
      cout << "Error while getting system_min_charge_percentage: "
           << e.what() << " Response: " << charge_config_response << endl;

      required_system_min_charge_percentage = 0;
   }

   cout << " Selected Inspection Type: " << inspection_type << endl;
   cout << " Required system minimum charge percentage: "
        << required_system_min_charge_percentage << "%" << endl;

   // If ERP returns 0, charge restriction is disabled for this inspection type.
   // If ERP returns > 0, check current system charge before continuing.
   if (required_system_min_charge_percentage > 0) {
      int current_charge_percentage = getCurrentBatteryChargePercentage(util);

      cout << " Current battery charge percentage: "
           << current_charge_percentage << "%" << endl;

      if (current_charge_percentage < 0) {
         util.generatePopupredok(
            "Battery Charge Not Found",
            "Unable to read current battery charge percentage. Please check battery and try again."
         );
         exit(0);
      }

      if (current_charge_percentage < required_system_min_charge_percentage) {
         // cout << "[DEBUG] Battery restriction failed. Showing warning popup and stopping script." << endl;

         string warning_msg =
            "Selected Inspection Type: " + inspection_type +
            ". Current battery charge is " + to_string(current_charge_percentage) +
            "%. Required minimum charge is " + to_string(required_system_min_charge_percentage) +
            "%. Please connect charger and charge the system before running NJQCA.";

         // cout << "[DEBUG] Warning message: " << warning_msg << endl;

         util.generatePopupredok(
            "Battery Charge Too Low",
            warning_msg
         );

         exit(0);
      }
   }
   vector<string> selected_checks;
   json selected_checks_input;
   selected_checks_input["inspection_type"] = inspection_type;
   std::string selected_checks_result = IQCservice.getParameterList(
       constant.API_selected_checks, "post", requestMap, to_string(selected_checks_input)
   );
   if(selected_checks_result.empty()){
       cout << "WARNING: Failed to get selected checks from ERP - all hardware checks will be skipped" << endl;
   } else {
       try {
           json selected_checks_json = json::parse(selected_checks_result);
           if (selected_checks_json.contains("message") && selected_checks_json["message"].is_array()) {
               selected_checks = selected_checks_json["message"].get<vector<string>>();
           }
       } catch (const exception& e) {
           cout << "Error while getting selected checks: " << e.what() << endl;
       }
   }
   json empty_input;
   empty_input["input"] = "1";
   std::string score_map_result = IQCservice.getParameterList(constant.score_mapping, "post", requestMap, to_string(empty_input));
   while(score_map_result.empty()){
      util.generatePopupredok("Score Mapping not found","Please check your internet connection?");
      Sleep(2000);
      score_map_result = IQCservice.getParameterList(constant.score_mapping, "post", requestMap, to_string(empty_input));
   }
   json score_map;
   try{
      json score_map_json=json::parse(score_map_result);
      if(score_map_json["message"]!=""){
        score_map=score_map_json["message"];
      }
   }catch(const exception& e){
      cout<<"Error while getting the score map"<<e.what()<<score_map_result<<endl;
   }
   // cout<<"score_map"<<score_map.dump(4)<<endl;
   partresultservice.score_map=score_map;

   json inspection_type_input;
   inspection_type_input["inspection_type"]=inspection_type;
   std::string inspection_score_map_result = IQCservice.getParameterList(constant.inspection_score_mapping, "post", requestMap, to_string(inspection_type_input));
   while(inspection_score_map_result.empty()){
      util.generatePopupredok("Score Mapping not found for selected inspection type","Please check your internet connection?");
      Sleep(2000);
      inspection_score_map_result = IQCservice.getParameterList(constant.inspection_score_mapping, "post", requestMap, to_string(inspection_type_input));
   }
   json inspection_score_map;
   try{
      json score_map_json=json::parse(inspection_score_map_result);
      if(score_map_json["message"]!=""){
        inspection_score_map=score_map_json["message"];
      }
   }catch(const exception& e){
      cout<<"Error while getting the score map"<<e.what()<<inspection_score_map_result<<endl;
   }
   
   partresultservice.inspection_score_map=inspection_score_map;
   json user_data;
   user_data["user"]=usr;
   user_data["selected_inspection_type"] = inspection_type;
   std::string inspection_item_group_data = IQCservice.getParameterList(constant.API_component_item_group, "post", requestMap, to_string(user_data));
   cout<<"inspection_item_group:"<<inspection_item_group_data<<endl;
   string item_group="Laptops";
   try{
      json inspection_item_group_json=json::parse(inspection_item_group_data);
      if(inspection_item_group_json["message"]!=""){
         item_group=inspection_item_group_json["message"].get<string>();
      }
   }catch(const exception& e){
      cout<<"Error while getting the item group"<<e.what()<<inspection_item_group_data<<endl;
   }
   IQCservice.item_group=item_group;
   if(item_group!="Laptops"){
      hddsentinel.is_component=true;
   }
   string Bios_serial_no;
   string model;
   const string CMD_SYSTEM= "powershell -Command \"Get-WmiObject -Class Win32_computersystemproduct\"";
   json jsonObject  = util.executeTerminalJSON(CMD_SYSTEM,":");
   // cout<<jsonObject.dump(2)<<endl;
   Bios_serial_no=jsonObject["IdentifyingNumber"];
   
   json part_list_input;
   vector<string> part_list_item_groups;
   part_list_input["bios_serial_no"]=Bios_serial_no;
   std::string part_list_result = IQCservice.getParameterList(constant.API_part_list, "post", requestMap, to_string(part_list_input));
   try{
      json part_list_json=json::parse(part_list_result);
      if(part_list_json["message"]!=""){
         part_list_item_groups = part_list_json["message"];
      }
   }catch(const exception& e){
      cout<<"Error while getting the part list"<<e.what()<<part_list_result<<endl;
   }
    
   hardwareInfoService.part_list_item_groups=part_list_item_groups;

   json jsonData;
   // map<string, string> requestMap;
   jsonData ["user"]=usr;
   jsonData ["bios_serial_no"]=Bios_serial_no;
   jsonData["selected_inspection_type"] = inspection_type;
// cout<<loggedUser<<endl;
   std::string paramter_response = IQCservice.getParameterList(constant.API_PARAMETER_URL, "post", requestMap, to_string(jsonData));
   cout<<"Parameters To RUN the Test:"<< paramter_response <<endl;
// if (item_group=="Laptops"){
   std::thread t;
   std::string vibration_result;

   json BIOS_SERIAL_NO;
   json data;
   BIOS_SERIAL_NO ["bios_serial_no"]=Bios_serial_no;
   std::string previous_test_data = IQCservice.getParameterList(constant.API_GET_PREVIOUS_TEST_DATA_URL, "post", requestMap, to_string(BIOS_SERIAL_NO));
   if(!previous_test_data.empty()){
   try{
   data =json::parse(previous_test_data);
   data=data["message"];
   cout<<"cascading results:"<<data.dump(2)<<endl;
   if(data.contains("error")){
      data.clear();
   }
   
   }catch(exception &e){
      cout<<"ERROR while getting the cascading data:"<<e.what()<<previous_test_data<<endl;
   }
   }

   batteryCheck.updateDepartmentTime(userDepartment,njsetting_result);
   systemcheck.updateNJwallpaperFilename(userDepartment,njsetting_result);
   driver.update_exclude_drivers(userDepartment,njsetting_result);
   StockInfoService stockinfoservice;
   stockinfoservice.DO_CBC=DO_CBC;
   stockinfoservice.DO_Adapter=DO_adapter;
   stockinfoservice.DO_RAM=DO_ram;

   // completebatterytest.updateCompleteBatteryCheckTime(userDepartment);
   wificheck.update_WIFI_file(userDepartment,njsetting_result);
   completebatterytest.updateCompleteBatteryCheckTime(userDepartment,njsetting_result);
   // performance.update_performance_fileURL(userDepartment,njsetting_result);
   hddsentinel.update_HDD_passingHealth(userDepartment,njsetting_result);

   PartTestService parttestservice;
   parttestservice.selected_inspection_type=inspection_type;
   parttestservice.loggedUser=usr;
   lancheck.update_LAN_fiile(userDepartment,njsetting_result);
   IQCservice.njqca_version=version;
   IQCservice.loggedUser = usr;

   if(paramter_response.find("Performance_status")!=std::string::npos){
      IQCservice.isNJQCAStored=true;

   }

   int stockInfoId =0 ;
   if (doTest) {
      stockInfoId = systemInfoService.getSystemInfo(userName);
      // cout<<"\nstockInfoId is"<<stockInfoId<<endl;
     
      IQCservice.current_stock_info_id=stockInfoId;

   }
   //    // Get Part configuration information only if manufacturer and Model is available in the database.
      if (stockInfoId != 0) {
         hardwareInfoService.do_cbc=DO_CBC;
         hardwareInfoService.do_adapter=DO_adapter;
         hardwareInfoService.do_ram=DO_ram;
         hardwareInfoService.selected_checks = selected_checks;
         hardwareInfoService.getConfig(stockInfoId);
      // }
   }

   // paramter_response+="Performance_status,heavy_load_status";
   if(paramter_response.find("Performance_status") != std::string::npos){

      bool is_reliability_test = (inspection_type == "Reliability Test");

      if (is_reliability_test) {
         // For Reliability Test, Performance is mandatory.
         // Do not reuse previous PASS / QI Reference.
         performance.run_test = false;
         performance.qi_name = "";

         cout << "[INFO] Reliability Test selected: Performance will run mandatory. Previous Performance result ignored." << endl;
      } else {
         for(int i=0;i<data.size();i++){
            if(data[i]["item_group"]=="LAPTOP RAM"){
               if(data[i]["parameter"]=="Performance_status"){
                  try{
                     float score=data[i]["score"].get<float>();
                     if(score==0){
                        performance.run_test=true;
                     }
                  }catch(exception &e){
                     cout<<"ERROR while getting the performance cascading data:"<<e.what()<<endl;
                  }
               }
               if(data[i]["qi_reference"]!=""){
                  performance.qi_name=data[i]["qi_reference"];
               }
            }
         }
      }
      // cout<<"performance.run_test "<<performance.run_test<<endl;
      if(!performance.run_test){
         if(!hddsentinelcheck.file_exist("Temp_Data/restart_times.txt")){   
            system("runperformance.exe");
            // cout<<"running performance test"<<endl;
         }
      }
   }
   if(paramter_response.find("Vibration_test_status")!=std::string::npos){
      for(int i=0;i<data.size();i++){
         if(data[i]["item_group"]=="MOTHERBOARD"){
            if(data[i]["parameter"]=="Vibration_test_status"){
               try{
                  float score=data[i]["score"].get<float>();
                  if(score==0){
                     vibrationcheck.runTest=true;
                  }
               }catch(exception &e){
                  cout<<"ERROR while getting the vibration cascading data:"<<e.what()<<endl;
               }
            }
            if(data[i]["qi_reference"]!=""){
               vibrationcheck.qi_reference=data[i]["qi_reference"];
            }
         }
      }
      if(!vibrationcheck.runTest){
        string driver_command_1="pnputil /add-driver \"C:\\WindowsNJQCA\\lENOVO FTDI-FORCED-10x64-2.12.36.20-drp\\*.inf\" /subdirs /install"; 
      //   string driver_command_2="pnputil /add-driver \"C:\\WindowsNJQCA\\Serial\\*.inf\" /subdirs /install"; 
        string test_command="ardunio.exe" ;  
        system(driver_command_1.c_str());
      //   system(driver_command_2.c_str());
      //   exit(0);       
        cout<<"\nVibration Test checking..." << endl;
        t = std::thread([&]() {
            vibration_result = util.executeTerminal(test_command);
        });
      }
      
      
   }

   if(paramter_response.find("heavy_load_status") != string::npos){

      bool is_reliability_test = (inspection_type == "Reliability Test");

      if (is_reliability_test) {
         // For Reliability Test, HeavyLoad is mandatory.
         // Do not reuse previous PASS / QI Reference.
         hddsentinelcheck.run_test = false;
         hddsentinelcheck.qi_name = "";

         cout << "[INFO] Reliability Test selected: HeavyLoad will run mandatory. Previous HeavyLoad result ignored." << endl;
      } else {
         for(int i=0;i<data.size();i++){
            if(data[i]["item_group"]=="LAPTOP STORAGE"){
               if(data[i]["parameter"]=="heavy_load_status"){
                  try{
                     float score=data[i]["score"].get<float>();
                     if(score==0){
                        hddsentinelcheck.run_test=true;
                     }
                  }catch(exception &e){
                     cout<<"ERROR while getting the hddsentinelcheck cascading data:"<<e.what()<<endl;
                  }
               }
               if(data[i]["qi_reference"]!=""){
                  hddsentinelcheck.qi_name=data[i]["qi_reference"];
               }
            }
         }
         // // cout<<"hddsentinelcheck.run_test "<<hddsentinelcheck.run_test<<endl;
         // if(!hddsentinelcheck.run_test){
         // string heavy_load_command="run_heavy_load.exe";
         // cout<<"\nHeavy Load Test checking..." << endl;
         // string heavy_load_result=util.executeTerminal(heavy_load_command);
         // hddsentinelcheck.heavy_load_result=heavy_load_result;
         // }
      }
   }
   // y_cruncher_status parameter
if(paramter_response.find("y_cruncher_status") != string::npos){

      bool is_reliability_test = (inspection_type == "Reliability Test");

      if (is_reliability_test) {
         // For Reliability Test, Y-Cruncher is mandatory.
         // Do not reuse previous PASS / QI Reference.
         hddsentinelcheck.run_y_cruncher_test = false;
         hddsentinelcheck.y_cruncher_qi_name = "";

         cout << "[INFO] Reliability Test selected: Y-Cruncher will run mandatory. Previous Y-Cruncher result ignored." << endl;
      } else {
         for(int i = 0; i < data.size(); i++){
            if(data[i]["item_group"] == "LAPTOP RAM"){
                  if(data[i]["parameter"] == "y_cruncher_status"){
                     try{
                        float score = data[i]["score"].get<float>();
                        if(score == 0){
                              hddsentinelcheck.run_y_cruncher_test = true;
                        }
                     }catch(exception &e){
                        cout << "ERROR while getting the y_cruncher cascading data:" << e.what() << endl;
                     }
                  }
                  if(data[i]["qi_reference"] != ""){
                     hddsentinelcheck.y_cruncher_qi_name = data[i]["qi_reference"];
                  }
            }
         }
      }
}
   if (t.joinable()){ t.join(); }
   if(paramter_response.find("Vibration_test_status")!=std::string::npos){
     cout<<"vibration_result "<<vibration_result<<endl;
     vibrationcheck.vibration_results=vibration_result;
   }

// else if (item_group=="BATTERY"){
//    util.executeTerminal("zadig-2.9.exe");
// }

   string existingNJQCA = IQCservice.readNJQCANameFromFile(usr);
   bool hasPerformance = paramter_response.find("Performance_status")!=std::string::npos;
   bool restartExists = hddsentinelcheck.file_exist("Temp_Data/restart_times.txt");

   if(existingNJQCA != "" && !DO_CBC && !DO_adapter && !DO_ram ){
      if((hasPerformance && restartExists) || (!hasPerformance) ){
         parttestservice.StartAllChecks(to_string(stockInfoId));
         // Sleep(30000);
         // parttestservice.startNJPULSE_Test();
      }
   }

   map<int,map<string,string>>partresult= partConfigService.getPartConfigList(to_string(stockInfoId));
   if(DO_CBC && !Do_sync){

      string screen_off_ac="powercfg /change monitor-timeout-ac 0";
      string screen_off_dc="powercfg /change monitor-timeout-dc 0";
      string sleep_off_ac="powercfg /change standby-timeout-ac 0";
      string sleep_off_dc="powercfg /change standby-timeout-dc 0";
      string hybernate_off_ac="powercfg /change hibernate-timeout-ac 0";
      string hybernate_off_dc="powercfg /change hibernate-timeout-dc 0";
      string default_settings="powercfg -restoredefaultschemes";
      string turn_off_auto_time_sync="powershell -Command \"Set-ItemProperty -Path \"HKLM:\\SYSTEM\\CurrentControlSet\\Services\\W32Time\\Parameters\" -Name \"Type\" -Value \"NoSync\"\"";
      string turn_on_auto_time_sync="powershell -Command \"Set-ItemProperty -Path \"HKLM:\\SYSTEM\\CurrentControlSet\\Services\\W32Time\\Parameters\" -Name \"Type\" -Value \"NTP\"\"";


      system(screen_off_ac.c_str());
      system(screen_off_dc.c_str());
      system(sleep_off_ac.c_str());
      system(sleep_off_dc.c_str());
      system(hybernate_off_ac.c_str());
      system(hybernate_off_dc.c_str());

      util.executeTerminal(turn_off_auto_time_sync);
      completebatterytest.completebatterytest(partresult,1);

      map<int, map<string,string>>battrey_info=hardwareInfoService.post_getBatteryInfo();
      if(battrey_info.size()>0){
         json battery_data=battrey_info[0];
         // cout<<battery_data<<endl;
         if (battery_data.contains("Cyclecount")&& battery_data.contains("Health") && battery_data.contains("Fullchargecapacity") && battery_data.contains("DesignCapacity")) {
            int full_charge_capacity=stoi(battery_data["Fullchargecapacity"].get<string>())/1000;
            int design_capacity=stoi(battery_data["DesignCapacity"].get<string>())/1000;
            json post_bdt_info;
            post_bdt_info["cycle_count"]=battery_data["Cyclecount"];
            post_bdt_info["design_capacity"]=to_string(design_capacity) +" WH";
            post_bdt_info["full_charge_capacity"]=to_string(design_capacity) +" WH";
            post_bdt_info["health"]=battery_data["Health"];
            post_bdt_info["njqca_name"]=IQCservice.config_updated_name; 
            // cout<<"post_bdt_info"<<post_bdt_info<<endl;   
            map<string,string>requestmapempty;
            string update_response=IQCservice.getParameterList(constant.API_update_post_bdt_info,"PUT",requestmapempty,to_string(post_bdt_info));
            cout<<"update response"<<update_response<<endl;
         }else{
            cout<<"Battery information is not available"<<endl;
         }
      }
   }

   // if(Do_sync){
   //    // completebatterytest.offline_data_sync();
   // }

   // if(DO_adapter){
   //    adaptertest.AdapterTest(to_string(stockInfoId),"24","24");
   // }

   // if(DO_ram){
   //    // ramcheck.RAMspeedTest(partresult);
   // }
   // completebatterytest.get_ntp_time();
   //Push all the data to ERP
   // allSyncService.allSync();
   // cout<<IQCservice.config_updated_name<<endl;
   // util.executeTerminal("xdg-open https://uat.newjaisa.biz/");
   string encrypted_pwd;
   json pwd_data;
   encrypted_pwd=encryption.encrypt_with_shift(pwd,4);
   pwd_data["pwd"]=encrypted_pwd;
   // IQCservice.ChecksResultSync("password","")

   // json completedStatus;
   pwd_data["app_status"] = "1";
   IQCservice.ChecksResultSync("passwor and completed Status","",pwd_data);
   

   

   string url= IQCservice.configname();

   const string CMD_ERP="powershell -Command \"Start-Process -FilePath 'chrome.exe' -ArgumentList '"+constant.apiBaseURL+"api/method/nj_features.nj_features.api.login.login?usr="+usr+"&pwd="+encrypted_pwd+"&doctype_url=njqca/"+url+"&encrypted_pwd=True"+"'\"";
   // cout<<CMD_ERP<<endl;
  
   cout<<CMD_ERP<<endl;
   util.executeTerminal(CMD_ERP);
   util.generatePopupOK("NJQCA Execution Completed");
   if(IQCservice.config_updated_name != ""){
      IQCservice.deleteNJQCAFile();
      IQCservice.deleteInspectionTypeFile();
   };
   auto end = std::chrono::high_resolution_clock::now();
   auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);

   printf("\nTime measured: %.2f seconds.\n", elapsed.count() * 1e-9);
   exit(0);
};
