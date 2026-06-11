
#include <iostream>
#include <../lib/freshweb/emit.h>
#include <fstream>
#include <vector>
#include <mutex>
using namespace std;


class USBcheck: public V2Service{
    // Placeholder classes for context; user-defined implementations are assumed
    IQCService iqcservice;
    PartConfigService partconfig;
    PartResultService partresultservice;
    NjsettingsService njsettings;
    Constant constant;
    TypeC_chargerCheck type_c;

public:
    Util util;  
    // std::string CMD_USB_MassStorage = "powershell -Command \"Get-WmiObject Win32_logicalDisk |Where-Object {$_.Description -eq 'Removable Disk'}\"";
    string countusbcommand="powershell.exe -Command \"Get-WmiObject -Class Win32_DiskDrive | Where-Object { $_.MediaType -NotLike '*Fixed*' -and  $_.Size -gt 9663676416  } | Measure-Object | Select-Object -ExpandProperty Count\"";
    // std::string cmd_USB_HID="powershell -Command \"Get-PnpDevice -PresentOnly | Where-Object { $_.Class -eq 'HIDClass' -and $_.InstanceId -match '^USB' -and $_.Status -eq 'OK'} | Format-List FriendlyName , InstanceId\"";
    // std::string CMD_USB = "powershell -Command \"Get-WmiObject Win32_logicalDisk\"";
    // static std::string FileSize;  
    // static std::string file_name; 
    static int USBPort_count;
    static string Selected_model;

    std::map<std::string, std::string> parameterMap;

    // static int HIDCount;
    // int InsertedHIDcount ;

    // Main USB port test method
    int USBport_test(const std::string stock_info_id, const std::string stock_part_id, const std::string item_id, const std::string user) {
        int return_value = 0;
        nj_emit({{"event","test_start"},{"test","USB_status"},{"label","USB Port Test"},{"part","Motherboard"}});
        // cout<<"\nUSB file name: "<<file_name<<endl;
        // string result=util.executeTerminal(cmd_USB_HID);
        // std::map<std::string, std::string> config;
        std::map<std::string, std::string>fieldmap ;
        // std::map<int, std::map<std::string, std::string>> configMap;
        // cout<<result<<endl;
        // result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
        // std::istringstream inputstream(result);
        // json AfterinsertjsonData= util.ExcetuteTerminalJSonArray1(inputstream);
        // int count = AfterinsertjsonData.size();
        // InsertedHIDcount = count-HIDCount;
        // cout<<"\nInserted HID count: "<<InsertedHIDcount<<endl;
        // int part_config_id = 0;
        // string port_number="";  
        // std::map<std::string, std::string> requestMap;
        // json jsonData;
        // jsonData ["manufacturer_model"]= iqcservice.model_name;
        // cout<<"manufacturer_model: "<<iqcservice.model_name<<endl;
        // string port=iqcservice.getParameterList(constant.API_PORT_URL, "PUT", requestMap, to_string(jsonData));
        // cout<<"port: "<<port<<endl;
        // if(port!=""){
        // try {
        // json jsonresponse = json::parse(port);
        // port_number= jsonresponse.at("message");
        // USBPort_count=stoi(port_number);

        // } catch (const exception& e) {
        // cerr << "Error while getting the port no: " << e.what() << port<<endl;
        // }
        // }
        // Emit interactive_needed — Flutter shows USB port test screen
        nj_emit({{"event","interactive_needed"},{"test","USB_status"},
                  {"label","USB Port Test"},
                  {"instruction","Connect USB drives to ALL ports, then click Done"}});
        bool usb_passed = nj_poll_ack();
        if (!usb_passed) {
            parameterMap["USB_Port_status"] = "Not Working";
        } else {
            // Run automatic detection to confirm
            if(!usb_working()){
                parameterMap["USB_Port_status"] = "Not Working";
            }
        }
        

        // for(int i=0; i<InsertedHIDcount; i++){
        //     // cout<<"\n Loop Running..."<<endl;
        //    config["part_id"] = stock_part_id;
        //    config["DeviceID"] = AfterinsertjsonData[i]["InstanceId"];
        //    config["DriveType"] = AfterinsertjsonData[i]["FriendlyName"];
        //    configMap[i] = config;
        //    part_config_id=partconfig.updatePartConfig(stock_info_id, configMap);
        //    cout<<"\ninsert HID part_config_id: "<<part_config_id<<endl;
        //    parameterMap["USB_Port_status"]="Working";
           parameterMap["Selected_model"]=Selected_model;
           fieldmap["stock_info_id"] = stock_info_id;
           fieldmap["part_id"] = stock_part_id;
        //    fieldmap["part_config_id"] =to_string(part_config_id); // part_config_id;
           fieldmap["item_id"] = item_id;
           fieldmap["created_by"] = user;
           partresultservice.updateBulkPartResult(parameterMap, fieldmap);
        
       

        // std::map<std::string, std::string> partTestInfomap;
       
        // part_config_id = update(stock_info_id, stock_part_id,item_id, user);

        nj_emit({{"event","test_done"},{"test","USB_status"},{"status","done"}});
        return return_value;
    }
    bool usb_working(){
        int type_c_count=0;
       if(USBPort_count==0){
         USBPort_count = 1; // Default if not set by Item Wizard
         }

         int usb_deteced_count;
         string detected_count=util.executeTerminal(countusbcommand);
         try{
            usb_deteced_count=stoi(detected_count);
         }catch(std::exception &e){
            cout<<"Error While getting the USB count"<<e.what()<<endl;
         }
        type_c_count=type_c.no_of_type_c("Type_C_USB.txt");
        parameterMap["Type_C_USB"]=to_string(type_c_count);
        usb_deteced_count=usb_deteced_count-type_c_count;  // remove TypeC
        parameterMap["Number_of_USBPort_Detected"]=to_string(usb_deteced_count);  // save only normal USB
        parameterMap["Number_of_USBPort_Available"]=to_string(USBPort_count);
         if(usb_deteced_count>=USBPort_count){
            parameterMap["USB_Port_status"]="Working";
            return true;
         }else{
            parameterMap["USB_Port_status"]="Not Working";
            return false;
        }
    }

 

    // Update port configuration and return part config ID
//    int update(const std::string& stockinfoID, const std::string& stock_part_id,std::string ItemId, std::string user) {
//     std::map<int, std::map<std::string, std::string>> partinfoMap;
//     std::map<std::string, std::string> partTestInfomap;
//     std::map<std::string, std::string> newport;
//     std::map<std::string, std::string>fieldmap ;
//     fieldmap["item_id"] = ItemId;
//     fieldmap["created_by"] = user;
//     fieldmap["stock_info_id"] = stockinfoID;
//     fieldmap["part_id"] = stock_part_id;

//     string result = util.executeTerminal(countusbcommand);
//     std::istringstream inputStream(result);
//     json HddData = parseHDDData(inputStream);
//     // int totalUSBcount = HddData.size()+InsertedHIDcount;
//     // std::cout<<"\n"<<totalUSBcount<<" USB Ports are detected"<<endl;

//     // partTestInfomap["Number_of_USBPort_Detected"]=to_string(totalUSBcount);
//     partresultservice.updateBulkPartResult(partTestInfomap, fieldmap);
//     partTestInfomap.clear();

//     for(int i=0; i<(USBPort_count-totalUSBcount); i++){
//         partTestInfomap["USB_Port_status"] = "Not Working";
//         partresultservice.updateBulkPartResult(partTestInfomap, fieldmap);
//     }
//     partTestInfomap.clear();
//     // cout << "totalUSBcount: " << totalUSBcount << endl; 

//     result = util.executeTerminal(CMD_USB_MassStorage);
//     std::istringstream inputstream(result);
//     HddData = parseHDDData(inputstream);
//     // cout << "HddData: " << HddData.dump(4) << endl;
//     int part_config_id;
 
//     // Loop through each device in the array and process it
//     for (int i = 0; i < HddData.size(); i++) {

//         newport["part_id"] = stock_part_id;

//         newport["DeviceID"] = HddData[i]["DeviceID"];
//         newport["DriveType"] = HddData[i]["DriveType"];
//         newport["VolumeName"] = HddData[i]["VolumeName"];
//         newport["FreeSpace"] = HddData[i]["FreeSpace"];
//         newport["Size"] = HddData[i]["Size"];

        
//         partinfoMap[0] = newport;
//         part_config_id = partconfig.updatePartConfig(stockinfoID, partinfoMap);
//         std::cout << "part_config_id: " << part_config_id <<flush <<std::endl;

        
//         string deviceid = HddData[i]["DeviceID"];
//         deviceid = util.trim(deviceid);
//         partTestInfomap = test(HddData[i]["DeviceID"]);

//         // if (partTestInfomap.find("USB_Port_status") == partTestInfomap.end()) {
//         //     partTestInfomap["USB_Port_status"] = "Working";
//         // }


//         fieldmap["part_config_id"] =to_string(part_config_id); // part_config_id;


//         if (!partTestInfomap.empty()) {
//             // cout<<"\n USB Data uploading... for partconfig id: "<<part_config_id<<endl;
//             partresultservice.updateBulkPartResult(partTestInfomap, fieldmap);
            
//         }
       
//        }
//     iqcservice.ChecksResultSync("USB PORT Check", std::to_string(part_config_id));
//     return part_config_id;

    // Update part configuration using partinfoMap

// }


//     std::map<std::string, std::string> test(const std::string& deviceID) {
//     std::map<std::string, std::string> partTestInfomap;

//     std::string Time2transfer_file = transfer_file(deviceID);
//     std::string Time2receive_file = receive_file(deviceID);

//     // Check if any of the transfer or receive times contain "error"
//     if (Time2receive_file == "error" || Time2transfer_file == "error") {
//         partTestInfomap["USB_Port_status"] = "Not Working";
//     } else {
//         partTestInfomap["Time2transfer_file"] = Time2transfer_file;
//         // partTestInfomap["Transfervd_File_size"] = FileSize;
//         partTestInfomap["Time2receive_file"] = Time2receive_file;
//         // partTestInfomap["Received_file_size"] = FileSize;
//         partTestInfomap["USB_Port_status"] = "Working";
//     }

//     return partTestInfomap;
// }

    // Transfer a file and return time taken
    // std::string transfer_file(const std::string& deviceID) {
    //     std::cout << "\nSending file..." << std::endl;
    //     std::string source_path=util.executeTerminal("cd");
    //     source_path = util.trim(source_path);
    //     // source_path = source_path+"\\Temp_Data\\"+file_name;
    //     // cout<<"source path=================" << source_path << endl;

    //     std::string destination_path =deviceID+"\\Temp";
    //     // cout<<"destination path=================" << destination_path << endl;

    //     destination_path = util.trim(destination_path);
    //     string Test_command="powershell -Command \"Test-Path "+destination_path+" -PathType Container\"";
    //     string dir_status=util.executeTerminal(Test_command);
    //     // cout<<"dir_status=================" << dir_status << endl;
    
    //     if(dir_status.find("False") != std::string::npos){
    //         std::string mkdir_command = "powershell -Command \"New-Item -ItemType Directory -Path " + destination_path+"\"";
    //         // cout<<mkdir_command<<endl;
    //         util.executeTerminal(mkdir_command);
    //     }
       
    //     std::string copy_command = "powershell -Command \"Copy-Item '" + source_path + "' -Destination '" + destination_path + "\\'\"";
    //     // cout<<copy_command<<endl;
        
    //     // std::cout << "Copy command Sending file: " << copy_command << std::endl;

    //     auto start = std::chrono::steady_clock::now();
    //     std::string result = executeCopy(copy_command);
    //     auto end = std::chrono::steady_clock::now();

    //     if (result.empty()) {
    //         std::chrono::duration<double> elapsed_seconds = end - start;
    //         return std::to_string(elapsed_seconds.count()) + " Sec";
    //     }
    //     return result;  // Return the result for error handling if needed
    // }

    // Function to receive a file
//     std::string receive_file(const std::string& deviceID) {
//         std::cout << "\nReceiving file..." << std::endl;        
//         // std::string source_path = deviceID+"\\Temp\\"+file_name;
//         // source_path = util.trim(source_path);
//         // cout<<"source path=================" << source_path << endl;

//         std::string destination_path = util.executeTerminal("cd"); 
//         destination_path = util.trim(destination_path);
//         destination_path = destination_path+"\\Temp_Data";
//         // cout<<"destination path=================" << destination_path << endl;

//         std::string copy_command = "powershell -Command \"Copy-Item '" + source_path + "' -Destination '" + destination_path + "'\"";
//         // cout << "copy command receiving file: " << copy_command << endl;

//         auto start = std::chrono::steady_clock::now();
//         std::string result = executeCopy(copy_command);
//         auto end = std::chrono::steady_clock::now();

//         if (result.empty()) {
//             std::chrono::duration<double> elapsed_seconds = end - start;
//             return std::to_string(elapsed_seconds.count()) + " Sec";
//         }
//         return result; 
//     }
//     // Execute terminal command and return output


//     // Simulate file copy command and return result
//     std::string executeCopy(const std::string& cmd) {
//         int result = system(cmd.c_str());
//         return result == 0 ? "" : "error"; // Return "error" on failure
//     }

//     // Utility method to populate field map for database update



//      private: json parseHDDData(std::istringstream& input) {
//     json jsonArray = json::array();
//     map<string,string> hddData;
//     std::string line;
//     while (std::getline(input, line)) {
//         if (line.empty()) {
//           if(!hddData.empty()){
//             json jsonObject = hddData; 
//             hddData.clear();
//             jsonArray.insert(jsonArray.end(), jsonObject);
//         }
         
//         }      

//         size_t delimiterPos = line.find(':');
//         if (delimiterPos != std::string::npos) {
//             std::string key = line.substr(0, delimiterPos);
//             key = util.trim(key);

//             std::string value = line.substr(delimiterPos+1);
//             hddData[key] = value;
//         }
//     }
//       if(!hddData.empty()){
//             json jsonObject = hddData; 
//             jsonArray.insert(jsonArray.end(), jsonObject);
//         }

//     return jsonArray;
//     }
//     public:void update_USB_filesize(string department){
//     json result=njsettings.getNjSettingsvalues();
//     for(const auto&item: result){
   
//       if(item["department"]==department && item["parameter"]=="USB_FileName"){
//         // file_name=item["value"].get<string>();
//       }    
      
//     }
//   }
public:    bool runUSBTest(string result){
        std::vector<std::string>USBCheckInfo;
        USBCheckInfo.push_back("USB_Port_status");
        USBCheckInfo.push_back("Number_of_USBPort_Detected");
        // RAMCheckInfo.push_back("TabSwitchTime");
        // LIDCheckInfo.push_back("HDMI_Status");

        // driverCheckInfo.push_back("Error_Drivers_Count");

        for(const std::string& bluetoothcheck:USBCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
};
//   string USBcheck::FileSize;
  string USBcheck::Selected_model;
  int USBcheck::USBPort_count=0;
//   int USBcheck::HIDCount;