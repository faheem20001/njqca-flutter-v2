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
  const string CMD_BATTERY = "powershell -Command \"Get-WmiObject -Class Win32_Battery | Format-List BatteryStatus,EstimatedChargeRemaining\""; 
  // string CMD_BATTERY = "wmic Path Win32_Battery get /format:list";
  string CMD_CPUload="powershell -Command \"Get-WmiObject -Class Win32_Processor | Select-Object -ExpandProperty LoadPercentage\"";
  Util util;
  PartResultService partresultservice;
  LanCheck lancheck;
  NjsettingsService njsettings;
  GetFromERPService getfromERP;
  Constant constant;
  std::thread videoThread;
  std::thread updateThread;
  std::string started_time;
  int cycle=0;
  bool endThread=true;
  bool run_performance=true;
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
  string CMD_discharge_rate="powershell -Command \"Get-CimInstance -Namespace \"root\\wmi\" -ClassName BatteryStatus | Where-Object { $_.Discharging -eq $true } | Select-Object -ExpandProperty DischargeRate\"";


  public:void completebatterytest(map<int,map<string,string>>mapfileds,int battery_count){
    cout<<"\nComplete Battery Check Running..."<<endl;
    cout<<"\nBattery Maximum Charging Time is set to "<<MAX_Charging_Time<<" Mins"<<endl;
    cout<<"\nBattery Percentage Checking Time is set to "<<Percentage_cheking_time<<" Mins"<<endl;
    cout<<"\nBattery MAX Discharging Time is set to "<<MAX_discharging_Time<< " Mins"<<endl;
    cout<<"\nBattery Background Video file is set to "<<BatteryVideoFile<<endl;
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
    map<string, string> paramtrmap;
    map<string, string> fieldidmap;
    int return_value = 0;
    map<int, map<string, string>> ::iterator it = battery_data.begin();
    while(it!=battery_data.end()){
      map<string, string> partdata = it->second;
      string partname=partdata["njhat_item_name"];
      if(partname=="Battery"){
        // if(partdata["attr_01"]=="PORTABLE BATTERY 0" || partdata["attr_01"]=="PORTABLE BATTERY 1"){
          fieldidmap["stock_info_id"] = partdata["stock_info_id"];
          fieldidmap["part_id"] = partdata["stock_part_id"];
          fieldidmap["part_config_id"] = partdata["part_config_id"];
          fieldidmap["item_id"] = partdata["item_id"];
          fieldidmap["created_by"] = partdata["created_by"];
          // BatteryThread=std::thread(&CompleteBatteryCheck::update_percentage_change,this,fieldidmap);
          for(int i=1;i<=cycle_count;i++){
            cycle=i;
            paramtrmap = test(fieldidmap);
          }
          if(!ischarging(CMD_BATTERY)){
            // util.generatePopup("Please Connect power cable to charge Fully?");
            Sleep(3000);
          }

          if(ischarging(CMD_BATTERY)){
            cout<<"\nBattery is Charging..."<<endl;
          
          while(Battery_percentage(CMD_BATTERY)<=relay_off_percentage){         
            Sleep(60);
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
    cout<<"\nComplete Battery Check Completed..."<<endl;
    return return_value;
  }

  private:  map<string, string> test(map<string, string> fieldMap) {
    map<string, string> paramtrmap;

    if(Battery_percentage(CMD_BATTERY)<relay_off_percentage){
      // util.generatePopup("Please insert the Power cable and press yes to continue?");
      Sleep(3000);
      if (!ischarging(CMD_BATTERY)) { 
      cout << "\nAre you sure U inserted the Power cable and Battery port Working.....[y/Y]? "<<std::flush;
      // util.generatePopup("Are you sure U inserted the Power cable and Battery port Working?");
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
    }
    }
    
    std::string command = "start /B \"\" \"performance.exe\"";
    string checkprocess="tasklist | findstr \"performance.exe\"";
    // relay_off();
    // sleep(10);
    cout << "\nPlease remove the Power cable and press 'y' to continue. .....[y/Y] "<<std::flush;
    // util.generatePopup("Please remove the Power cable and press yes to continue?");
    // confirm();
    Sleep(3000);

    if (!isdischarging(CMD_BATTERY)) {
      cout << "\nAre you sure you removed the Power Cable, Battery is  discharging......[y/Y] "<<std::flush;
      // util.generatePopup("Are you sure you removed the Power Cable, Battery is  discharging?");
      // confirm();
      Sleep(3000);
      if (!isdischarging(CMD_BATTERY)) {
        paramtrmap["Battery_status"] = "Not Discharging";
        update_data(paramtrmap,fieldMap);
        return paramtrmap;
      }
    }
    
    auto start = std::chrono::steady_clock::now();
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
      // executeterminalVidioCommand("pkill vlc");
      if(battery_percentage<=relay_on_percentage){
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::minutes>(end - start);
        json Backup_duration;
        Backup_duration["backup_duration_cycle_"+to_string(cycle)]=to_string(duration.count());
        iqcService.ChecksResultSync("Complete Battery Check","",Backup_duration);
        util.generatePopupOK("Cycle "+to_string(cycle)+" Completed");
        // relay_on();
        Sleep(5000);
        break; 
      }
    }
    return paramtrmap;
  }


   
  public:void update_data(map<string,string>parameters,map<string,string>fileds, string cpu_load="", string discharge_rate=""){
    int returnValue = 0;
    std::map<std::string, string>::iterator it = parameters.begin();
    
    while (it != parameters.end()) {
      std::map<std::string, string> partResult;
      partResult["param"] = it->first;
      partResult["result"] = it->second;
      partResult["cpu_load"] = cpu_load;
      partResult["discharge_rate"] = discharge_rate;
      partResult["stock_info_id"]=fileds["stock_info_id"];
      partResult["part_config_id"]=fileds["part_config_id"];
      partResult["part_id"]=fileds["part_id"];
      partResult["item_id"]=fileds["item_id"];
      partResult["created_by"]=fileds["created_by"];
      returnValue = partresultservice.updatePartResult(partResult);
      it++; 
    }
  
    iqcService.ChecksResultSync("Complete Battery Check");
  }

    public: bool ischarging(string cmd_battery){
    json jsondata = getTestResult(cmd_battery);
    cout<<"\nCharging Battery Status:"<<jsondata[0]["BatteryStatus"]<<endl;
    if (jsondata[0]["BatteryStatus"] == "2") {
      return true;
    }
    return false;
  }
 
    public: bool isdischarging(string cmd_battery){
    json jsondata = getTestResult(cmd_battery);
    cout<<"\nDischarging Battery Status:"<<jsondata[0]["BatteryStatus"]<<endl;
    if (jsondata[0]["BatteryStatus"] == "1") {
      return true;
    }
    return false;
  }


  public: int Battery_percentage(string cmd_battery){
    json jsondata = getTestResult(cmd_battery);
	  int current_percentage = stoi(jsondata[0]["EstimatedChargeRemaining"].get<string>());
    // cout<<"\nBattery_Percentage: "<<current_percentage<<endl;
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

  // private:bool relay_off(){
  //   string relay_off_result;
  //   string relay_off_command="./relay_off";
  //   relay_off_result=util.executeTerminal(relay_off_command);
  //   if(relay_off_result.find("BoardCount: 0") != std::string::npos ||relay_off_result.find("Error") != std::string::npos){
  //     cout<<"\nRelay module not found Please Remove and insert again...[y/Y]?"<<std::flush<<endl;
  //     confirm();
  //     sleep(2);
  //     relay_off_result=util.executeTerminal(relay_off_command);
  //   }
  //   if(relay_off_result.find("Turn Off Successfully") != std::string::npos){
  //     cout<<"\nRelay turn off Successfully..."<<endl;
  //     return true;
  //   }else{
  //     cout<<"\nRelay Failed to turn off..."<<endl;
  //     cout<<relay_off_result<<endl;
  //     return false;
  //   }
  
  // }

  public :void download_URL_file(string fileurl, string filename){
    if(!fileurl.empty()){
    
   fileurl=lancheck.urlEncode(fileurl);
   lancheck.downloadFile(fileurl,filename);}
  }

  // private:bool relay_on(){
  //   string relay_on_result;
  //   string relay_on_command="./relay_on";
  //   relay_on_result=util.executeTerminal(relay_on_command);
  //   if(relay_on_result.find("BoardCount: 0") != std::string::npos ||relay_on_result.find("Error") != std::string::npos){
  //     cout<<"\nRelay module not found Please Remove and insert again...[y/Y]?"<<std::flush<<endl;
  //     confirm();
  //     sleep(2);
  //     relay_on_result=util.executeTerminal(relay_on_command);
  //   }
  //   if(relay_on_result.find("Turn On Successfully") != std::string::npos){
  //     cout<<"\nRelay turn on Successfully..."<<endl;
  //     return true;
  //   }else{
  //     cout<<"\nRelay Failed to turn on..."<<endl;
  //     cout<<relay_on_result<<endl;
  //     return false;
  //   }
  
  // }

};
    int CompleteBatteryCheck::MAX_Charging_Time=180;
    int CompleteBatteryCheck::Percentage_cheking_time=1;
    int CompleteBatteryCheck::MAX_discharging_Time=720;
    int CompleteBatteryCheck::cycle_count=2;
    int CompleteBatteryCheck::relay_on_percentage=55;
    int CompleteBatteryCheck::relay_off_percentage=61;
    string CompleteBatteryCheck::BatteryVideoFile;
    string CompleteBatteryCheck::urlfile;