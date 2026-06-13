#include <iostream>
#include <string>
#include <map>
#include <chrono>
#include <thread>
#include <memory>
#include <stdexcept>

#pragma once
#include <../lib/freshweb/emit.h>

using namespace std;

class LidCheck : public V2Service {

    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;
    string led_status = "powershell -Command \"Get-CimInstance -Namespace root\\wmi -ClassName WmiMonitorBasicDisplayParams |Select-Object Active";


public:
    int Lid_test(string stockinfoID, string partId,  string item_id) {
        nj_emit({{"event","test_start"},{"test","LID_status"},{"label","Lid Test"},{"part","Motherboard"}});
        int return_value = 0;
        map<string, string> parametrmap;
        map<string, string> fieldmap; 

        string setDonothinginDC="powercfg -setdcvalueindex SCHEME_CURRENT SUB_BUTTONS lidAction 0";
        string setDonothinginAC="powercfg -setacvalueindex SCHEME_CURRENT SUB_BUTTONS lidAction 0";
        string applysettings="powercfg -SetActive SCHEME_CURRENT";
        // string initial_stage = util.executeTerminal(led_status);
        system(setDonothinginDC.c_str());
        Sleep(1000);
        system(setDonothinginAC.c_str());
        Sleep(1000);
        system(applysettings.c_str());
        Sleep(1000);
        // Output the initial status
        // cout << "\nInitial lid status: " << (initial_stage.empty() ? "Closed" : "Open") << endl;
        // Flutter handles the HDMI removal instruction via the interactive screen
        // (The interactive_needed emit below tells Flutter to show the full lid test screen)
        
        // Emit interactive_needed — Flutter shows lid test screen with instructions
        nj_emit({{"event","interactive_needed"},{"test","LID_status"},
                  {"label","Lid Test"},
                  {"instruction","Remove any HDMI cable, then close the laptop lid"}});
        bool lid_passed = nj_poll_ack();
        parametrmap["LID_status"] = lid_passed ? "Working" : "Not Working";
       


        fieldmap["stock_info_id"] = stockinfoID;
        fieldmap["part_id"] = partId;
        // fieldmap["part_config_id"] = partconfigid;
        fieldmap["item_id"] = item_id;
        // fieldmap["created_by"] = created_by;
        return_value = partresultservice.updateBulkPartResult(parametrmap, fieldmap);
        iqcservice.ChecksResultSync("Lid Check");
        nj_emit({{"event","test_done"},{"test","LID_status"},
                  {"status",parametrmap["LID_status"]=="Working" ? "pass" : "fail"},
                  {"score",parametrmap["LID_status"]=="Working" ? 100 : 0}});
        return return_value;
    }
    public:    bool runLIDTest(string result){
        std::vector<std::string>LIDCheckInfo;
        LIDCheckInfo.push_back("LID_status");
        // LIDCheckInfo.push_back("HDMI_Status");

        // driverCheckInfo.push_back("Error_Drivers_Count");

        for(const std::string& bluetoothcheck:LIDCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
    bool check_hdmi_conncted(){
        std::string command = "powershell.exe -Command \"Get-CimInstance -Namespace root\\wmi -ClassName WmiMonitorConnectionParams | Select-Object  VideoOutputTechnology\"";
       
        string status = util.executeTerminal(command);                
        std::istringstream deviceStream(status);
        string port;
        while (std::getline(deviceStream, port)) {
            port.erase(std::remove(port.begin(), port.end(), ' '), port.end());
            //cout << port << "numbrergterbehd" << endl;
            if (port=="5") {
                return true; 
            }
            
        }
        return false;
    }

    bool lid_closed(){
       bool runloop = true;
       util.generatePopupOK("Please Click Ok, then close the LID");
       auto start = std::chrono::steady_clock::now();
       while (runloop) {
           
            string current_stage = util.executeTerminal(led_status);
            // cout<<current_stage<<endl;          
            if (current_stage.find("True")==std::string::npos) {
                return true;
                // runloop = false;
                break;
            }
            // Check for timeout of 30 seconds
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
            if (duration.count() > 30) {
                runloop = false;
            }
            
        }
        return false;
    }
};
