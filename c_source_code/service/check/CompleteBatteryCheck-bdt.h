#include <iostream>
#include <string>
#include <map>

#include <ctime>
#include <iomanip>

using namespace std;



bool executeterminalVidioCommand(const std::string& command) {
    return system(command.c_str()) == 0;}

class CompleteBatteryCheck: public V2Service {

  private:
  const string CMD_BATTERY = "powershell -NoProfile -NonInteractive -Command \"Get-WmiObject -Class Win32_Battery | Format-List BatteryStatus,EstimatedChargeRemaining\""; 
  const string CMD_BATTERY_count = "powershell -NoProfile -NonInteractive -Command \"@((Get-WmiObject -Class Win32_Battery)).Count\""; 
  // string CMD_BATTERY = "wmic Path Win32_Battery get /format:list";
  string CMD_CPUload="powershell -Command \"Get-WmiObject -Class Win32_Processor | Select-Object -ExpandProperty LoadPercentage\"";
  Util util;
  PartResultService partresultservice;
  LanCheck lancheck;
  NjsettingsService njsettings;
  GetFromERPService getfromERP;
  HDDSentinelCheck hdd;
  Constant constant;
  DocType docType;
  std::thread videoThread;
  std::thread updateThread;
  std::string started_time;
  int cycle=0;
  bool endThread=true;
  bool run_performance=true;
  int backup_duration_mins=120;
  int sudden_drop_limit=15;
  int no_of_bdt=0;
  bool run_bdt_cycle=true;
  bool battery_detected=false;
  json jsonPartArray=json::array();
  // map<string, string> paramtrmap_battery2;
  // map<string, string> paramtrmap_battery1;
  // string cmd_battery1="upower -i /org/freedesktop/UPower/devices/battery_BAT0";
  // string cmd_battery2="upower -i /org/freedesktop/UPower/devices/battery_BAT1";
  // BatteryCheck batterycheck;
  NjsettingsService njset;
  IQCService iqcService;
  
  static int MAX_Charging_Time;
  static int Percentage_cheking_time;
  static int MAX_discharging_Time;
  static string BatteryVideoFile;
  static string urlfile;
  static int cycle_count;
  static int relay_on_percentage;
  static int relay_off_percentage;
  string CMD_discharge_rate="powershell -Command \"Get-CimInstance -Namespace \"root\\wmi\" -ClassName BatteryStatus | Select-Object -ExpandProperty DischargeRate\"";

  public:void completebatterytest(map<int,map<string,string>>mapfileds,int battery_count){
    cout<<"\nComplete Battery Check Running..."<<endl;
    cout<<"\nBattery Maximum Charging Time is set to "<<MAX_Charging_Time<<" Mins"<<endl;
    cout<<"\nBattery Percentage Checking Time is set to "<<Percentage_cheking_time<<" Mins"<<endl;
    cout<<"\nBattery MAX Discharging Time is set to "<<MAX_discharging_Time<< " Mins"<<endl;
    // cout<<"\nBattery Background Video file is set to "<<BatteryVideoFile<<endl;
    cout<<"\nBattery Testing Cycle Count is set to "<<cycle_count<<endl;
    cout<<"\nBattery Relay On Percentage is set to "<<relay_on_percentage<<endl;
    cout<<"\nBattery Relay Off Percentage is set to "<<relay_off_percentage<<endl;
    // cout<<"\n Battery URL file is set to "<<urlfile<<endl;
    // download_URL_file(urlfile,"Battery_urls.txt");
    if(battery_count==1){
      SingleBatteryTest(mapfileds);
    }
    // else if(battery_count==2){
    //   MultipleBatteryTest(mapfileds);
    // }
  };

  private: int SingleBatteryTest(map<int,map<string,string>>battery_data){
    string battery_count=util.executeTerminal(CMD_BATTERY_count);
    cout<<"NO OF BATTERY DETEDTED:"<<battery_count<<endl;
    for(int try_count=0;try_count<4;try_count++){
      try{
        if(stoi(battery_count)>1){
           util.generatePopupOK("More than one battery is detected in system.");
           exit(0);
        }else{
          break;
        }
      }catch(const std::exception& e){
        battery_count=util.executeTerminal(CMD_BATTERY_count);
        cout<<"NO OF BATTERY DETEDTED:"<<battery_count<<endl;
      }
       
    }
    
    // if(stoi(battery_count)>1){
    //   util.generatePopupOK("More than one battery is detected in system.");
    //   exit(0);
    // }

    map<string, string> paramtrmap;
    map<string, string> fieldidmap;
    string filepath;
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile != nullptr) {
        std::string path(userProfile);
        filepath = path + "\\Desktop\\battery_log.csv";
        // cout<<fullPath<<endl;
    }
    hdd.deletefile(filepath);
    int return_value = 0;
    string SerialNo;
    const string CMD_SYSTEM= "powershell -Command \"Get-WmiObject -Class Win32_computersystemproduct\"";
    json jsonObject  = util.executeTerminalJSON(CMD_SYSTEM,":");
    for (const auto& item : jsonObject.items()) {
        if (item.key() == "IdentifyingNumber") {SerialNo = item.value();
        break;
        }
    }
    map<int, map<string, string>> ::iterator it = battery_data.begin();
    while(it!=battery_data.end()){
      map<string, string> partdata = it->second;
      string partname=partdata["njhat_item_name"];
      if(partname=="Battery"){
        battery_detected=true;
        json input;
        input["bios_serial_no"]=SerialNo;
        input["device_id"]=partdata["attr_01"];
        map<string,string>requestmap;
        string response=iqcService.getParameterList(constant.API_NO_OF_BDT_URL,"PUT",requestmap,to_string(input));
        try {
          json jsonresponse = json::parse(response);
           no_of_bdt= jsonresponse["message"];
           cout<<"\nno of bdt done:"<<no_of_bdt<<endl;
           cycle_count-=no_of_bdt;
          } catch (const exception& e) {
          cerr << "Error while getting the no of bdt: " << e.what() << response<<endl;
         }
         cout<<"\nno of cycle remaining:"<<cycle_count<<endl;
         if (cycle_count<=0){
          util.Generatepopup("Cycle count for the battery");
          exit(0);
         }
         json jsonData;
         jsonData ["manufacturer_model"]= iqcService.model_name;
         // cout<<"manufacturer_model: "<<iqcService.model_name<<endl;
         string wizard=iqcService.getParameterList(constant.API_Wizard, "PUT", requestmap, to_string(jsonData));
         // cout<<"item wizard: "<<wizard<<endl;
         if(wizard!=""){
           try {
           json jsonresponse = json::parse(wizard);
           backup_duration_mins = stoi(jsonresponse.at("message").at("backup_duration_mins").get<string>());
           sudden_drop_limit = stoi(jsonresponse.at("message").at("sudden_drop_limit").get<string>());
           } catch (const exception& e) {
           cerr << "Error while getting the Wizard criteria: " << e.what() << wizard<<endl;
           }
         }
         cout<<"backup duration required:"<<backup_duration_mins<<endl;
         cout<<"sudden drop limit:"<<sudden_drop_limit<<endl;
        // if(partdata["attr_01"]=="PORTABLE BATTERY 0" || partdata["attr_01"]=="PORTABLE BATTERY 1"){
          fieldidmap["stock_info_id"] = partdata["stock_info_id"];
          fieldidmap["part_id"] = partdata["stock_part_id"];
          fieldidmap["part_config_id"] = partdata["part_config_id"];
          fieldidmap["item_id"] = partdata["item_id"];
          fieldidmap["created_by"] = partdata["created_by"];
          // BatteryThread=std::thread(&CompleteBatteryCheck::update_percentage_change,this,fieldidmap);
          for(int i=1;i<=cycle_count;i++){
            if(run_bdt_cycle){
            cycle=i;
            paramtrmap = test(fieldidmap);}
          }
          if(!ischarging(CMD_BATTERY)){
            util.generatePopupOK("Please Connect power cable to charge Fully?");
            Sleep(3000);
          }

          if(ischarging(CMD_BATTERY)){
            cout<<"\nBattery is Charging..."<<endl;
          
          while(Battery_percentage(CMD_BATTERY)<=relay_off_percentage){         
            Sleep(10000);
          }}else{
            util.generatePopupOK("Battery Not Chrageing. Exiting without fully charge..");
          }
        // util.executeTerminal("pkill firefox");
        util.generatePopupOK("Battery Test Completed...");
        // system("zenity --warning --title=\"NJQCA Script\" --text=\"<span font='30'>NJ BDT Completed...</span>\" --width=700 --height=300");


        // util.executeTerminal("xdg-open Temp_Data/index.html"); 
        // util.executeTerminal("xdg-open Temp_Data/index.html");

          // return_value = partresultservice.updateBulkPartResult(paramtrmap, fieldidmap);
          // iqcService.ChecksResultSync("Battery Check");
          break;
        // }
      }
    ++it;
    }
    if(!battery_detected){
      util.generatePopupOK("Battery Not Detected in the system.");
    }
    cout<<"\nComplete Battery Check Completed..."<<endl;
    return return_value;
  }

  private:  map<string, string> test(map<string, string> fieldMap) {
    map<string, string> paramtrmap;

    if(Battery_percentage(CMD_BATTERY)<relay_off_percentage){
      relay_off();
      Sleep(3000);

      // util.generatePopup("Please insert the Power cable and press yes to continue?");
      // Sleep(3000);
      if (!ischarging(CMD_BATTERY)) { 
      cout << "\nAre you sure U inserted the Power cable and Battery port Working.....[y/Y]? "<<std::flush;
      util.generatePopupOK("Are you sure U inserted the Power cable and Battery port Working?");
      // confirm();
      Sleep(3000);
      if (!ischarging(CMD_BATTERY)) {
        paramtrmap["Battery_status"] = "Not charging";
        update_data(paramtrmap,fieldMap);
        return paramtrmap;
      }
      
    }
    
    cout << "\nPlease Wait Battery is charging.........\n"<<std::flush;

    for(int i=0;i<MAX_Charging_Time*60;i+=10){
      if(Battery_percentage(CMD_BATTERY)>=relay_off_percentage){ break; }
       Sleep(10000);
      if(!ischarging(CMD_BATTERY)){
        util.generatePopup("Battery Not Charging", "Please inser power cable and click OK?");
      }
    }
    }
    
    std::string command = "start /B \"\" \"performance.exe\"";
    string checkprocess="tasklist | findstr \"performance.exe\"";
    relay_on();
    Sleep(5000);
    // cout << "\nPlease remove the Power cable and press 'y' to continue. .....[y/Y] "<<std::flush;
    // util.generatePopup("Please remove the Power cable and press yes to continue?");
    // // confirm();
    // Sleep(3000);

    if (!isdischarging(CMD_BATTERY)) {
      cout << "\nAre you sure you removed the Power Cable, Battery is  discharging......[y/Y] "<<std::flush;
      util.generatePopupOK("Are you sure you removed the Power Cable, Battery is  discharging?");
      // confirm();
      Sleep(3000);
      if (!isdischarging(CMD_BATTERY)) {
        paramtrmap["Battery_status"] = "Not Discharging";
        update_data(paramtrmap,fieldMap);
        return paramtrmap;
      }
    }
    
    auto start = std::chrono::steady_clock::now();
    json bdt_cycles;
    bdt_cycles["number"]=to_string(cycle+no_of_bdt);
    bdt_cycles["name"]=iqcService.config_updated_name;
    map<string,string>requestmapempty;
    string update_response=iqcService.getParameterList(constant.API_update_BDT_cycle,"PUT",requestmapempty,to_string(bdt_cycles));
    cout<<"update response:"<<update_response<<endl;

    string discharging_time=getCurrentTime();
    string CPU_load=cpuLoad_percentage();
    string dischaging_rate=discharge_rate();
    int battery_percentage=Battery_percentage(CMD_BATTERY);
    cout<<"\nBattery DisCharging starts at "<<discharging_time<<" with Battery Percentage "<< battery_percentage << "%"<<std::flush<<endl;
    paramtrmap["Battery_Discharging_Start_Time"]=discharging_time;
    paramtrmap[discharging_time]=to_string(battery_percentage);
    update_data(paramtrmap,fieldMap,CPU_load, dischaging_rate);
    paramtrmap.clear();

    for(int i=Percentage_cheking_time;i<=MAX_discharging_Time;i+=Percentage_cheking_time){
      if(!isdischarging(CMD_BATTERY)){
        util.generatePopup("Battery Not Discharging", "Please remove the power cable and click OK?");      
      }
      create_offline_data();
      string result=util.executeTerminal(checkprocess);
      // cout<<"\noutput of checkprocess: "<<result<<endl;
      if(result.empty()){
      // cout<<"\nresult true..."<<endl;
      system(command.c_str());
      // cout<<"\nchrome running in background..."<<endl;
      }
      

      // cout<<"video running in background..."<<endl;
      this_thread::sleep_for(chrono::minutes(Percentage_cheking_time));
      battery_percentage=Battery_percentage(CMD_BATTERY);
      discharging_time=getCurrentTime();
      CPU_load=cpuLoad_percentage();
      dischaging_rate=discharge_rate();
      cout<<"\nBattery percentage after "+to_string(i)+" Mins of DisCharging:" << battery_percentage <<"% at "<<discharging_time<<std::flush<<endl;
      paramtrmap[discharging_time]=to_string(battery_percentage);
      if(updateThread.joinable()){updateThread.join();}
      updateThread=std::thread(&CompleteBatteryCheck::update_data,this,paramtrmap,fieldMap,CPU_load,dischaging_rate);
      paramtrmap.clear();
      if(battery_percentage<=sudden_drop_limit){
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::minutes>(end - start);
        if(int(duration.count())>=backup_duration_mins){
           run_bdt_cycle=false;
        }
      }
      // executeterminalVidioCommand("pkill vlc");
      if(battery_percentage<=relay_on_percentage){
        // auto end = std::chrono::steady_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::minutes>(end - start);
        // json Backup_duration;
        // Backup_duration["backup_duration_cycle_"+to_string(cycle)]=to_string(duration.count());
        // iqcService.ChecksResultSync("Complete Battery Check","",Backup_duration);
        // util.generatePopupOK("Cycle "+to_string(cycle)+" Completed");
        relay_off();
        Sleep(5000);
        break; 
      }
    }
    return paramtrmap;
  }


   
  // public:void update_data(map<string,string>parameters,map<string,string>fileds, string cpu_load="", string discharge_rate=""){
  //   int returnValue = 0;
  //   std::map<std::string, string>::iterator it = parameters.begin();
    
  //   while (it != parameters.end()) {
  //     std::map<std::string, string> partResult;
  //     partResult["param"] = it->first;
  //     partResult["result"] = it->second;
  //     partResult["cpu_load"] = cpu_load;
  //     partResult["discharge_rate"] = discharge_rate;
  //     partResult["stock_info_id"]=fileds["stock_info_id"];
  //     partResult["part_config_id"]=fileds["part_config_id"];
  //     partResult["part_id"]=fileds["part_id"];
  //     partResult["item_id"]=fileds["item_id"];
  //     partResult["created_by"]=fileds["created_by"];
  //     returnValue = partresultservice.updatePartResult(partResult);
  //     it++; 
  //   }
  
  //   iqcService.ChecksResultSync("Complete Battery Check");
  // }
  
  public:void update_data(map<string,string>parameters,map<string,string>fileds,string cpu_load="", string discharge_rate=""){
    int returnValue = 0;
    string parent=iqcService.config_updated_name;
    std::map<std::string, string>::iterator it = parameters.begin();
    
    while (it != parameters.end()) {
      json jsonPartObject; 
      jsonPartObject["parameter"] = it->first;
      jsonPartObject["value"] = it->second;
      jsonPartObject["part_config_id"] = fileds["part_config_id"];
      jsonPartObject["discharge_rate"] = discharge_rate;
      jsonPartObject["cpu_load"] = cpu_load;
      jsonPartObject["parent"] = parent;
      jsonPartObject["item_group"] = "BATTERY";

      jsonPartArray.insert(jsonPartArray.end(), jsonPartObject);
      it++; 
    }
    json result;
    result["njqca_result_parameters"]=jsonPartArray;
    // cout<<result.dump(2)<<endl;
    bool status=iqcService.ResultSync("Complete Battery Check",result);
    if(status){
      jsonPartArray.clear();
    }
  }


    public: bool ischarging(string cmd_battery){
    json jsondata = getTestResult(cmd_battery);
    if(jsondata.size()==0){
      for(int i=0;i<5;i++){
        jsondata = getTestResult(cmd_battery);
        if(jsondata.size()>0){
          break;
        }
      }
    }
    cout<<"\nCharging Battery Status:"<<jsondata[0]["BatteryStatus"]<<endl;
    if (jsondata[0]["BatteryStatus"] == "2") {
      return true;
    }
    return false;
  }
 
    public: bool isdischarging(string cmd_battery){
    json jsondata = getTestResult(cmd_battery);
    if(jsondata.size()==0){
      for(int i=0;i<5;i++){
        jsondata = getTestResult(cmd_battery);
        if(jsondata.size()>0){
          break;
        }
      }
    }
    cout<<"\nDischarging Battery Status:"<<jsondata[0]["BatteryStatus"]<<endl;
    if (jsondata[0]["BatteryStatus"] == "1") {
      return true;
    }
    return false;
  }


  public: int Battery_percentage(string cmd_battery){
    int current_percentage =0;
    json jsondata = getTestResult(cmd_battery);
    if(jsondata.size()==0){
      for(int i=0;i<5;i++){
        jsondata = getTestResult(cmd_battery);
        if(jsondata.size()>0){
          break;
        }
      }
    }
    try{
	  current_percentage = stoi(jsondata[0]["EstimatedChargeRemaining"].get<string>());
    cout<<"\nBattery_Percentage: "<<current_percentage<<endl;
    }catch(const std::exception& e) {
      cout<<"\nUnable to fetch battery percentage..."<<endl;
    }
    if(current_percentage>100){
      current_percentage=100;
    }
    return current_percentage;
  }

    public: string cpuLoad_percentage(){
    string CPUdata = util.executeTerminal(CMD_CPUload);
	  CPUdata=util.trim(CPUdata);
    return CPUdata;
  }

    public: string discharge_rate(){
    string dischargedata = util.executeTerminal(CMD_discharge_rate);
	  dischargedata=util.trim(dischargedata);
    float rate;
    try{
    rate=stoi(dischargedata);
    rate=rate/1000;
    dischargedata=to_string(rate);
    }catch(std::exception& e){
      cout<<"\nUnable to fetch discharge rate..."<<dischargedata<<endl;
    }
    return dischargedata;
  }

  private:  bool confirm() {
    string confirm;
    map<string, string> validConfirmInfo = { {"Yes", "0"}, {"yes", "1"}, {"YES", "2"}, {"y", "3"}, {"Y", "4"} };
    do {
        cout<<"\nPlease confirm:";
        cin >> confirm;
    } while (validConfirmInfo.find(confirm) == validConfirmInfo.end());
    return true;
  
  }
  

public:json getTestResult(std::string cmd_battery) {        
    std::string result = util.executeTerminal(cmd_battery);
    // cout<<"\nresult: "<<result<<endl;
    // result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
    // Parse the result into a JSON array
    std::istringstream inputStream(result);
    json batteryData = util.ExcetuteTerminalJSonArray1(inputStream);
    
    return batteryData; 
  }

  std::string getCurrentTime() {
    
    auto now = std::chrono::system_clock::now();

    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    std::tm* localTime = std::localtime(&currentTime);

    std::ostringstream timeStream;
    timeStream << std::put_time(localTime, "%H:%M:%S");
    return timeStream.str();
}

  public:void updateCompleteBatteryCheckTime(string department,json result){
    // json result=njsettings.getNjSettingsvalues();
    for(const auto&item: result){
      if(item["department"]==department){
        if(item["parameter"]=="MAX_Charging_Time"){
          MAX_Charging_Time=stoi(item["value"].get<string>());}
        else if(item["parameter"]=="Percentage_Checking_Time"){
          Percentage_cheking_time=stoi(item["value"].get<string>());
        }
        else if(item["parameter"]=="MAX_discharging_Time"){
          MAX_discharging_Time=stoi(item["value"].get<string>());
        }
        else if(item["parameter"]=="BatteryVideoFile"){
          BatteryVideoFile=item["value"].get<string>();
        }
        else if(item["parameter"]=="cycle_count"){
          cycle_count=stoi(item["value"].get<string>());
        }
        else if(item["parameter"]=="relay_on_percentage"){
          relay_on_percentage=stoi(item["value"].get<string>());
        }
        else if(item["parameter"]=="relay_off_percentage"){
          relay_off_percentage=stoi(item["value"].get<string>());
        }
        else if(item["parameter"]=="BatteyURLfile"){
          urlfile=item["value"].get<string>();
        }
      }
    }
  }



  public :void download_URL_file(string fileurl, string filename){
    if(!fileurl.empty()){
    
   fileurl=lancheck.urlEncode(fileurl);
   lancheck.downloadFile(fileurl,filename);}
  }

  private:bool relay_on(){
    string relay_on_result;
    string relay_on_command="relay_on.exe";
    relay_on_result=util.executeTerminal(relay_on_command);
    if(relay_on_result.find("Relay turned ON successfully.") != std::string::npos){
      cout<<"\nRelay turn on Successfully..."<<endl;
      return true;
    }else{
      cout<<"\nRelay Failed to turn on..."<<endl;
      cout<<relay_on_result<<endl;
      return false;
    }
  
  }
  private:bool relay_off(){
    string relay_off_result;
    string relay_off_command="relay_off.exe";
    relay_off_result=util.executeTerminal(relay_off_command);
    if(relay_off_result.find("Relay turned OFF successfully.") != std::string::npos){
      cout<<"\nRelay turn off Successfully..."<<endl;
      return true;
    }else{
      cout<<"\nRelay Failed to turn off..."<<endl;
      cout<<relay_off_result<<endl;
      return false;
    }
  
  }

  void create_offline_data(){
    std::string Script = 
    "powershell -NoProfile -Command \""
    "$desktopPath = [Environment]::GetFolderPath('Desktop'); "
    "$csvPath = Join-Path $desktopPath 'battery_log.csv'; "
    "if (!(Test-Path $csvPath)) { "
    "    'Time,BatteryPercentage,DischargingRate,CpuLoad' | Set-Content -Path $csvPath -Encoding UTF8; "
    "} "
    "$time = (Get-Date).ToString('HH:mm:ss'); "
    "$battery = Get-CimInstance -Namespace root\\wmi -ClassName BatteryStatus; "
    "$full = Get-CimInstance -Namespace root\\wmi -ClassName BatteryFullChargedCapacity; "
    "if ($battery -and $full -and $full.FullChargedCapacity -ne 0) { "
    "    $batteryPercent = '{0:N0}' -f (($battery.RemainingCapacity / $full.FullChargedCapacity) * 100); "
    "} else { "
    "    $batteryPercent = 'N/A'; "
    "} "
    "$dischargingRateObj = $battery.DischargeRate; "
    "$dischargingRate = if ($dischargingRateObj) { $dischargingRateObj } else { 'N/A' }; "
    "$cpuLoad = Get-WmiObject -Class Win32_Processor | Select-Object -ExpandProperty LoadPercentage; "
    "Add-Content -Path $csvPath -Value \\\"$time,$batteryPercent,$dischargingRate,$cpuLoad\\\"; "
    "\"";   
    util.executeTerminal(Script); 
  }

  public: bool offline_data_sync(){
    std::string fullPath;
    json inputjson;
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile != nullptr) {
        std::string path(userProfile);
        fullPath = path + "\\Desktop\\battery_log.csv";
        // cout<<fullPath<<endl;
    }
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file.\n";
        return false;
    }

    std::string line;
    std::vector<std::string> headers;
    std::vector<std::string> NJQCA_headers;
    json jsonArray = json::array();

    NJQCA_headers.push_back("parameter");
    NJQCA_headers.push_back("value");
    NJQCA_headers.push_back("discharge_rate");
    NJQCA_headers.push_back("cpu_load");

    // Read headers
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string col;
        while (std::getline(ss, col, ',')) {
            headers.push_back(col);
        }
    }
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        json record;

        for (size_t i = 0; i < headers.size(); ++i) {
            if (!std::getline(ss, value, ',')) break;

            record[NJQCA_headers[i]] = value;
            record["item_group"]="BATTERY";
            
        }

        if (!record.empty()) {
            jsonArray.push_back(record);
        }
    }

    file.close();
    json discharging_time;
    discharging_time["parameter"]="Battery_Discharging_Start_Time";
    discharging_time["value"]=jsonArray[0]["parameter"];
    discharging_time["item_group"]="BATTERY";

    jsonArray.insert(jsonArray.begin() + 1, discharging_time);

    inputjson["njqca_result_parameters"]=jsonArray;
    std::cout << inputjson.dump(2) << std::endl;
    string updated_name="/"+util.encodeURL(iqcService.config_updated_name);
    // cout<<updated_name<<endl;
    map<string,string>requestMap;
    string response = iqcService.api2ERP(constant.API_DOC_URL + docType.IQC + updated_name , "PUT", requestMap, to_string(inputjson));
    // cout<<"response: "<<response<<endl;
    logInfo(response);
    if(response.empty()){
      cout<<"\nFailed to sync offline data to ERP..."<<endl;
      return false;
    }
    cout<<"offline battery data synced to ERP successfully..."<<endl;
    return true;
  }

};
    int CompleteBatteryCheck::MAX_Charging_Time=300;
    int CompleteBatteryCheck::Percentage_cheking_time=1;
    int CompleteBatteryCheck::MAX_discharging_Time=720;
    int CompleteBatteryCheck::cycle_count=3;
    int CompleteBatteryCheck::relay_on_percentage=6;
    int CompleteBatteryCheck::relay_off_percentage=99;
    string CompleteBatteryCheck::BatteryVideoFile;
    string CompleteBatteryCheck::urlfile;