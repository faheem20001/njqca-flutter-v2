#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <synchapi.h>

using namespace std;
using json = nlohmann::json;

class BatteryCheck: public V2Service {

  private:
    IQCService iqcService;
    NjsettingsService njsetting;
    Constant constant;
    std::thread dischargeThread;
    // string CMD_BATTERY = "upower -e|grep battery";
    Util util;
    PartResultService partresultservice;
    map<string, string> paramtrmap_battery2;
    map<string, string> paramtrmap_battery1;
    //string cmd_battery1="upower -i /org/freedesktop/UPower/devices/battery_BAT0";
    //string cmd_battery1 = "wmic Path Win32_Battery Get BatteryStatus,EstimatedChargeRemaining /format:list";
    const string cmd_battery1 = "powershell -Command \"Get-WmiObject -Class Win32_Battery | Format-List BatteryStatus,EstimatedChargeRemaining\""; 
    const string cmd_power="powershell -Command \"Get-WmiObject -Class BatteryStatus -Namespace root\\wmi | Format-List PowerOnline\"";
    // string cmd_battery1 = "wmic Path Win32_Battery get /format:list";
    // string cmd_battery2="upower -i /org/freedesktop/UPower/devices/battery_BAT1";
    std::mutex io_mutex;
    std::condition_variable cv;
    bool chargingCompleted = false;
    
    public:
     static int time;
     static int health_passing_percenatge;
     static int passing_cc;
     static int battery_charge_percentage;

public:json getTestResult(std::string cmd_battery) {        
    std::string result = util.executeTerminal(cmd_battery);
    result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
    // Parse the result into a JSON array
    std::istringstream inputStream(result);
    json batteryData = util.ExcetuteTerminalJSonArray1(inputStream);
    // cout<<"\nBattery Data: "<<batteryData.dump(2)<<endl;

    return batteryData; 
  }


  public: void Battery_test(map<int,map<string,string>>battery_data,int battery_count,string parameters) {
    map<string, string> paramtrmap;
    map<string, string> fieldidmap;
    cout<<"\nBattery Check Running..."<<endl;
    cout<<"\nBattery Charging and Discharging Time is set to "<<time<<" Mins"<<endl;
    if(runBatteryTest(parameters)){
      if(parameters.find("Full_BDT_criteria")!=string::npos){
      cout<<"Battery critria checking..."<<endl;
      map<string, string> requestMap;
      json jsonData;
      jsonData ["manufacturer_model"]= iqcService.model_name;
      // cout<<"manufacturer_model: "<<iqcService.model_name<<endl;
      string wizard=iqcService.getParameterList(constant.API_Wizard, "PUT", requestMap, to_string(jsonData));
      string FBDT_criteria="60 - 100";
      int cycle_count=700;
      // cout<<"item wizard: "<<wizard<<endl;
      if(wizard!=""){
        try {
        json jsonresponse = json::parse(wizard);
        FBDT_criteria = jsonresponse.at("message").at("fbdt_health_range");
        cycle_count = stoi(jsonresponse.at("message").at("cycle_count").get<string>());
        } catch (const exception& e) {
        cerr << "Error while getting the Wizard criteria: " << e.what() << wizard<<endl;
        }
      }
      map<int, string>FBDT=util.splitString(FBDT_criteria, "-");

      map<int, map<string, string>> ::iterator object = battery_data.begin();
      while(object!=battery_data.end()){
        map<string, string> partdata = object->second;
        string partname=partdata["njhat_item_name"];
        // cout<<"partname: "<<partname<<endl;
        try{
        if(partname=="Battery"){
            fieldidmap["stock_info_id"] = partdata["stock_info_id"];
            fieldidmap["part_id"] = partdata["stock_part_id"];
            fieldidmap["part_config_id"] = partdata["part_config_id"];
            fieldidmap["item_id"] = partdata["item_id"];
            fieldidmap["created_by"] = partdata["created_by"];
          
          bool cc_status=true;
          string cc=partdata["attr_09"];
          cc=util.trim(cc);
          // cout<<"cycle count"<<cc<<endl;
           for(char ch:cc){
            if(!isdigit(ch)){
              cc_status=false;
              break;
            } 
          }
  
          if(!cc_status){
            cc="0";
          }
       
          

        if((stoi(partdata["attr_10"])>=stoi(FBDT[0])) && (stoi(partdata["attr_10"])<=stoi(FBDT[1])) && (cycle_count>=stoi(cc)) ){
            
          paramtrmap["Full_BDT_criteria"]="Need to run Full BDT";
        }else{
          paramtrmap["Full_BDT_criteria"]="scrap";
        }
          
        }
          }catch(std::exception &e){
          cout<<"ERROR While getting the Battery condition:"<<e.what()<<endl;
        }
        ++object;
  
      } 
      partresultservice.updateBulkPartResult(paramtrmap, fieldidmap);
      iqcService.ChecksResultSync("Battery criteria Check");
      
    }

    if(battery_count==1){
      dischargeThread=std::thread(&BatteryCheck::SingleBatteryTest,this,battery_data,parameters);
      // std::unique_lock<std::mutex> lock(io_mutex);
      // cv.wait(lock, [this]() { return chargingCompleted; });
    }
    else if(battery_count==2){
      dischargeThread=std::thread(&BatteryCheck::MultipleBatteryTest,this,battery_data,parameters);
      // std::unique_lock<std::mutex> lock(io_mutex);
      // cv.wait(lock, [this]() { return chargingCompleted; });
    }

    }
  }

  public:void battery_charge_test(string stock_info_id,string part_id,string item_id){
    map<string, string> paramtrmap;
    map<string, string> fieldidmap;
    
    int current_percentage=0;
    current_percentage=Battery_percentage(cmd_battery1);
    int second_battery=Battery_percentage2(cmd_battery1);
    if(second_battery>0){
      current_percentage=(current_percentage+second_battery)/2;
    }
    if(current_percentage>=battery_charge_percentage){
      paramtrmap["charge_percentage"] = to_string(current_percentage);
      paramtrmap["Battery_charge_percentage_status"] = "PASS";
    }else{
      bool run_loop=false;
      if(ischarging(cmd_battery1) || ischarging2(cmd_battery1)){
        run_loop=true;
      }else{
        bool status=util.generatePopup("Battery is not charging","If U want to retry connect Adapter then click retry, to exit click Not working?");
        if(status){
          paramtrmap["charge_percentage"] = to_string(current_percentage);
          paramtrmap["Battery_charge_percentage_status"] = "FAIL";
          paramtrmap["reason"]="Cancelled By User";
        }else{
          run_loop=true;
        }
      }
      // Replace the while(run_loop) block (lines 168-190) with:
int max_retries = 200; // 200 x 3s = ~10 min max wait
int retry_count = 0;
while(run_loop){
    cout<<"\033[33m\nBattery is charging... ("<<retry_count+1<<"/"<<max_retries<<")\033[0m"<<flush;
    current_percentage=Battery_percentage(cmd_battery1);
    second_battery=Battery_percentage2(cmd_battery1);
    if(second_battery>0){
      current_percentage=(current_percentage+second_battery)/2;
    }
    if(current_percentage>=battery_charge_percentage){
      paramtrmap["charge_percentage"] = to_string(current_percentage);
      paramtrmap["Battery_charge_percentage_status"] = "PASS";
      break;
    }
    if(!ischarging(cmd_battery1) && !ischarging2(cmd_battery1)){
      bool status=util.generatePopup("Battery is not charging","Connect Adapter then click retry, or click Not working to skip");
      if(status){
        paramtrmap["charge_percentage"] = to_string(current_percentage);
        paramtrmap["Battery_charge_percentage_status"] = "FAIL";
        paramtrmap["reason"]="Cancelled By User";
        break;
      }
    }
    retry_count++;
    if(retry_count >= max_retries){
      paramtrmap["charge_percentage"] = to_string(current_percentage);
      paramtrmap["Battery_charge_percentage_status"] = "FAIL";
      paramtrmap["reason"]="Timeout: charge did not reach "+to_string(battery_charge_percentage)+"%";
      cout<<"\nBattery charge timeout. Current: "<<current_percentage<<"% Required: "<<battery_charge_percentage<<"%"<<endl;
      break;
    }
    Sleep(3000);
}
      
    }
    fieldidmap["stock_info_id"] = stock_info_id;
    fieldidmap["part_id"] = part_id;
    // fieldidmap["part_config_id"] = part_config_id;
    fieldidmap["item_id"] = item_id;
    // fieldidmap["created_by"] = created_by;
    partresultservice.updateBulkPartResult(paramtrmap, fieldidmap);
    iqcService.ChecksResultSync("Battery charge Check");
  }

  private: int SingleBatteryTest(map<int,map<string,string>>battery_data,string paramters){
    map<string, string> paramtrmap;
    map<string, string> fieldidmap;
    int return_value = 0;
    // string batteries = util.executeTerminal(CMD_BATTERY);
    //CMD_BATTERY="upower -i "+batteries;
    paramtrmap = test(paramters);
    map<int, map<string, string>> ::iterator it = battery_data.begin();
    while(it!=battery_data.end()){
      map<string, string> partdata = it->second;
      string partname=partdata["njhat_item_name"];
      if(partname=="Battery"){
        // if(partdata["attr_01"]=="PORTABLE BATTERY 0" || partdata["attr_01"]=="PORTABLE BATTERY 1"){
          // paramtrmap["Cycle_count"] = partdata["attr_09"];
          // paramtrmap["Health"]=partdata["attr_10"];
          fieldidmap["stock_info_id"] = partdata["stock_info_id"];
          fieldidmap["part_id"] = partdata["stock_part_id"];
          fieldidmap["part_config_id"] = partdata["part_config_id"];
          fieldidmap["item_id"] = partdata["item_id"];
          fieldidmap["created_by"] = partdata["created_by"];
          updateScore("Health",partdata["attr_10"],health_passing_percenatge,fieldidmap);
        
          updateScore("Cycle_count",partdata["attr_09"],passing_cc,fieldidmap);
          return_value = partresultservice.updateBulkPartResult(paramtrmap, fieldidmap);
          iqcService.ChecksResultSync("Battery Check");
          break;
        // }
      }
    ++it;
    }
    cout<<"\nBattery Check Completed..."<<endl;
    return return_value;
  }

  private: int MultipleBatteryTest(map<int,map<string,string>>battery_data,string parameters){
    map<string,string>fieldidmap;
    bool isbatterycharging;
    int return_value=0;
    if(parameters.find("Battery_charging_status")!=string::npos){
    isbatterycharging=charging();}
    
    if(parameters.find("Battery_discharging_status")!=string::npos){
      discharge();}
    map<int, map<string, string>> ::iterator it = battery_data.begin();
    while(it!=battery_data.end()){
      map<string, string> partdata = it->second;
      string partname=partdata["njhat_item_name"];
      // cout<<partname<<endl;
      if(partname=="Battery"){
          fieldidmap["stock_info_id"] = partdata["stock_info_id"];
          fieldidmap["part_id"] = partdata["stock_part_id"];
          fieldidmap["part_config_id"] = partdata["part_config_id"];
          fieldidmap["item_id"] = partdata["item_id"];
          fieldidmap["created_by"] = partdata["created_by"];
          if(parameters.find("Health")!=string::npos){
          updateScore("Health",partdata["attr_10"],health_passing_percenatge,fieldidmap);}
          if(parameters.find("Cycle_count")!=string::npos){
          updateScore("Cycle_count",partdata["attr_09"],passing_cc,fieldidmap);}
          return_value = partresultservice.updateBulkPartResult(paramtrmap_battery1, fieldidmap);
          iqcService.ChecksResultSync("Battery Check");
    }++it;
    }
    cout<<"\nBattery Check Completed..."<<endl;
    return return_value;
  }

  public:  map<string, string> test(string parameters) {
    map<string, string> paramtrmap;
    if(parameters.find("Battery_charging_status")!=string::npos){
    int current_percentage;
    
    std::unique_lock<std::mutex> lock(io_mutex);
    if (!ischarging(cmd_battery1)) {
      cout<<"\033[33m\nAre you sure U inserted the Power cable and Battery port Working.....[y/yes]?\033[0m" <<flush;
      // cout << "\nAre you sure U inserted the Power cable and Battery port Working.....[y/yes]? "<<std::flush;
      // confirm();
      while(true){
       bool adaptor_connected=is_adapter_connected(cmd_power);
       if(!adaptor_connected){
        bool charging_port_working=util.generatePopup("Charging Port Not Conncted","If want to retry connect Adapter then click retry, If charging Port not working click Not working?");
        if(charging_port_working){
          paramtrmap["Battery_charging_status"] = "Charging";
          paramtrmap["Charging_port_status"]="FAIL";
          break;
        }else{
          continue;
        }
       }else{
         paramtrmap["Charging_port_status"]="PASS";
       }
        
       if(adaptor_connected && (ischarging(cmd_battery1)|| Battery_percentage(cmd_battery1)>95)){
          paramtrmap["Battery_charging_status"] = "Charging";
          break;
       }else{
         paramtrmap["Battery_charging_status"] = "Not charging";
         return paramtrmap;
         break;
       }
      }
      //  bool result=util.generatePopup("Battery Not Charging", "Please insert the Power cable then Click Retry. if Battery Not Charging Click Not Working?");
      //  if(!result){
      //    if(ischarging(cmd_battery1)){
      //      break;
      //     }
      //  }else{
      //    paramtrmap["Battery_charging_status"] = "Not charging";
      //    return paramtrmap;
      //    break;
      //   }
      
      // Sleep(2);
      // if (!ischarging(cmd_battery1)) {
      //   paramtrmap["Battery_charging_status"] = "Not charging";
      //   // chargingCompleted = true;
      //   // cv.notify_one();
      //   //Automatic_tab_open();
      //   return paramtrmap;
      // } 
    }
    current_percentage = Battery_percentage(cmd_battery1);
    cout << "\nPlease Wait Battery is charging.........\n"<<std::flush;
    // chargingCompleted = true;
    // cv.notify_one();
    
    this_thread::sleep_for(chrono::minutes(time));
    cout<<"\033[33m\nPlease remove the Power cable and press 'y' or 'Y' to continue. .....[y/yes] \033[0m" <<flush;
    // util.generatePopupOK("Please remove the Power cable then Click OK to continue?");
    // // confirm();
    // Sleep(2000);
    // if (!isdischarging(cmd_battery1)) {
    // cout<<"\033[33m\nAre you sure you removed the Power Cable, Battery is not discharging......[y/yes]\033[0m" <<flush;
    // while(true){
    //   bool result=util.generatePopup("Battery Not Discharging", "Please remove the Power cable then Click Retry. if Battery Not Discharging Click Not Working?");
    //   if(!result){
    //     if(isdischarging(cmd_battery1)){
    //       break;
    //     }
    //    }else{
    //      break;
    //     }
    //   }     
    // }
    paramtrmap["Battery_charging_status"] = "Charging";
    int Battery_percentage_after_charging=Battery_percentage(cmd_battery1);
    int percentage_increased = Battery_percentage_after_charging - current_percentage;
    // if(percentage_increased>=1){
    
    // }else{
    //   paramtrmap["Battery_charging_status"] = "No change in Battery percentage";
    // }
    paramtrmap["Time_charged"] = to_string(time) + " mins";
    paramtrmap["Percentage_increased"] = to_string(percentage_increased);

    }

    if(parameters.find("Battery_discharging_status")!=string::npos){
     int before_discharge=Battery_percentage(cmd_battery1) ;
    if (!isdischarging(cmd_battery1)) {
      while(true){
      bool result=util.generatePopup("Battery Not Discharging", "Please remove the Power cable then Click Retry. if Battery Not Discharging Click Not Working?");
      if(!result){
        if(isdischarging(cmd_battery1)){
          break;
        }
       }else{
         paramtrmap["Battery_discharging_status"] = "Not Discharging";
         return paramtrmap;
         break;
        }
      } 
      // Automatic_tab_open();
    }
    //Automatic_tab_open();

    cout << "\nPlease Wait Battery is Discharging.......\n";
    this_thread::sleep_for(chrono::minutes(time));
    int Battery_percentage_after_dicharging=Battery_percentage(cmd_battery1);
    int percentage_decreased = before_discharge- Battery_percentage_after_dicharging;
    paramtrmap["Battery_discharging_status"] = "Discharging";
    paramtrmap["Time_discharged"] = to_string(time) + " mins";
    paramtrmap["Percentage_decreased"] = to_string(percentage_decreased);
    }
    return paramtrmap;
  }

  public: bool charging(){
    
    int Battery2_percentage_before_charging;
    int Battery1_percentage_before_charging;
    
     std::unique_lock<std::mutex> lock(io_mutex);
    //  cout << "Please insert the Power cable and Press 'y' or 'Y' or 'yes' to Continue.......[y/yes]? " << std::flush;
    //  confirm();
     if(!ischarging(cmd_battery1)&& !ischarging2(cmd_battery1)){
      cout<<"\033[33m\nAre you sure U inserted the Power cable and Battery port Working.....[y/yes]?\033[0m" <<flush;
      // util.generatePopup("Are you sure U inserted the Power cable and Battery port Working?");
      while(true){
      bool adaptor_connected=is_adapter_connected(cmd_power);
      if(!adaptor_connected){
        bool charging_port_working=util.generatePopup("Charging Port Not Conncted","If want to retry connect Adapter then click retry, If charging Port not working click Not working?");
        if(charging_port_working){
          paramtrmap_battery1["Battery_charging_status"] = "Charging";
          paramtrmap_battery2["Battery_charging_status"] = "Charging";
          paramtrmap_battery1["Charging_port_status"]="FAIL";
          return true;
          break;
        }else{
          continue;
        }
       }else{
         paramtrmap_battery1["Charging_port_status"]="PASS";
       }

       if(adaptor_connected &&(ischarging(cmd_battery1) || ischarging2(cmd_battery1))){
          paramtrmap_battery1["Battery_charging_status"] = "Charging";
          paramtrmap_battery2["Battery_charging_status"] = "Charging";
          return true;
       }else{
         paramtrmap_battery1["Battery_charging_status"] = "Not charging";
         paramtrmap_battery2["Battery_charging_status"] = "Not charging";
         return false;
         break;
       }
      }
      //  bool result=util.generatePopup("Battery Not Charging", "Please insert the Power cable then Click Retry. if Battery Not Charging Click Not Working?");
      //  if(!result){
      //    if(ischarging(cmd_battery1) || ischarging2(cmd_battery1)){
      //      break;
      //     }
      //  }else{
      //    paramtrmap_battery1["Battery_charging_status"] = "Not charging";
      //    paramtrmap_battery2["Battery_charging_status"] = "Not charging";
      //    break;
      //   }
      
      // cout << "\nAre you sure U inserted the Power cable and Battery port Working.....[y/yes]? " << std::flush;
     }
    //  chargingCompleted = true;
    //  cv.notify_one();
    
    bool battery2_status=ischarging2(cmd_battery1);
    // cout<<"\nBattery2 charging status:"<<battery2_status << std::flush;
    bool battery1_status=ischarging(cmd_battery1);
    // cout<<"\nBattery1 charging status:"<<battery1_status << std::flush;
    Battery1_percentage_before_charging=Battery_percentage(cmd_battery1);//cout<<"\nbattery1 percentage before charging:"<<Battery1_percentage_before_charging << std::flush;
    Battery2_percentage_before_charging=Battery_percentage2(cmd_battery1);//cout<<"\nbattery2 percentage before charging:"<<Battery2_percentage_before_charging << std::flush;
    
    if(!battery1_status && !battery2_status){
      paramtrmap_battery1["Battery_charging_status"]="Not charging";
      paramtrmap_battery2["Battery_charging_status"]="Not charging";
      // Automatic_tab_open();
      return false;
    }else if(battery1_status){
      
      paramtrmap_battery1["Time_charged"]=to_string(time)+" mins";
      cout<<"\nPlease wait Battery is Charging....\n" << std::flush;
      this_thread::sleep_for(chrono::minutes(time));
      cout<<"\033[33m\nPlease remove the Power cable and press 'y' or 'Y' to continue. .....[y/yes]\033[0m" <<flush;
      // util.generatePopupOK("Please remove the Power cable then press OK to continue?");
      // Sleep(1000);
      // cout << "\nPlease remove the Power cable and press 'y' or 'Y' to continue. .....[y/yes] " << std::flush;
      // confirm();
      //Automatic_tab_open();
      int battery2_percentage_after_charging=Battery_percentage2(cmd_battery1);
      int battery1_percentage_after_charging=Battery_percentage(cmd_battery1);
      // cout<<"\nbattery1 percentage after charging:"<<battery1_percentage_after_charging << std::flush;
      // cout<<"\nbattery2 percentage after charging:"<<battery2_percentage_after_charging<< std::flush;
      if(battery1_percentage_after_charging-Battery1_percentage_before_charging>=1){
        paramtrmap_battery1["Battery_charging_status"]="charging";
        paramtrmap_battery1["Adapter_status"]="Working";
      }else{
        paramtrmap_battery1["Battery_charging_status"]="No change in charging";
        paramtrmap_battery1["Adapter_status"]="Not Working";
      }
      paramtrmap_battery1["Percentage_increased"]=to_string(battery1_percentage_after_charging-Battery1_percentage_before_charging);
      if(battery2_percentage_after_charging==Battery2_percentage_before_charging){
        paramtrmap_battery2["Battery_charging_status"]="Not checked";
      }
      else{
        paramtrmap_battery2["Battery_charging_status"]="Charging";
        paramtrmap_battery2["Percentage_increased"]=to_string(battery2_percentage_after_charging-Battery2_percentage_before_charging);
      }
    }else if(battery2_status){
     
      paramtrmap_battery2["Time_charged"]=to_string(time)+" mins";
      cout<<"\nPlease wait Battery is Charging...." << std::flush;
      this_thread::sleep_for(chrono::minutes(time));
      cout<<"\033[33m\nPlease remove the Power cable and press 'y' or 'Y' to continue. .....[y/yes] \033[0m" <<flush;
      // util.generatePopupOK("Please remove the Power cable then press OK to continue?");
      // Sleep(1000);
      // cout << "\nPlease remove the Power cable and press 'y' or 'Y' to continue. .....[y/yes] " << std::flush;
      // confirm();
      //Automatic_tab_open();
      int battery1_percentage_after_charging=Battery_percentage(cmd_battery1);
      int battery2_percentage_after_charging=Battery_percentage2(cmd_battery1);
      // cout<<"\nbattery1 percentage after charging:"<<battery1_percentage_after_charging << std::flush;
      // cout<<"\nbattery2 percentage after charging:"<<battery2_percentage_after_charging << std::flush;
      if(battery2_percentage_after_charging-Battery2_percentage_before_charging>=1){
        paramtrmap_battery2["Battery_charging_status"]="charging";
        paramtrmap_battery2["Adapter_status"]="Working";
      }else{
        paramtrmap_battery2["Battery_charging_status"]="No change in charging";
        paramtrmap_battery2["Adapter_status"]="Not Working";
      }
      paramtrmap_battery2["Percentage_increased"]=to_string(battery2_percentage_after_charging-Battery2_percentage_before_charging);
      if(battery1_percentage_after_charging==Battery1_percentage_before_charging){
        paramtrmap_battery1["Battery_charging_status"]="Not checked";
      }else{
        paramtrmap_battery1["Battery_charging_status"]="Charging";
        paramtrmap_battery1["Percentage_increased"]=to_string(battery1_percentage_after_charging-Battery1_percentage_before_charging);
      }
    }
    return true;
  }

  private: bool discharge(){
      util.generatePopupOK("Please remove the Power cable then press OK to continue?");
      Sleep(1000);
      if (!isdischarging(cmd_battery1) && !isdischarging2(cmd_battery1)) {
      while(true){
      bool result=util.generatePopup("Battery Not Discharging", "Please remove the Power cable then Click Retry. if Battery Not Discharging Click Not Working?");
      if(!result){
        if(isdischarging(cmd_battery1) || isdischarging2(cmd_battery1)){
          break;
        }
       }else{
         paramtrmap_battery1["Battery_discharge_status"]="Not discharging";
         paramtrmap_battery2["Battery_discharge_status"]="Not discharging";
         return false;
         break;
        }
      } 
      }
    // if(!isdischarging(cmd_battery1) || !isdischarging2(cmd_battery1)){
    // util.generatePopup("Please remove the Power cable and press yes to continue?");}
    int Battery2_percentage_before_discharge=Battery_percentage2(cmd_battery1);
    int Battery1_percentage_before_discharge=Battery_percentage(cmd_battery1);
    // cout<<"\nplease wait battery is discharging 3 min ..." << std::flush;
    // this_thread::sleep_for(chrono::minutes(3));

    bool battery2_status=isdischarging2(cmd_battery1);//cout<<"\nbattery2 discharge status:"<<battery2_status << std::flush;
    bool battery1_status=isdischarging(cmd_battery1);//cout<<"\nbattery1 discharge status:"<<battery1_status << std::flush;

    if(!battery1_status && !battery2_status){
      paramtrmap_battery1["Battery_discharge_status"]="Not discharging";
      paramtrmap_battery2["Battery_discharge_status"]="Not discharging";
      // Automatic_tab_open();
      return false;
    }else if(battery1_status){
      paramtrmap_battery1["Battery_discharge_status"]="discharging";
      paramtrmap_battery1["Time_discharged"]=to_string(time)+" mins";
      cout<<"\nPlease wait Battery is Disharging....\n" << std::flush;
      this_thread::sleep_for(chrono::minutes(time));
      int battery2_percentage_after_discharge=Battery_percentage2(cmd_battery1);//cout<<"\nbattery2 percentage after discharge"<<battery2_percentage_after_discharge << std::flush;
      int batter1_percentage_after_discharge=Battery_percentage(cmd_battery1);//cout<<"\nbattery1 percentahe after discharge:"<<batter1_percentage_after_discharge << std::flush;
      paramtrmap_battery1["percentage_decreased"]=to_string(Battery1_percentage_before_discharge-batter1_percentage_after_discharge);
      if(Battery2_percentage_before_discharge==battery2_percentage_after_discharge){
        paramtrmap_battery2["Battery_discharge_status"]="Not Checked";
      }else{
        paramtrmap_battery2["Battery_discharge_status"]="Discharging";
        paramtrmap_battery2["Percentage_decreased"]=to_string(Battery2_percentage_before_discharge-battery2_percentage_after_discharge);
      }
    }else if(battery2_status){
      paramtrmap_battery2["Battery_discharging_status"]="discharging";
      paramtrmap_battery2["Time_discharged"]=to_string(time)+" mins";
      cout<<"\nPlease wait Battery is Disharging....\n" << std::flush;
      this_thread::sleep_for(chrono::minutes(time));
      int battery2_percentage_after_discharge=Battery_percentage2(cmd_battery1);//cout<<"\nbattery2 percentage after discharge"<<battery2_percentage_after_discharge << std::flush;
      int battery1_percentage_after_discharge=Battery_percentage(cmd_battery1);//cout<<"\nbattery1 percentahe after discharge:"<<battery1_percentage_after_discharge << std::flush;
      paramtrmap_battery2["Percentage_decreased"]=to_string(Battery2_percentage_before_discharge-battery2_percentage_after_discharge);
      if(Battery1_percentage_before_discharge==battery1_percentage_after_discharge){
        paramtrmap_battery1["Battery_discharge_status"]="Not checked";
      }else{
        paramtrmap_battery1["Battery_discharge_status"]="Discharging";
        paramtrmap_battery1["percentage_decreased"]=to_string(Battery1_percentage_before_discharge-battery1_percentage_after_discharge);
      }
    }
    return true;
  }

  public: bool ischarging(string cmd_battery){
    json jsondata = getTestResult(cmd_battery);
    // cout<<jsondata.dump(2)<<endl;
    // cout<<"\nCharging Battery Status:"<<jsondata[0]["BatteryStatus"]<<endl;
    if(jsondata.size()!=0){
      if (jsondata[0]["BatteryStatus"] == "2") {
        return true;
      }else{
        return false;
      }
    }
    return true;
  }

  public: bool isdischarging(string cmd_battery){
    json jsondata = getTestResult(cmd_battery);
    try
    {
      if (jsondata[0]["BatteryStatus"] == "2") {
        return false;
      }
    }
    catch(const std::exception& e)
    {
      std::cerr <<"Error While getting Battery discharging status:"<< e.what() << '\n';
    }
    return true;
  }

  public: int Battery_percentage(string cmd_battery){
     int current_percentage=0;
     string percentage;
    try{
    json jsondata = getTestResult(cmd_battery);
	  current_percentage = stoi(jsondata[0]["EstimatedChargeRemaining"].get<string>());
    }catch(const std::invalid_argument& e) {
      cout<<"\n Error While getting Battery percentage: "<<e.what() <<percentage<<endl;
    }catch(const std::exception& e) {
      cout<<"\n Error While getting Battery percentage: "<<e.what() <<percentage<<endl;
    }
    return current_percentage;
  }

public: bool ischarging2(string cmd_battery) {
    json jsondata = getTestResult(cmd_battery);
    try{
      if (jsondata[1]["BatteryStatus"] == "2") {
        return true;
      }
    }catch(const std::exception& e) {
      cout<<"\n Error While getting Battery 2 charging status: "<<e.what() <<endl;
    }
    return false;
}

  public: bool isdischarging2(string cmd_battery) {
      json jsondata = getTestResult(cmd_battery);
      try{
      if(jsondata.size()>1){
      if (jsondata[1]["BatteryStatus"] == "2") {
          return false;
      }else{
        return true;
      }
      }
    }catch(const std::exception& e) {
      cout<<"\n Error While getting Battery 2 discharging status: "<<e.what() <<endl;
    }
    return false;
  }
    public: bool is_adapter_connected(string cmd_power) {
      json jsondata = getTestResult(cmd_power);
      if (jsondata.size()>0){
        for(int i=0;i<jsondata.size();i++){
          if (jsondata[i]["PowerOnline"] == "True") {
            return true;
          }
        }
      }else{
        return true;
      }      
      return false;
  }

  private: int Battery_percentage2(string cmd_battery) {
    int current_percentage=0;
    string percentage;
    try{
      json jsondata = getTestResult(cmd_battery);
      if(jsondata.size() < 2) return 0;  // single battery — no Battery 2
      percentage=jsondata[1]["EstimatedChargeRemaining"].get<string>();
      current_percentage = stoi(percentage);
    }catch (const std::invalid_argument& e) {
      cout<<"\n Error While getting Battery 2 percentage: "<<e.what() <<percentage<< endl;
    }catch(const std::exception& e) {
      std::cerr << "Error: While getting Battery 2 percentage :" << e.what() << percentage<<std::endl;      
    }
    return current_percentage;
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
  
  public: void joindischargeThread(){
    if(dischargeThread.joinable()){
      cout<<"\nJoing Battery thread results..."<<endl;
      dischargeThread.join();
    }
  }

  public:void updateDepartmentTime(string department,json result){
    // json result=njsetting.getNjSettingsvalues();
    for(const auto&item: result){
      if(item["department"]==department){
        if (item["parameter"]=="BatteryTime"){
          time=stoi(item["value"].get<string>());
        }else if(item["parameter"]=="health_passing_percentage"){
          health_passing_percenatge=stoi(item["value"].get<string>());
        }else if(item["parameter"]=="passing_cc"){
          passing_cc=stoi(item["value"].get<string>());
        }             
        else if(item["parameter"]=="passing_battery_charge_percentage"){
          battery_charge_percentage=stoi(item["value"].get<string>());
        }
    }
  }
  }
  public : void updateScore(string param,string healthscore,int conditionValue,map<string,string>fieldMap){
    
    fieldMap["param"]=param;
    fieldMap["result"]=healthscore;
    
    try{
      if(param=="Health"){
      if(stoi(healthscore)>=conditionValue){
       fieldMap["score"]="0";
    }else if(stoi(healthscore)>=50){
      fieldMap["score"]="1";
    }else{
      fieldMap["score"]="41";
    }
  }
    else{
      if(stoi(healthscore)<conditionValue){
       fieldMap["score"]="0";
    }else{
      fieldMap["score"]="41";
    }
    }}catch(const std::exception& e) {
      cout<<"\n Error While calculating Score: "<<e.what()<<healthscore<<endl;
    }
    partresultservice.updatePartResult(fieldMap);
  } 
  // private:void Automatic_tab_open(){

  //   system("nohup xdg-open https://erp.newjaisa.com/ > /dev/null 2>&1 &");
  //   system("nohup xdg-open https://njpulse.newjaisa.com/ > /dev/null 2>&1 &");
    
  // }
  public:    bool runBatteryTest(string result){
        std::vector<std::string>batteryCheckInfo;
        batteryCheckInfo.push_back("Battery_discharging_status");
        batteryCheckInfo.push_back("percentage_decreased");
        batteryCheckInfo.push_back("Health");
        batteryCheckInfo.push_back("Cycle_count");
        batteryCheckInfo.push_back("Percentage_increased");
        batteryCheckInfo.push_back("Full_BDT_criteria");
        batteryCheckInfo.push_back("Battery_charging_status");
        batteryCheckInfo.push_back("Charging_port_status");

                                   
                                    
        for(const std::string& bluetoothcheck:batteryCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }

};

int BatteryCheck::time = 1;
int BatteryCheck::health_passing_percenatge = 80;
int BatteryCheck::passing_cc = 600;
int BatteryCheck::battery_charge_percentage=60;