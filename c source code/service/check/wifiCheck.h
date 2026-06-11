#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
// #include "../service/NJsettinsService.h"
#include "../service/check/LANchek.h"
using namespace std;


bool executeCommand(const std::string& command) {
    return system(command.c_str()) == 0;
}



class WifiCheck : public V2Service {
     private:
        IQCService Iqcservice;
        PartResultService partResultService;
        Util util;
        NjsettingsService njset;
        LanCheck lancheck;

        static string file_location;
        static string wifi_name;
        static string wifi_password;
        map<string, string> paramtrmap;

   public:int wifi_test(string stockinfoID, string partId, string partconfigid, string item_id, string created_by) {
        cout << "\nWifi check is Running..." << endl;
        LAN_Down();
        cout<<"\nWifi File Location is set to "<<file_location<<endl;
        // cout<<"\nWifi Name is set to "<<wifi_name<<endl;
        // cout<<"\nWifi Password is set to "<<wifi_password<<endl;
        int returnvalue = 0;
        bool wifi_connected;
        wifi_connected = isWiFiConnected();
        // string open_privacy_location="start ms-settings:privacy-location";
        if (!wifi_connected) {            
            // util.executeTerminal(open_privacy_location);
            // Sleep(7000);
            if(!wifi_name.empty() && !wifi_password.empty() ){
            updateWiFiProfile("wifitest.xml",wifi_name,wifi_password);
            connect_network(wifi_name, wifi_password);
            }
            // cout << "WiFi is not connected." << endl;
            // cout<<"\033[33m\nWiFi is offline. To Continue Turn On wifi and press [y/Y]or To Skip Press [n/N]:\033[0m" <<flush;
        
            // cout << "WiFi is offline. To Continue Turn On wifi and press [y/Y]or To Skip Press [n/N]: ";
            // string user_choice;
            // cin >> user_choice;

            wifi_connected=isWiFiConnected();
                // wifi_connected =connect_network(wifi_name, wifi_password);
            // if(!wifi_connected){
            //     util.generatePopupOK("Wifi not connected, please connect wifi and then click OK");
            //     Sleep(2000);
            
            // wifi_connected=isWiFiConnected();
            // if (!wifi_connected) {
            //     paramtrmap["Wifi_status"] = "Not Working";
            //     paramtrmap["Wifi_File_status"] = "File not Downloaded";
                
            // }
            // }

            if(!wifi_connected){
                while(true){
                  if(isWiFiConnected()){
                    wifi_connected=true;
                    break; 
                  }else{
                    bool status=util.generatePopup("Wifi Not Connected", "Please connect wifi and then click Retry. if wifi not able to connect Click Not Working?");
                    if(status){
                        wifi_connected=false;
                        paramtrmap["Wifi_status"] = "Not Working";
                        paramtrmap["Wifi_File_status"] = "File not Downloaded";
                        break;
                    }
                  }                 
                }
            }
            
        }

        if (wifi_connected) {
            string filename = lancheck.getFileNameFromUrl(file_location);
            string fileUrl = lancheck.urlEncode(file_location);
            string file_transfer_time = lancheck.download(fileUrl, filename);
            // cout << "File transfer time: " << file_transfer_time << endl;

            if (file_transfer_time != "0") {
                paramtrmap["Wifi_File_status"] = "File Downloaded";
                paramtrmap["Wifi_Time_taken"] = file_transfer_time;
                paramtrmap["Wifi_status"] = "Working";
                double filesize = lancheck.getFileSize(fileUrl);
                filesize=std::round(filesize*100)/100;
                paramtrmap["Wifi_File_size"] = to_string(filesize);
                // cout << "File size: " << lancheck.byteconversion(filesize) << endl;
            }
            else {
                paramtrmap["Wifi_status"] = "Working";
                paramtrmap["Wifi_File_status"] = "File not Downloaded";
                // paramtrmap["Wifi_Time_taken"] = "0";
                // paramtrmap["Wifi_File_size"] = "0";
                
            }
        }
        map<string, string> fieldidmap;
        fieldidmap["stock_info_id"] = stockinfoID;
        fieldidmap["part_id"] = partId;
        fieldidmap["part_config_id"] = partconfigid;
        fieldidmap["item_id"] = item_id;
        fieldidmap["created_by"] = created_by;
        LAN_up();
        returnvalue = partResultService.updateBulkPartResult(paramtrmap, fieldidmap);
        // cout << "Return value: " << returnvalue << endl;
        Iqcservice.ChecksResultSync("WIFI Check");

        cout << "\nWifi Check Completed......\n";
        return returnvalue;
   }

   public:bool isWiFiConnected() {
        string command ="powershell -Command \"(Get-NetAdapter | Where-Object {$_.Name -like '*wi*'}).Status\"";
        string result = util.executeTerminal(command);
        paramtrmap["Wifi_connection_status"]=result;
        if (result.find("Up") != std::string::npos) {
            return true;
        }
        
        return false;
    }

    // Function to connect to a WiFi network
   public:bool connect_network( string wifi_name, string wifi_password) {
           string xmlcommand="netsh wlan add profile filename=\"updatedwifiprofile.xml\"";
           executeCommand(xmlcommand);
           Sleep(2000);
           string command = "netsh wlan connect name=\"" + wifi_name + "\"";
           if (executeCommand(command)) {
           Sleep(4000); // Sleep for 5 seconds to allow connection
           return true;
           }
           return false;
   }

   public:
    void update_WIFI_file(const string& department, json njsetinfo) {
        // json njsetinfo = njset.getNjSettingsvalues();
        for (const auto& entry : njsetinfo) {
            if (entry["department"] == department) {
                if (entry["parameter"] == "file_location") {
                    file_location = entry["value"];
                    // cout << "File location: " << file_location << endl;
                }
                else if (entry["parameter"] == "wifi_name") {
                    wifi_name = entry["value"];
                }
                else if (entry["parameter"] == "wifi_password") {
                    wifi_password = entry["value"];
                }
            }
        }
    }
    private: void LAN_Down(){
        string lan_Down_command = "powershell -Command \"Get-NetAdapter | Where-Object { $_.Name -like '*Ethernet*' } | ForEach-Object { Disable-NetAdapter -Name $_.Name -Confirm:$false }\"";
    //    string lan_Down_Command="powershell -Command \"Disable-NetAdapter -Name 'Ethernet' -Confirm:$false\"";
       bool result=executeCommand(lan_Down_command);
       Sleep(2000);
       if(result){
        cout<<"\nLAN Disabled Successfully..."<<flush<<endl;
        }else{
            cout<<"\nLAN Not Disabled..."<<flush<<endl;
        }
    }
    private: void LAN_up(){
        string lan_up_Command = "powershell -Command \"Get-NetAdapter | Where-Object { $_.Name -like '*Ethernet*' } | ForEach-Object { Enable-NetAdapter -Name $_.Name -Confirm:$false }\"";
    //    string lan_Down_Command="powershell -Command \"Enable-NetAdapter -Name 'Ethernet' -Confirm:$false\"";
       Sleep(3000);
       bool result=executeCommand(lan_up_Command);
       Sleep(2000);
       if(result){
        cout<<"\nLAN Enabled Successfully..."<<flush<<endl;
        }else{
            cout<<"\nLAN Not Enabled..."<<flush<<endl;
        }
    }

void updateWiFiProfile(const std::string& filePath, const std::string& wifiName, const std::string& wifiPassword) {
    // Open the input file for reading
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return;
    }

    std::string xmlContent((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    std::string ssidPlaceholder = "wifiname";
    std::string passwordPlaceholder = "wifipassword";
    std::string namePlaceholder = "wifinametoshow";

    
    try {
        
        xmlContent = std::regex_replace(xmlContent, std::regex(namePlaceholder), wifiName);
        xmlContent = std::regex_replace(xmlContent, std::regex(ssidPlaceholder), wifiName);
        xmlContent = std::regex_replace(xmlContent, std::regex(passwordPlaceholder), wifiPassword);

    } catch (const std::regex_error& e) {
        std::cerr << "Error in regex_replace: " << e.what() << std::endl;
        return;
    }

    std::string newFilePath = "updatedwifiprofile.xml";
    std::ofstream outputFile(newFilePath);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << newFilePath << std::endl;
        return;
    }
    outputFile << xmlContent;
    outputFile.close();

    std::cout << "WiFi profile updated successfully and written to " << newFilePath << std::endl;
}

    public:    bool runwifiTest(string result){
        std::vector<std::string>wifiCheckInfo;
        wifiCheckInfo.push_back("Wifi_status");
        wifiCheckInfo.push_back("Wifi_File_status");
        // RAMCheckInfo.push_back("TabSwitchTime");
        // LIDCheckInfo.push_back("HDMI_Status");

        // driverCheckInfo.push_back("Error_Drivers_Count");

        for(const std::string& bluetoothcheck:wifiCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }

};

string WifiCheck::file_location;
string WifiCheck::wifi_name;
string WifiCheck::wifi_password;