#include <iostream>
#include <../lib/freshweb/emit.h>
#include <string>
#include <map>

using namespace std;

class HDMI_VGA_PortCheck : public V2Service {
    IQCService iqcservice;
    Util util; 
    PartResultService partresultservice;
    std::string sd_card_command = "powershell.exe -Command \"Get-WmiObject Win32_DiskDrive | Where-Object { $_.Size -lt 9663676416 }| Select-Object Caption\"";

 public:
    static bool VGA_available; 
    static bool HDMI_available; 
    static bool DP_available; 
    static bool sd_card_available;

public:
    int HDMI_VGA_PortTest(string stockinfoID, string partId, string partconfigid, string item_id, string created_by,string parameters) {
        nj_emit({{"event","test_start"},{"test","HDMI_Status"},{"label","HDMI / VGA Test"},{"part","Motherboard"}});
        // Emit interactive_needed — Flutter shows display connection screen
        nj_emit({{"event","interactive_needed"},{"test","HDMI_Status"},
                  {"label","HDMI / VGA Test"},
                  {"instruction","Connect HDMI and VGA cables to external displays, then click Done"}});
        bool hdmi_ack = nj_poll_ack();
        if (!hdmi_ack) {
            // User marked as not working — skip automatic detection
        }
        cout << "\nHDMI and VGA checking..." << endl;
        int display_count=1;
        
        int return_value = 0;
        map<string, string> parametrmap;
        map<string, string> fieldmap;
        string hdmiStatus="";
        string vgaStatus="" ;
        string dpStatus ="";
        string sd_card_Status="";
        std::string display_count_command = "powershell.exe -Command \"(Get-CimInstance -Namespace root\\wmi -ClassName WmiMonitorConnectionParams | Measure-Object).Count\"";
        string msg="";
        vector<string> display_names;
        if(parameters.find("VGA_Status")!=string::npos || parameters.find("HDMI_Status")!=string::npos){
            display_names.push_back("Laptop Display");
            if(VGA_available && HDMI_available){
                display_count+=2;
                msg= to_string(display_count)+" Active Displays Needed (Laptop Display, VGA, HDMI)";
                display_names.push_back("VGA");
                display_names.push_back("HDMI");
            }else if(VGA_available){
                display_count+=1;
                msg= to_string(display_count)+" Active Displays Needed (Laptop Display, VGA)";
                display_names.push_back("VGA");
            }else if(HDMI_available){
                display_count+=1;
                msg= to_string(display_count)+" Active Displays Needed (Laptop Display, HDMI)";
                display_names.push_back("HDMI");
            }
            // Execute the command and capture the output
            string status = util.executeTerminal(display_count_command);
            cout<<"Display Count: "<<display_count<<endl;
            int temp_count=display_count;
            int detected_display_count=stoi(status);

            if(detected_display_count < display_count){
                         {
                    nlohmann::json _e;
                    _e["event"] = "test_progress";
                    _e["test"]  = "HDMI_Status";
                    _e["msg"]   = "Not all displays detected. Found: " + to_string(detected_display_count) + ". Please connect all displays.";
                    nj_emit(_e);
                }
                Sleep(2000); // brief pause before re-check
                while(true){
                    hdmiStatus="";
                    vgaStatus ="";
                    display_count=temp_count;
                    status = util.executeTerminal(display_count_command);
                    detected_display_count=stoi(status);
                    cout<<"detected_display_count: "<<detected_display_count<<endl;
                    if(detected_display_count < display_count){
                        string not_working_displays=util.multi_selection_option(display_names);
                        // cout<<"Not Working Displays: "<<not_working_displays<<endl;
                        if(not_working_displays.empty()){
                            continue;
                        }
                        if(not_working_displays.find("VGA")!=string::npos){
                            display_count-=1;
                            vgaStatus = "Not Working";
                        }
                        if(not_working_displays.find("HDMI")!=string::npos){
                            display_count-=1;
                            hdmiStatus = "Not Working";
                        }
                        if(not_working_displays.find("Laptop Display")!=string::npos){
                            display_count-=1;
                        }
                        // cout<<"display_count: "<<display_count<< "and detected_display_count: "<<detected_display_count<<endl;
                        if(display_count==detected_display_count){
                            if(parameters.find("VGA_Status")!=string::npos && !VGA_available){
                                parametrmap["VGA_Status"] = "Not Available";
                            }else if(parameters.find("VGA_Status")!=string::npos && VGA_available && !vgaStatus.empty()){
                                parametrmap["VGA_Status"] = "Not Working";
                            }else{
                                parametrmap["VGA_Status"] = "Working";
                            }

                            if(parameters.find("HDMI_Status")!=string::npos && !HDMI_available){
                                parametrmap["HDMI_Status"] = "Not Available";
                            }else if(parameters.find("HDMI_Status")!=string::npos && HDMI_available && !hdmiStatus.empty()){
                                parametrmap["HDMI_Status"] = "Not Working";
                            }else{
                                parametrmap["HDMI_Status"] = "Working";
                            }
                            break;
                        }else{
                            nj_emit({{"event","test_progress"},{"test","HDMI_Status"},{"msg","Display selection mismatch. Rechecking..."}});
                        }
                        
                        
                    }else{
                        if(parameters.find("VGA_Status")!=string::npos && !VGA_available){
                                parametrmap["VGA_Status"] = "Not Available";
                        }else if(parameters.find("VGA_Status")!=string::npos && VGA_available && !vgaStatus.empty()){
                                parametrmap["VGA_Status"] = "Not Working";
                        }else{
                                parametrmap["VGA_Status"] = "Working";
                            }

                        if(parameters.find("HDMI_Status")!=string::npos && !HDMI_available){
                                parametrmap["HDMI_Status"] = "Not Available";
                        }else if(parameters.find("HDMI_Status")!=string::npos && HDMI_available && !hdmiStatus.empty()){
                                parametrmap["HDMI_Status"] = "Not Working";
                        }else{
                                parametrmap["HDMI_Status"] = "Working";
                            }
                        break;
                    }
                }
            }else{
                if(parameters.find("VGA_Status")!=string::npos && VGA_available){
                    parametrmap["VGA_Status"] = "Working";
                }else{
                    parametrmap["VGA_Status"] = "Not Available";
                }
                if(parameters.find("HDMI_Status")!=string::npos && HDMI_available){
                    parametrmap["HDMI_Status"] = "Working";
                }else{
                    parametrmap["HDMI_Status"] = "Not Available";
                }
            }
        }

       //cout << "Status Output: " << status << endl;


        // std::istringstream deviceStream(status);
        // string port;
        // while (std::getline(deviceStream, port)) {
        //     port.erase(std::remove(port.begin(), port.end(), ' '), port.end());
        //     cout << port<< endl;
        //     if ((port=="5" && HDMI_available) || (port=="10" && !VGA_available) || (port == "4" && HDMI_available) ) {
        //         hdmiStatus = "Working"; 
        //     }
        //     else if ((port == "10" ||port=="0") && VGA_available) {
        //         vgaStatus = "Working";

        //     }else if (port == "5" && DP_available) {
        //         dpStatus = "Working";
        //     }
            
        // }
        

        // if(parameters.find("VGA_Status")!=string::npos){
        //     if(!VGA_available){
        //       vgaStatus = "Not Available"; 
        //     }
        //     if(vgaStatus.empty()){
        //       vgaStatus="Not Working";
        //     }

        //     if(vgaStatus=="Not Working"){
        //     while(true){
        //         vga_retry_count++;
        //        if(check_VGA_conncted()){
        //            vgaStatus="Working";
        //            break;
        //         }else{
        //             bool status=util.generatePopup("VGA Not Detected","Please connect VGA then click Retry else Click Not Working?");
        //             if(status){
        //                 break;
        //             }
        //         }
        //         if(vga_retry_count>1){
        //           bool run=  util.generatePopupYes("Is VGA Connected and Working?");
        //           if(run){
        //             bool laptop_display=util.generatePopupYes("Is Laptop Display Available and Working(Display Visible)?");
        //             if(!laptop_display){
        //               vgaStatus="Working";
        //               break;
        //             }
        //           }
        //         }    
        //     }
        //    }
        // }

        // if(parameters.find("HDMI_Status")!=string::npos){
        //     if(!HDMI_available){
        //       hdmiStatus = "Not Available"; 
        //     }
        //     if(hdmiStatus.empty()){
        //       hdmiStatus="Not Working";
        //     }
        //     if(hdmiStatus=="Not Working"){
        //     while(true){
        //         hdmi_retry_count++;
        //        if(check_hdmi_conncted()){
        //            hdmiStatus="Working";
        //            break;
        //         }else{
        //             bool status=util.generatePopup("HDMI Not Detected","Please connect HDMI cable then click Retry else Click Not Working?");
        //             if(status){
        //                 break;
        //             }
        //         }
        //         if(hdmi_retry_count>1){
        //           bool run=  util.generatePopupYes("Is HDMI Connected and Working?");
        //           if(run){
        //             bool laptop_display=util.generatePopupYes("Is Laptop Display Available and Working(Display Visible)?");
        //             if(!laptop_display){
        //               hdmiStatus="Working";
        //               break;
        //             }
        //           }
        //         }  
        //     }
        //     }
        // }

        // if(parameters.find("DP_Status")!=string::npos){
        //     if(!DP_available){
        //       dpStatus = "Not Available"; 
        //     }
        //     if(dpStatus.empty()){
        //       dpStatus="Not Working";
        //     }
        //     if(dpStatus=="Not Working"){
        //     while(true){
        //        if(check_hdmi_conncted()){
        //            dpStatus="Working";
        //            break;
        //         }else{
        //             bool status=util.generatePopup("DP Port Not Detected","Please connect DP then click Retry else Click Not Working?");
        //             if(status){
        //                 break;
        //             }
        //         }  
        //     }
        //     }
        // }


        if(parameters.find("SD_card_Status")!=string::npos){
            
            if(!sd_card_available){
                sd_card_Status = "Not Available"; 
            }

            string sd_card_result = util.executeTerminal(sd_card_command);
            if(!sd_card_result.empty() && sd_card_available){
                sd_card_Status = "Working";
            }

            if(sd_card_Status.empty()){
                sd_card_Status="Not Working";
            }

            if(sd_card_Status=="Not Working"){
                while(true){
                if(check_sd_card_conncted()){
                    sd_card_Status="Working";
                    break;
                    }else{
                        bool status=util.generatePopup("SD Card Not Detected","Please connect SD Card then click Retry else Click Not Working?");
                        if(status){
                            break;
                        }
                    }  
                }
            }

        }


        // Store statuses in the parameter map
        // parametrmap["HDMI_Status"] = hdmiStatus; // Store HDMI status
        // parametrmap["VGA_Status"] = vgaStatus;
        // parametrmap["DP_Port_Status"] = dpStatus; 
        if (!sd_card_Status.empty()) parametrmap["SD_card_Status"] = sd_card_Status;
         // Store VGA status

        // Prepare fieldmap for the update
        fieldmap["stock_info_id"] = stockinfoID;
        fieldmap["part_id"] = partId;
        fieldmap["part_config_id"] = partconfigid;
        fieldmap["item_id"] = item_id;
        fieldmap["created_by"] = created_by;

        // Update the part result with HDMI and VGA status
        return_value = partresultservice.updateBulkPartResult(parametrmap, fieldmap);

        // Sync the check result with IQC service
        iqcservice.ChecksResultSync("HDMI and VGA Check");
        nj_emit({{"event","test_done"},{"test","HDMI_Status"},{"status","done"}});
        return return_value;
    }
        public:    bool runHDMITest(string result){
        std::vector<std::string>HDMICheckInfo;
        HDMICheckInfo.push_back("VGA_Status");
        HDMICheckInfo.push_back("HDMI_Status");
        HDMICheckInfo.push_back("DP_Port_Status");
        HDMICheckInfo.push_back("SD_card_Status");


        // driverCheckInfo.push_back("Error_Drivers_Count");

        for(const std::string& bluetoothcheck:HDMICheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
    // bool check_hdmi_conncted(){
    //     std::string command = "powershell.exe -Command \"Get-CimInstance -Namespace root\\wmi -ClassName WmiMonitorConnectionParams | Select-Object  VideoOutputTechnology\"";
    //     string status = util.executeTerminal(command);                
    //     std::istringstream deviceStream(status);
    //     string port;
    //     while (std::getline(deviceStream, port)) {
    //         port.erase(std::remove(port.begin(), port.end(), ' '), port.end());
    //         //cout << port << "numbrergterbehd" << endl;
    //         if ((port=="5" && HDMI_available) || (port=="10" && !VGA_available) || (port=="4" && HDMI_available)) {
    //             return true;
    //         }
            
    //     }
    //     if(HDMI_available && VGA_available){
    //         util.generatePopupOK("Please connect both HDMI and VGA then click OK?");
    //         int dp_count=0;
    //         while(std::getline(deviceStream, port)){
    //             if(port=="10"){
    //                 dp_count++;
    //             }
    //         }

    //         if (dp_count>1){
    //             return true;
    //         }

    //     }
    //     return false;
    // }
    // bool check_VGA_conncted(){
    //     std::string command = "powershell.exe -Command \"Get-CimInstance -Namespace root\\wmi -ClassName WmiMonitorConnectionParams | Select-Object  VideoOutputTechnology\"";
    //     string status = util.executeTerminal(command);                
    //     std::istringstream deviceStream(status);
    //     string port;
    //     while (std::getline(deviceStream, port)) {
    //         port.erase(std::remove(port.begin(), port.end(), ' '), port.end());
    //         //cout << port << "numbrergterbehd" << endl;
    //         if (port=="10"  || port=="0") {
    //             return true; 
    //         }
    //     }
    //     return false;
    // }

    bool check_sd_card_conncted(){
        string sd_card_result = util.executeTerminal(sd_card_command);
        if(!sd_card_result.empty()){
            return true;
        }
        return false;
    }

};
bool HDMI_VGA_PortCheck::VGA_available = true;
bool HDMI_VGA_PortCheck::HDMI_available = true;
bool HDMI_VGA_PortCheck::DP_available = false; 
bool HDMI_VGA_PortCheck::sd_card_available = false; 