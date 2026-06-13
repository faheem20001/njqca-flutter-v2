#include <iostream>
#include <string>

using namespace std;

class DriverCheck: public V2Service {

    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;
    public :
    static string exclude_driver;

    public:void drivertest(string stockinfoID,string partId, string item_id,string created_by){
        map<string,string> parametrmap;
        map<string,string> fieldmap; 
        cout<<"\nDriver Check Running..."<<endl;
        string drivercommand="powershell -Command \"Get-PnpDevice | Where-Object { $_.Status -eq 'Error' } | Select-Object -Property ConfigManagerErrorCode, DeviceID, PNPClass,Caption,Present | Format-List\"";
        string result=util.executeTerminal(drivercommand);
        std::istringstream inputstream(result);
        json ErrorDrivers=util.ExcetuteTerminalJSonArray1(inputstream);
        // if(ErrorDrivers.size()==0){
        //   parametrmap["Driver_Check_Status"]="PASS";
        // }else{
        //   parametrmap["Driver_Check_Status"]="FAIL";
        // }
        int error_count=0;
        parametrmap["Error_Drivers_Count"]=to_string(ErrorDrivers.size());  
        for(int i=0;i<ErrorDrivers.size();i++){
          if(exclude_driver.find(ErrorDrivers[i]["DeviceID"])==std::string::npos){
            error_count+=1;
          }
          parametrmap["DeviceID"+to_string(i)]=ErrorDrivers[i]["DeviceID"];
          parametrmap["PNPClass"+to_string(i)]=ErrorDrivers[i]["PNPClass"];
          parametrmap["Caption"+to_string(i)]=ErrorDrivers[i]["Caption"];
          parametrmap["ErrorCode"+to_string(i)]=ErrorDrivers[i]["ConfigManagerErrorCode"];
          parametrmap["DriverPresent"+to_string(i)]=ErrorDrivers[i]["Present"];          
        }
          if(error_count==0){
          parametrmap["Driver_Check_Status"]="PASS";
        }else{
          parametrmap["Driver_Check_Status"]="FAIL";
        }
        fieldmap["stock_info_id"]=stockinfoID;
        fieldmap["part_id"]=partId;
        // fieldmap["part_config_id"]=partconfigid;
        fieldmap["item_id"]=item_id;
        fieldmap["created_by"]=created_by;
        partresultservice.updateBulkPartResult(parametrmap,fieldmap);
        iqcservice.ChecksResultSync("Driver Check");
        cout<<"\nDriver Check Completed..."<<endl;
    }
      public:  bool runDriverTest(string result){
        std::vector<std::string>driverCheckInfo;
        driverCheckInfo.push_back("Driver_Check_Status");
        driverCheckInfo.push_back("Error_Drivers_Count");

        for(const std::string& bluetoothcheck:driverCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
    public:void update_exclude_drivers(string department,json result){
      // json result=njsettings.getNjSettingsvalues();
      for(const auto&item: result){
     
        if(item["department"]==department ){
          if(item["parameter"]=="exclude_driver")
          exclude_driver=item["value"];
      
        }
        
      }
      
    }

};
string DriverCheck::exclude_driver;