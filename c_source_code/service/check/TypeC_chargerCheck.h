#include <iostream>
#include <string>

using namespace std;

class TypeC_chargerCheck: public V2Service {

    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;
    BatteryCheck battery;
    HDDSentinelCheck hdd;
    BatteryCheck batterycheck;
    public:
    static bool tunderbolt_available;
    static int no_of_typeC_port;
    const string cmd_battery = "powershell -Command \"Get-WmiObject -Class Win32_Battery | Format-List BatteryStatus,EstimatedChargeRemaining\""; 
    map<string,string> parametrmap;
    


    public:void TypeC_test(string stockinfoID,string partId, string item_id){
        map<string,string> fieldmap; 
        cout<<"\nTypeC usb Check Running..."<<endl;
        cout<<"\nNo of Type C Available:"<<no_of_typeC_port<<endl;
        if(!tunderbolt_available && no_of_typeC_port==0){
          parametrmap["TypeC_Port_Status"] = "Not Available";
        }else{
          bool result=is_type_c_working();
          if(result){
            parametrmap["TypeC_Port_Status"] = "Working";
          }
          while(!result){
              bool status=util.generatePopup("Type C Port Not Working","Please connect  Type C ports then click retry else click Not Working?");
              if(status){
                parametrmap["TypeC_Port_Status"] = "Not Working";
                break;
              }else{
                result=is_type_c_working();
                if(result){
                  parametrmap["TypeC_Port_Status"] = "Working";
                  break;
                }
              }
              
          }
        }
        fieldmap["stock_info_id"]=stockinfoID;
        fieldmap["part_id"]=partId;
        // fieldmap["part_config_id"]=partconfigid;
        fieldmap["item_id"]=item_id;
        // fieldmap["created_by"]=created_by;
        partresultservice.updateBulkPartResult(parametrmap,fieldmap);
        iqcservice.ChecksResultSync("TypeC usb Check");
        cout<<"\nTypeC charger Check Completed..."<<endl;
    }
      public:  bool runTypeC_check(string result){
        std::vector<std::string>driverCheckInfo;
        driverCheckInfo.push_back("TypeC_Port_Status");

        for(const std::string& bluetoothcheck:driverCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }

    public:int no_of_type_c(string file_name, bool msg=false){
      int no_of_typec=0;
      string path_command="powershell -Command \"Get-WmiObject -Class Win32_logicaldisk | Select-Object -ExpandProperty DeviceID\"";
      string check_bit_locker="manage-bde -status";
      string password="NJ-production@1234";
      int no_of_encrypted=0;
      
      bool bit_locker_enabled=false;
      string result=util.executeTerminal(path_command);
      std::istringstream paths(result); 
      std::string drive;
      bool encrypted_found=false;
      while(std::getline(paths,drive)){
        if(drive.find("C:")!=std::string::npos){
          continue;
        }
        string is_encrypted=util.executeTerminal(check_bit_locker+drive);
        // cout<<is_encrypted<<endl;
        if(is_encrypted.find("Unlocked") == std::string::npos){
           encrypted_found=true;
           std::string unlock = "powershell -Command \"$p = ConvertTo-SecureString '" + password +"' -AsPlainText -Force; Unlock-BitLocker -MountPoint '" + drive +"' -Password $p\"";
          //  cout<<unlock<<endl;
           util.executeTerminal(unlock);
           Sleep(2000);
           bit_locker_enabled=true;
           no_of_encrypted+=1;
        }

        string file_path=drive+"\\"+file_name;
        if(hdd.file_exist(file_path)){
          if(bit_locker_enabled){
            std::string lock = "powershell -Command \"Lock-BitLocker -MountPoint '" + drive + "'\"";
            util.executeTerminal(lock);
            Sleep(2000);
            bit_locker_enabled=false;
          }
          no_of_typec+=1;
        }
      }
      // if(!encrypted_found && msg){
      //   util.generatePopupOK("No pendrive is encrypted, file might not be available?");
      // }
      parametrmap["no_of_encrypted_drive"]=std::to_string(no_of_encrypted);
      return no_of_typec;
    }

  public: bool is_type_c_working(){
    // if((no_of_typeC_port==2 && !tunderbolt_available) || (no_of_typeC_port==1 && tunderbolt_available)){
    //   util.generatePopupOK("Please connect 2 Type C ports with Charger and pendrive then click OK?");
    //   if((batterycheck.ischarging(cmd_battery) || batterycheck.ischarging2(cmd_battery)) && is_file_exist("NJQCA_Type_C_Detection_File.txt")){
    //     return true;
    //   }else{
    //     return false;
    //   }
    // }else if(no_of_typeC_port==1 && !tunderbolt_available){
    //   util.generatePopupOK("Please connect  Type C port with Charger then click OK?");
    //   if(batterycheck.ischarging(cmd_battery) || batterycheck.ischarging2(cmd_battery)|| is_file_exist("NJQCA_Type_C_Detection_File.txt")){
    //     return true;
    //   }else{
    //     return false;
    //   }
    // }else if(tunderbolt_available){
    //   util.generatePopupOK("Please connect  Type C port with Pendrive then click OK?");
    //   if(is_file_exist("NJQCA_Type_C_Detection_File.txt")){
    //     return true;
    //   }else{
    //     return false;
    //   }
    // }

      
      int result=no_of_type_c("NJQCA_Type_C_Detection_File.txt", true);
      if(no_of_typeC_port==1 && result==1){
        return true;
      }
      else if((no_of_typeC_port!=1) && (no_of_typeC_port==result || (result=no_of_typeC_port-1 &&(batterycheck.ischarging(cmd_battery) || batterycheck.ischarging2(cmd_battery)) ))){
        return true;
      }

    return false;
  }

    // public:int no_of_type_c(string file_name){
    //   int no_of_typec=0;
    //   string path_command="powershell -Command \"Get-WmiObject -Class Win32_logicaldisk | Select-Object -ExpandProperty DeviceID\"";
    //   string result=util.executeTerminal(path_command);
    //   std::istringstream paths(result); 
    //   std::string path;
    //   while(std::getline(paths,path)){
    //     if(path.find("C:")!=std::string::npos){
    //       continue;
    //     }
    //     string file_path=path+"\\"+file_name;
    //     if(hdd.file_exist(file_path)){
    //       no_of_typec+=1;
    //     }
    //   }
    //   return no_of_typec;
    // }

};
bool TypeC_chargerCheck::tunderbolt_available = false;
int TypeC_chargerCheck::no_of_typeC_port = 0;