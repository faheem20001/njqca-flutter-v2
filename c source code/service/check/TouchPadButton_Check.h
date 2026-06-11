#include <iostream>
#include <string>

using namespace std;

class TouchPadButtonCheck: public V2Service {

    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;
    BatteryCheck battery;
    public:
    static bool TouchPad_Button_available;

    public:void TouchPadButton_test(string stockinfoID,string partId, string item_id){
        map<string,string> parametrmap;
        map<string,string> fieldmap; 
        bool initial_status_enabled=false;
        string touchpad_button_command="powershell -Command \"Get-ChildItem -Path \"HKCU:\\Software\\Synaptics\" -Recurse | Get-ItemProperty | Where-Object {$_.PsPath -like \"*TouchPad*\"} | Select-Object -ExpandProperty DisableDevice\"";
        cout<<"\nTouchPad Button Check Running..."<<endl;
        string button_status_result=util.executeTerminal(touchpad_button_command);
        if(button_status_result.find("cannot be found")!=std::string::npos){
            util.generatePopupOK("TouchPad Button is not detected,Please enable and disable the Touchpad through the Button and then click OK?");
            Sleep(1000);
            button_status_result=util.executeTerminal(touchpad_button_command);
        }
        
        if(button_status_result.find("0")!=std::string::npos){
            initial_status_enabled=true;
        }
        if(initial_status_enabled){
            util.generatePopupOK("Please Disable the TouchPad through the Button and then click OK?");  
        }else{
            util.generatePopupOK("Please Enable the TouchPad through the Button and then click OK?");
        }

        button_status_result=util.executeTerminal(touchpad_button_command);
        if(button_status_result.find("1")!=std::string::npos && initial_status_enabled){
            parametrmap["TouchPad_DisableButton_Status"]="Working";
        }else{
            parametrmap["TouchPad_DisableButton_Status"]="Not Working";
        }
        

        fieldmap["stock_info_id"]=stockinfoID;
        fieldmap["part_id"]=partId;
        // fieldmap["part_config_id"]=partconfigid;
        fieldmap["item_id"]=item_id;
        // fieldmap["created_by"]=created_by;
        partresultservice.updateBulkPartResult(parametrmap,fieldmap);
        iqcservice.ChecksResultSync("TypeC Check");
        cout<<"\nTouchPad Button Check Completed..."<<endl;
    }
      public:  bool runTouchPad_check(string result){
        std::vector<std::string>driverCheckInfo;
        driverCheckInfo.push_back("TouchPad_DisableButton_Status");

        for(const std::string& bluetoothcheck:driverCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
};
bool TouchPadButtonCheck::TouchPad_Button_available = false;