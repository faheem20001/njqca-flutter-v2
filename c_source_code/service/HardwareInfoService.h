#include <map>
#include <cctype>
#include <iostream>
#include <string>
#include <list>
#include <any>
#include <typeinfo>
#include <sstream>
#include <../model/ItemBomModel.h>
#include <../service/ItemBomService.h>
// #include <../service/sync2ERP/AllSyncService.h>
#include "../service/PartTestService.h"
//#include <../service/MasterFieldService.h>
//#include <../service/PartConfigService.h>

using namespace std;
using json = nlohmann::json;

class HardwareInfoService: public V2Service {

  private:
    const string CMD_SYSTEM = "sudo lshw -json ";
    const string CMD_CPU = "powershell -Command \"Get-WmiObject -Class Win32_Processor\"";
    const string CMD_MEMORY = "powershell -Command \"Get-WmiObject Win32_PhysicalMemory\"";
    const string CMD_MOTHER_BOARD = "powershell -Command \"Get-WmiObject Win32_BaseBoard\"";
    const string CMD_DISPLAY = "powershell -Command \"Get-CimInstance -Namespace root\\wmi -ClassName WmiMonitorConnectionParams |Where-Object { $_.VideoOutputTechnology -notin @(0, 10, 5) } |Format-List *\"";
    const string CMD_DISPLAY1 = "powershell -Command \"Get-CimInstance Win32_VideoController | Where-Object {$_.AdapterDACType -eq 'Internal'} | Format-List VideoProcessor, VideoModeDescription,MaxRefreshRate,MinRefreshRate,Description,Description\"";
    const string CMD_STORAGE = "powershell -Command \"Get-WmiObject Win32_DiskDrive | Where-Object {$_.MediaType -like '*Fixed*'}\"";
    const string CMD_Keyboard = "powershell -Command \"\"";
    const string CMD_DISK = " sudo lshw -json -c disk";
    // const string CMD_WIFI = "powershell -Command \"Get-NetAdapter | Where-Object { $_.Name -like 'Wi*' } | Format-List Name,Status,MacAddress,LinkSpeed,InterfaceDescription\"";
    const string CMD_WIFI = "powershell -Command \"Get-NetAdapter | Where-Object { $_.Name -like 'Wi*' }|Select Name, MacAddress, MediaType, LinkSpeed,NdisVersion,DriverVersion,InterfaceDescription,InterfaceType,@{Name='ReceiveSpeed';Expression={[math]::Round($_.ReceiveLinkSpeed/1MB,2)}},@{Name='TransmitSpeed';Expression={[math]::Round($_.TransmitLinkSpeed/1MB,2)}} |Format-List\"";
    const string CMD_BATTERY = "powershell -Command \"Get-WmiObject -Class Win32_Battery | Format-List Caption,Description,DeviceID,DesignVoltage,Name\"";
    // const string CMD_BATTERY = "wmic path Win32_PortableBattery get  /format:list";
    // Get-WmiObject -Namespace root\wmi -List | Where-Object { $_.Name -like "*Battery*" }
    const string CMD_NETWORK = "powershell -Command \"Get-NetAdapter -Name '*Ethernet*' |Select Name, MacAddress, MediaType, LinkSpeed,NdisVersion,DriverVersion,InterfaceDescription,InterfaceType,@{Name='ReceiveSpeed';Expression={[math]::Round($_.ReceiveLinkSpeed/1MB,2)}},@{Name='TransmitSpeed';Expression={[math]::Round($_.TransmitLinkSpeed/1MB,2)}} |Format-List\"";
    const string CMD_INPUT = "sudo lshw -json -c input";
    const string CMD_USB="powershell -Command \"Get-PnpDevice -Class USB | Where-Object {$_.Status -eq 'OK'} | Format-List FriendlyName, Status,Manufacturer,DeviceID\"";
    // const string CMD_NJPULSE="powershell -Command \"Start-Process -FilePath \"chrome.exe\" -ArgumentList \"http://3.110.131.250/\"\"";
    // const string CMD_ERP="powershell -Command \"Start-Process -FilePath \"chrome.exe\" -ArgumentList \"https://uat.newjaisa.biz/ \"\"";
    const string CMD_MULTIMEDIA = "sudo lshw -json -c multimedia";
    const string CMD_Bluetooth = "powershell -Command \" Get-NetAdapter -Name '*Bluetooth*'|Select Name, MacAddress, MediaType, LinkSpeed,NdisVersion,DriverVersion,InterfaceDescription,InterfaceType,@{Name='ReceiveSpeed';Expression={[math]::Round($_.ReceiveLinkSpeed/1MB,2)}},@{Name='TransmitSpeed';Expression={[math]::Round($_.TransmitLinkSpeed/1MB,2)}} |Format-List\"";
    const string CMD_Speaker = "powershell -Command \"Get-PnpDevice -Class AudioEndpoint | Where-Object { $_.Status -eq 'OK' -and $_.FriendlyName -like '*speaker*'} | Format-List FriendlyName, Manufacturer,DeviceID,Description\"";
    const string CMD_Microphone = "powershell -Command \"Get-PnpDevice -Class AudioEndpoint | Where-Object { $_.Status -eq 'OK' -and $_.FriendlyName -like '*microphone*'} | Format-List FriendlyName, Manufacturer,DeviceID,Description\"";
    const string CMD_Fullchargecapacity="powershell -Command \"Get-WmiObject -Namespace root\\wmi -Class BatteryFullChargedCapacity | Select-Object -Property FullChargedCapacity | Format-List\"";
    const string CMD_DesignCapacity="powershell -Command \"Get-WmiObject -Namespace root\\wmi -Class BatteryStaticData | Select-Object -Property DesignedCapacity | Format-List\"";
    const string CMD_Cyclecount="powershell -Command \"Get-WmiObject -Namespace root\\wmi -Class BatteryCycleCount | Select-Object -Property CycleCount | Format-List\"";
    const string CMD_GPU = "powershell -Command \"Get-CimInstance Win32_VideoController | Select-Object Name, AdapterRAM, DriverVersion, VideoProcessor, AdapterDACType | Format-List; Write-Output ''\"";
    map<int, map<string, string>> allPartConfigMap; 
    

    PartConfigService partConfigService;
    
    Util util;
    RawDataService rawDataService;
    StockInfoModel stockInfoModel;
    StockInfoService stockInfoService;
    ModelModel modelModel;
    ModelService modelService;
    ItemService itemService;
    ItemBomService itemBomService;
    PartTestService parttestservice;
    AllSyncService allSynservice;
  
  public: bool do_cbc;
  public: bool do_adapter;
  public: bool do_ram;
  public: vector<string> selected_checks;

  public: static vector<string> part_list_item_groups;
  

  /*
  * Gets configuration of the system. 
  * @todo: Need to clean up the code
  */
  public: void getConfig(int stockInfoId) {
    map<string,string> item_group_mapping;
    item_group_mapping["cpu"] = "Laptop Processors";
    item_group_mapping["motherboard"] = "MOTHERBOARD";
    item_group_mapping["keyboard"] = "KEYBOARD";
    item_group_mapping["touchpad"] = "TOUCHPAD";
    item_group_mapping["speaker"] = "SPEAKER";
    item_group_mapping["wifi"] = "WIFI Card";
    item_group_mapping["storage"] = "LAPTOP STORAGE";
    item_group_mapping["network"] = "LAN PORT";
    item_group_mapping["display"] = "Display";
    item_group_mapping["battery"] = "BATTERY";
    item_group_mapping["memory"] = "LAPTOP RAM";
    item_group_mapping["camera"] = "CAMERA";
    item_group_mapping["bluetooth"]="Bluetooth";
    item_group_mapping["microphone"]="Microphone";
    item_group_mapping["gpu"] = "GRAPHICS CARD";


    logMsg("Starting to get the configuration 2/2 ....");
    map<string, string> whereMap;
    whereMap["stock_info_id"] = to_string(stockInfoId);
    stockInfoService.whereMap = whereMap;

    //Get the lshw json to map
    map<std::string, std::string> systemInfoMap;


    map<string, string> stockInfoData = stockInfoService.getStockInfoData();
    map<int, map<string, string>> itemPartList = itemBomService.getItemPartList(stockInfoData["item_id"]);
    map<int, map<string, string>> ::iterator it = itemPartList.begin();

    while (it != itemPartList.end()) {
      map<int, map<string, string>> partInfoList;
      map<string, string> itemPartData = it->second;
    //   //Need to use dynmaic call to avoid if condition
      string partMethod = itemPartData["item_njhat_code"];
      // cout<<partMethod<<endl;
    //   logMsg("Starting to get the " + itemPartData["itemPartName"] + " Information...");

      transform(partMethod.begin(), partMethod.end(), partMethod.begin(), ::tolower);
    string checkKey = partMethod;
      if (selected_checks.empty()) {
          ++it;
          continue;
      }
      bool checkFound = std::find(selected_checks.begin(), selected_checks.end(), checkKey) != selected_checks.end();
      if (!checkFound) {
          ++it;
          continue;
      }
    if (partMethod == "cpu")          {partInfoList = getPartInfo(itemPartData, CMD_CPU); }
    if (partMethod == "memory")       {partInfoList = getPartInfo(itemPartData, CMD_MEMORY);
       if(partInfoList.size()>0){
        if(std::count(part_list_item_groups.begin(),part_list_item_groups.end(),item_group_mapping[partMethod])>part_list_item_groups.size()){
          util.generatePopupredok("RAM Not Detected","No of RAM in the Part list is more than Available RAM in the system");
        }
      }
    }
    if (partMethod == "motherboard")  {partInfoList = getPartInfo(itemPartData,CMD_MOTHER_BOARD);}  
    if (partMethod == "keyboard")  {partInfoList = getKeyboardInfo(itemPartData);}  
    if (partMethod == "keyboard cable")  {partInfoList = getKeyboardInfo(itemPartData);}  
    if (partMethod == "touchpad")  {partInfoList = getTouchPadInfo(itemPartData);}  
    if (partMethod == "touchpad cable")  {partInfoList = getTouchPadInfo(itemPartData);}  
    //   // if (partMethod == "display")      {partInfoList = getPartData(std::any_cast<json>(pciChildConfigMap["display"]), itemPartData, false, true);}
    //   // if (partMethod == "storage")      {partInfoList = getPartData(std::any_cast<json>(pciChildConfigMap["pci:1"]), itemPartData, true, true);}
    if (partMethod == "storage")      {partInfoList = getStorageInfo(itemPartData, CMD_STORAGE);} //getPartData(std::any_cast<json>(pciChildConfigMap["pci:1"]), itemPartData);}
    if (partMethod == "speaker")      {partInfoList = getPartInfo(itemPartData, CMD_Speaker);
      if(partInfoList.size()>0){
        if(std::count(part_list_item_groups.begin(),part_list_item_groups.end(),item_group_mapping[partMethod])>part_list_item_groups.size()){
          util.generatePopupredok("Disk Not Detected","No of Disk in the Part list is more than Available Disk in the system");
        }
      }
    } 
    if (partMethod == "microphone")      {partInfoList = getPartInfo(itemPartData, CMD_Microphone);
        if(partInfoList.empty()){
          util.generatePopupredok(item_group_mapping[partMethod]+" Not Detected","It may not be Available in the system, please check?");
        }
    } 
      
    //   /*if (partMethod == "bios")         {partInfoList = getPartData(std::any_cast<json>(mbChildConfigMap["firmware"]), itemPartData, false, true);}
    //   if (partMethod == "cache")        {partInfoList = getPartData(std::any_cast<json>(mbChildConfigMap["cache:0"]), itemPartData);}*/
                     
                       
    //   // if (partMethod == "cpu")         {partInfoList = getCPUInfo(itemPartData["item_part_id"]);}
    //   // if (partMethod == "motherboard") {partInfoList = getPartInfo(CMD_MOTHER_BOARD, itemPartData["item_part_id"], "core");}
    //   // if (partMethod == "memory")      {partInfoList = getMemoryInfo(itemPartData["item_part_id"]);}
    //   // if (partMethod == "bios")        {partInfoList = getPartInfo(CMD_MEMORY, itemPartData["item_part_id"], "firmware");}
    //   // if (partMethod == "cache")       {partInfoList = getPartInfo(CMD_MEMORY, itemPartData["item_part_id"],"cache:",true);}
      if (partMethod == "display")     {partInfoList = getPartInfo(itemPartData, CMD_DISPLAY1);}
      if (partMethod == "gpu")         {partInfoList = getPartInfo(itemPartData, CMD_GPU);}
    //   if (partMethod == "disk")        {partInfoList = getPartInfo(CMD_DISK, itemPartData["item_part_id"]);}
      if (partMethod == "battery")     {partInfoList = getBatteryInfo(itemPartData, CMD_BATTERY);}
    //   if (partMethod == "audio")       {partInfoList = getPartInfo(CMD_MULTIMEDIA, itemPartData["item_part_id"],"Audio device", false, "description");}
      if (partMethod == "camera")      {partInfoList = cameraInfo(itemPartData);}
      if (partMethod == "camera cable")      {partInfoList = cameraInfo(itemPartData);}
      if (partMethod == "network")     {partInfoList = getPartInfo(itemPartData, CMD_NETWORK);}
      if (partMethod == "wifi")        {partInfoList = getPartInfo(itemPartData, CMD_WIFI);}
      if (partMethod == "antenna cable")        {partInfoList = getPartInfo(itemPartData, CMD_WIFI);}
      if (partMethod=="usb")      {partInfoList=getPartInfo(itemPartData,CMD_USB);}
      if (partMethod == "bluetooth") {

        // util.executeTerminal("rfkill unblock bluetooth");
        partInfoList = getPartInfo(itemPartData,CMD_Bluetooth);
        if(partInfoList.empty()){
          util.generatePopupredok(item_group_mapping[partMethod]+" Not Detected","It may not be Available in the system, please check?");
        }
      }
      // if (partMethod == "sleepbutton") {partInfoList = getPartInfo(CMD_INPUT, itemPartData["item_part_id"],"Mouse",false,"description");}

      // if (partMethod == "powerbutton") {partInfoList = getPartInfo(CMD_INPUT, itemPartData["item_part_id"],"Power Button",false,"product");}
      // if (partMethod == "lidswitch")   {partInfoList = getPartInfo(CMD_INPUT, itemPartData["item_part_id"],"Lid Switch",false,"product");}


      // if (partMethod == "input")       {partInfoList = getPartInfo(CMD_INPUT, itemPartData["item_part_id"]);}
      // if (partMethod == "multimedia")  {partInfoList = getPartInfo(CMD_MULTIMEDIA, itemPartData["item_part_id"]);}
      if (partInfoList.empty()) {
        // if(std::find(part_list_item_groups.begin(), part_list_item_groups.end(), item_group_mapping[partMethod]) != part_list_item_groups.end()){
        //   util.generatePopupredok(item_group_mapping[partMethod]+" Not Detected","It may not be Available in the system, please check?");
        // }
        map<string,string>partmap;
        partmap["part_id"] = itemPartData["item_part_id"];
        partmap["Availability"]="Not Available";
        partInfoList[0] = partmap;
      }
      addPartConfig(partInfoList);
    //   logMsg("Completed " + itemPartData["itemPartName"] + " Information...");
    cout<<"\nCompleted " + itemPartData["item_njhat_code"] + " Information..."<<flush<<endl;
  		++it;
  	}
    // util.executeTerminal(CMD_NJPULSE);
    // util.executeTerminal(CMD_ERP);
    partConfigService.updatePartConfig(to_string(stockInfoId), allPartConfigMap);
    allSynservice.allSync();   
  }


  public: map<int, map<std::string, std::string>>  getBatteryInfo( map<string,string> itemPartInfo, string command) {
    string partId = itemPartInfo["item_part_id"];
    map<int, map<std::string, std::string>> partInfoMap;
    map<std::string, std::string> partMap;
    int count = 0;
  
    // string CMD_battery="powershell.exe -Command \"Get-WmiObject -Class Win32_Battery\"";
    // const string CMD_battery = "wmic path Win32_Battery get  /format:list";
    string result=util.executeTerminal(command);
    result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
    std::istringstream inputstream(result);
    // cout<<"\nCPU Info"<<"Started.."<<CMD_CPU<<"\n";
     json jsonData= util.ExcetuteTerminalJSonArray1(inputstream);
    //  cout<<jsonData.dump(2)<<endl;
    //  jsonData.clear();
    //  if(jsonData.size()==0){
    //   result=util.executeTerminal(CMD_battery);
    //   result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
    //   inputstream=std::istringstream(result);
    //   // cout<<"\nCPU Info"<<"Started.."<<CMD_CPU<<"\n";
    //   jsonData= util.ExcetuteTerminalJSonArray(inputstream);
    //  }
     result=util.executeTerminal(CMD_Fullchargecapacity);
     inputstream=std::istringstream(result);
     json Fullchargecapacity= util.ExcetuteTerminalJSonArray1(inputstream);
    //  cout<<Fullchargecapacity.dump(2)<<endl;
     result=util.executeTerminal(CMD_Cyclecount);
     inputstream=std::istringstream(result);
     json Cyclecount= util.ExcetuteTerminalJSonArray1(inputstream);
     
     result=util.executeTerminal(CMD_DesignCapacity);
     inputstream=std::istringstream(result);
     json Designcapacity= util.ExcetuteTerminalJSonArray1(inputstream);
    //  cout<<Cyclecount.dump(2)<<endl;
    // jsonData.erase(std::remove(jsonData.begin(), jsonData.end(), '\r'), jsonData.end());
    // cout<<jsonData.dump(2);
    int designcapacity=0;
    for(int i = 0; i < jsonData.size(); ++i) {
    try {
      string execFunction = itemPartInfo["exec_func"];
      for (const auto& item : jsonData[i].items()) {
        // if(item.key() == "DesignCapacity"){ designcapacity = stoi(item.value().get<string>());}
        partMap[item.key()] = util.json2String(item.value());
        //util.printMap(partMap);
        partMap = partExecFunction(execFunction, partMap);
      }
        
      
    }
        catch (...) {
      logError("Error: ");
    }
    int Health=0;
    // Fullchargecapacity.clear();
    try{
    int fullchargecapcity=stoi(Fullchargecapacity[i]["FullChargedCapacity"].get<string>());
    designcapacity=stoi(Designcapacity[i]["DesignedCapacity"].get<string>());
  
    Health=static_cast<int>((static_cast<double>(fullchargecapcity) / designcapacity) * 100);
   
    //util.printMap(partMap);
    partMap["Health"]=to_string(Health);
    partMap["Fullchargecapacity"] = Fullchargecapacity[i]["FullChargedCapacity"];
    partMap["DesignCapacity"]=Designcapacity[i]["DesignedCapacity"];
    partMap["Cyclecount"] = Cyclecount[i]["CycleCount"];
     }catch(std ::exception e){
      cout<<"Unable to get the battery data: "<<e.what()<<endl;
    }
    // cout<<"ruunig after catch\n"<<endl;
    partMap["part_id"] = partId;
    partInfoMap[partInfoMap.size()] = partMap;
    count++;
    }
    return partInfoMap;
  }
     public: map<int,map<std::string, std::string>>post_getBatteryInfo() {
    map<int, map<std::string, std::string>> partInfoMap;
    map<std::string, std::string> partMap;
    int count = 0;
  
    // string CMD_battery="powershell.exe -Command \"Get-WmiObject -Class Win32_Battery\"";
    // const string CMD_battery = "wmic path Win32_Battery get  /format:list";
    string result=util.executeTerminal(CMD_BATTERY);

    result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
    std::istringstream inputstream(result);
    // cout<<"\nCPU Info"<<"Started.."<<CMD_CPU<<"\n";
     json jsonData= util.ExcetuteTerminalJSonArray1(inputstream);
     if(jsonData.size()==0){
      for(int run=0;run<5;run++){  
       result=util.executeTerminal(CMD_BATTERY);
       result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
       inputstream=std::istringstream(result);
       // cout<<"\nCPU Info"<<"Started.."<<CMD_CPU<<"\n";
       jsonData=util.ExcetuteTerminalJSonArray1(inputstream);
       if(jsonData.size()>0){
        break;
       }
      }
     }
     result=util.executeTerminal(CMD_Fullchargecapacity);
     inputstream=std::istringstream(result);
     json Fullchargecapacity= util.ExcetuteTerminalJSonArray1(inputstream);
     if(Fullchargecapacity.size()==0){
      for(int run=0;run<5;run++){  
       result=util.executeTerminal(CMD_Fullchargecapacity);
       result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
       inputstream=std::istringstream(result);
       // cout<<"\nCPU Info"<<"Started.."<<CMD_CPU<<"\n";
       Fullchargecapacity=util.ExcetuteTerminalJSonArray1(inputstream);
       if(Fullchargecapacity.size()>0){
        break;
       }
      }
     }
    //  cout<<Fullchargecapacity.dump(2)<<endl;
     result=util.executeTerminal(CMD_Cyclecount);
     inputstream=std::istringstream(result);
     json Cyclecount= util.ExcetuteTerminalJSonArray1(inputstream);

     if(Cyclecount.size()==0){
      for(int run=0;run<5;run++){  
       result=util.executeTerminal(CMD_Cyclecount);
       result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
       inputstream=std::istringstream(result);
       // cout<<"\nCPU Info"<<"Started.."<<CMD_CPU<<"\n";
       Cyclecount=util.ExcetuteTerminalJSonArray1(inputstream);
       if(Cyclecount.size()>0){
        break;
       }
      }
     }
     
     result=util.executeTerminal(CMD_DesignCapacity);
     inputstream=std::istringstream(result);
     json Designcapacity= util.ExcetuteTerminalJSonArray1(inputstream);

     if(Designcapacity.size()==0){
      for(int run=0;run<5;run++){  
       result=util.executeTerminal(CMD_DesignCapacity);
       result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
       inputstream=std::istringstream(result);
       // cout<<"\nCPU Info"<<"Started.."<<CMD_CPU<<"\n";
       Designcapacity=util.ExcetuteTerminalJSonArray1(inputstream);
       if(Designcapacity.size()>0){
        break;
       }
      }
     }
    //  cout<<Cyclecount.dump(2)<<endl;
    // jsonData.erase(std::remove(jsonData.begin(), jsonData.end(), '\r'), jsonData.end());
    // cout<<jsonData.dump(2);
    int designcapacity=0;
    for(int i = 0; i < jsonData.size(); ++i) {
    int Health=0;
    // Fullchargecapacity.clear();
    try{
    int fullchargecapcity=stoi(Fullchargecapacity[i]["FullChargedCapacity"].get<string>());
    designcapacity=stoi(Designcapacity[i]["DesignedCapacity"].get<string>());
  
    Health=static_cast<int>((static_cast<double>(fullchargecapcity) / designcapacity) * 100);
   
    //util.printMap(partMap);
    partMap["Data_updated"]="Pre-BDT-info";
    partMap["Health"]=to_string(Health);
    partMap["Fullchargecapacity"] = Fullchargecapacity[i]["FullChargedCapacity"];
    partMap["DesignCapacity"]=Designcapacity[i]["DesignedCapacity"];
    partMap["Cyclecount"] = Cyclecount[i]["CycleCount"];
     }catch(std ::exception e){
      cout<<"Unable to get the battery data: "<<e.what()<<endl;
    }
    // cout<<"ruunig after catch\n"<<endl;
    partInfoMap[partInfoMap.size()] = partMap;
    count++;
    }
    return partInfoMap;
  }
  public: map<int, map<std::string, std::string>>  getStorageInfo( map<string,string> itemPartInfo, string command) {
    string diskcommand="powershell.exe -Command \"Get-PhysicalDisk | Select-Object Model ,BusType,MediaType | Format-List\"";
    string partId = itemPartInfo["item_part_id"];
    map<int, map<std::string, std::string>> partInfoMap;
    map<std::string, std::string> partMap;   
    string result=util.executeTerminal(command);
    std::istringstream inputstream(result);
    json jsonData= util.ExcetuteTerminalJSonArray1(inputstream);

    string diskresult=util.executeTerminal(diskcommand);
    std::istringstream diskinputstream(diskresult);
    json diskdata= util.ExcetuteTerminalJSonArray1(diskinputstream);
    for(int i=0; i< jsonData.size(); i++) {
    try {
      string execFunction = itemPartInfo["exec_func"];
      for (const auto& item : jsonData[i].items()) {
        
        partMap[item.key()] = util.json2String(item.value());
        //util.printMap(partMap);
        partMap = partExecFunction(execFunction, partMap);
      }
      string model=partMap.at("Model");
      for(int j=0 ; j< diskdata.size(); j++){
        if(model.find(diskdata[j]["Model"].get<string>()) != std::string::npos ||diskdata[j]["Model"].get<string>().find(model) != std::string::npos){
          partMap["Interface"]=diskdata[j]["BusType"].get<string>() + " disk";
          partMap["Type"]=diskdata[j]["MediaType"].get<string>();
          break;
        }
      }
      
    }
        catch (std ::exception e) {
      cout<<"ERROR while getting the Storage Info: "<<e.what()<<endl;
      logError("Error: ");
    }
    //util.printMap(partMap);
    partMap["part_id"] = partId;
    partInfoMap[partInfoMap.size()] = partMap;
 
    }

    return partInfoMap;
  }
    public: map<int, map<std::string, std::string>>  getKeyboardInfo( map<string,string> itemPartInfo) {
    std::string detectKeyboardsScript =
    "powershell -NoProfile -Command \""
        "Get-PnpDevice -Class Keyboard | Where-Object { $_.Status -eq 'OK' } | ForEach-Object { "
            "$device = $_; "
            "if ($device.InstanceId -match '^ACPI\\\\|^PNP') { "
                "Write-Output \\\"Internal Keyboard: $($device.FriendlyName) [$($device.InstanceId)]\\\"; "
            "} "
            "elseif ($device.InstanceId -match '^HID\\\\|^USB\\\\') { "
                "Write-Output \\\"External Keyboard: $($device.FriendlyName) [$($device.InstanceId)]\\\"; "
            "} "
            "else { "
                "Write-Output \\\"Unknown Keyboard: $($device.FriendlyName) [$($device.InstanceId)]\\\"; "
            "} "
      "}\"";
      
    string partId = itemPartInfo["item_part_id"];
    map<int, map<std::string, std::string>> partInfoMap;
    map<std::string, std::string> partMap;   
    string result=util.executeTerminal(detectKeyboardsScript);
    if(result.find("Internal Keyboard") != std::string::npos) {
      string execFunction = itemPartInfo["exec_func"];
      if (result.length() > 140) {
        result = result.substr(0, 140);
      }
      partMap["Name"]=result;
      partMap = partExecFunction(execFunction, partMap);
      partMap["part_id"] = partId;
      partInfoMap[partInfoMap.size()] = partMap;
    } 
    //util.printMap(partMap);


    return partInfoMap;
  }

   public: map<int, map<std::string, std::string>>  getTouchPadInfo( map<string,string> itemPartInfo) {
    std::string detectTouchpadScript =
    "powershell -NoProfile -Command \""
    "Get-PnpDevice -Class Mouse | Where-Object { $_.Status -eq 'OK' } | ForEach-Object { "
        "$device = $_; "
        "$id = $device.InstanceId; "
        "$name = $device.FriendlyName; "
        "if ($id -match 'ETD|SYN|SMBUS|ACPI\\\\DLL|ELAN' -or $name -match 'Touchpad|TrackPad|Synaptics|ELAN') { "
            "Write-Output \\\"Touchpad: $name [$id]\\\"; "
        "} "
    "}\"";

    string partId = itemPartInfo["item_part_id"];
    map<int, map<std::string, std::string>> partInfoMap;
    map<std::string, std::string> partMap;   
    string result=util.executeTerminal(detectTouchpadScript);
    if(result.find("Touchpad:") != std::string::npos) {
       string execFunction = itemPartInfo["exec_func"];
      if (result.length() > 140) {
        result = result.substr(0, 140);
      }
      
      partMap["Name"]=result;
      partMap = partExecFunction(execFunction, partMap);
      partMap["part_id"] = partId;
      partInfoMap[partInfoMap.size()] = partMap;
    } 
    //util.printMap(partMap);


    return partInfoMap;
  }

  public: map<int, map<std::string, std::string>>  cameraInfo( map<string,string> itemPartInfo) {
    std::string detectCameraScript ="blur.exe";


    string partId = itemPartInfo["item_part_id"];
    map<int, map<std::string, std::string>> partInfoMap;
    map<std::string, std::string> partMap;   
    string result=util.executeTerminal(detectCameraScript);
    if(result.find("Video Blurriness Test") != std::string::npos) {
      
      result = result.substr(result.length()-20, result.length());     
      partMap["Availability"]="Found";
      partMap["Results"]=result;
      partMap["part_id"] = partId;
      partInfoMap[partInfoMap.size()] = partMap;
    } 
    //util.printMap(partMap);
    

    return partInfoMap;
  }
  // public: map<int, map<std::string, std::string>>  getSpeakerInfo( map<string,string> itemPartInfo) {
  //   std::string speakerScript ="";

  //   string partId = itemPartInfo["item_part_id"];
  //   map<int, map<std::string, std::string>> partInfoMap;
  //   map<std::string, std::string> partMap;   
  //   string result=util.executeTerminal(speakerScript);
  //   if(result.find("Touchpad:") != std::string::npos) {
  //     if (result.length() > 140) {
  //       result = result.substr(0, 140);
  //     }
  //     partMap["name"]=result;
  //     partMap["part_id"] = partId;
  //     partInfoMap[partInfoMap.size()] = partMap;
  //   } 
  //   //util.printMap(partMap);


  //   return partInfoMap;
  // }
  public: map<int, map<std::string, std::string>> getPartInfo( map<string,string> itemPartInfo, string command){
    string partId = itemPartInfo["item_part_id"];
    map<int, map<std::string, std::string>> partInfoMap;
    map<std::string, std::string> partMap;
    
    int count = 0;
    // cout<<"\nCPU Info"<<"Started.."<<CMD_CPU<<"\n";
    // json jsonData= util.executeTerminalJSON1(command);
    string jsonresult=util.executeTerminal(command);
    std::istringstream inputstream(jsonresult);
    json jsonData= util.ExcetuteTerminalJSonArray1(inputstream);
  
    for(int i=0; i< jsonData.size(); i++) {
    try {
      string execFunction = itemPartInfo["exec_func"];
      for (const auto& item : jsonData[i].items()) {
        partMap[item.key()] = util.json2String(item.value());
        //util.printMap(partMap);
        partMap = partExecFunction(execFunction, partMap);
      }
        
      
    }
        catch (...) {
      logError("Error: ");
    }
    //util.printMap(partMap);
    partMap["part_id"] = partId;
    partInfoMap[partInfoMap.size()] = partMap;
    count++;
    }
    return partInfoMap;
  }
  
  // private: map<int, map<std::string, std::string>> getpartChildData(json jsonData, map<std::string, std::string> itemPartInfo, map<int, map<std::string, std::string>> partInfoMap) {
  //   int count = partInfoMap.size();
  //   string partId = itemPartInfo["item_part_id"];
  //   string execFunction = itemPartInfo["exec_func"];
  //   for (const auto& itemChild : jsonData.items()) {
  //     map<std::string, std::string> partMap;
  //     json partValue = itemChild.value();
  //     for (const auto& itemChildField : partValue.items()) {
  //       partMap[itemChildField.key()] = util.json2String(itemChildField.value());
  //       //std:cout << "Key" << itemChildField.key() << "::"   << itemChildField.value() <<"\n";
  //     }
  //     partMap = partExecFunction(execFunction, partMap);
  //     //std:cout << "Key" << itemChild.key() << "::" << partValue["description"] << "::"  << itemChild.value() <<"\n";
  //     //std::cout << partChildren[i].dump(2);
  //     partMap["part_id"] = partId;
  //     partInfoMap[count] = partMap;
  //     count++;
  //   }
  //   return partInfoMap;
  // }

  private: map<string, string> partExecFunction(std::string execFunction, map<std::string, std::string> partMap) {
    if (execFunction != "") {
      map<int, std::string> execFunctionSplitArray =  util.splitString(execFunction, "::");
      if (execFunctionSplitArray[0] == "split") {
        string partValue = partMap[execFunctionSplitArray[1]];
        map<int, std::string> partSplitValue = util.splitString(partMap[execFunctionSplitArray[1]], execFunctionSplitArray[2]);
        json jsonPartKey = json::parse(execFunctionSplitArray[3]);
        for (const auto& item : jsonPartKey.items()) {
          //std:cout << "Key" << item.key() << "::"   << item.value() <<"\n";
          partMap[item.value()] = partSplitValue[stoi(item.key())];
        }
        //util.printMap(partMap);
        //std::cout << partSplitValue[0] << "::" <<jsonPartKey["1"];
      }
      if (execFunctionSplitArray[0] == "exec") {
        //Need to work with exec variable
        if (execFunctionSplitArray[1] == "cpu") {partMap = getCPU(partMap);}
      }
    }
    return partMap;
  }

  // private: map<std::string, std::string> mergePartObject(json jsonObject, map<string, string> partMap) {
  //   for (const auto& item : jsonObject.items()) {
  //     partMap[item.key()] = util.json2String(item.value());
  //   }
  //   return partMap;
  // }

  private: map<std::string, std::string> getCPU(map<std::string, std::string> partMap) {
    map<int, std::string>splitStringMap = util.splitString(partMap["Name"], " ");
    string generation;
    if(partMap["Name"].find("AMD") == std::string::npos) {
    // cout<<splitStringMap[0] << " " << splitStringMap[1] << " " << splitStringMap[2] << " " << splitStringMap[3] << " " << splitStringMap[4] << " " << splitStringMap[5] << "\n";
    partMap["Vendor"] = splitStringMap[0];
    
    partMap["Currentspeed"] = splitStringMap[5];
    map<int, std::string>cpuTypeStringMap = util.splitString(splitStringMap[2], "-");
    partMap["Type"] = splitStringMap[1] + " " + cpuTypeStringMap[0];
    // int endPos = 2;
    generation=util.extractInteger(cpuTypeStringMap[1]);
    if(!generation.empty()){
    try{
    int gen=stoi(generation)/1000;
    generation=to_string(gen);
    }catch(std::exception const &e){
      cout<<"ERROR While getting the generation: "<<e.what()<<endl;
    }
    }
    // if (cpuTypeStringMap[1].length() == 5) {endPos = 1;}
    partMap["Generation"] = generation;
    partMap["Model"] =  cpuTypeStringMap[1];
    if(partMap["Name"].find("11th Gen") != std::string::npos){
      partMap["Generation"] = "11";
    }
    }else{
      partMap["Vendor"] = splitStringMap[0];
      map<int, std::string>cpuTypeStringMap;
    if(splitStringMap[1].find("-") != std::string::npos){
       cpuTypeStringMap= util.splitString(splitStringMap[1], "-");
       partMap["Generation"] = cpuTypeStringMap[1].substr(0, 1);
    }else if(splitStringMap[2].find("-") != std::string::npos){
       cpuTypeStringMap= util.splitString(splitStringMap[2], "-");
       partMap["Generation"] = cpuTypeStringMap[1].substr(0, 1);
    }else{
      partMap["Generation"] = splitStringMap[4].substr(0, 1);
    }
    }
    // cout<<partMap["vendor"] << " " << partMap["type"] << " " << partMap["generation"] << " " << partMap["model"] << "\n";   
    return partMap; 
  }

  // private: map<int, map<std::string, std::string>>  getPartInfo(string execCommand, string partId, string id = "", bool compare=false, string keyName ="id") {
  //   //std::cout << "\nPart Id : " << partId << " - Started" << "\n";
  //   map<int, map<std::string, std::string>> partInfoMap;
  //   json jsonObject = util.executeTerminal(execCommand, true);
  //   int count = 0;
  //   for (int i = 0; i < jsonObject.size(); ++i) {
  //     map<std::string, std::string> partMap;
  //     string idValue = jsonObject[i][keyName];
  //     idValue.erase(remove(idValue.begin(), idValue.end(), '\"'), idValue.end());
  //     //logInfo(jsonObject[i].dump(2));
  //     if (compare) {
  //       //if (idValue.find("cache:") == std::string::npos) {compare = true;}
  //       if (idValue.find("usb:") == std::string::npos) {compare = false;}
  //     } 
  //     logInfo(id + "-" + idValue + "-" + keyName + "-" + to_string(compare));
  //     if ((id == "" || id == idValue) && !compare) { 
  //       for (const auto& item : jsonObject[i].items()) {
  //         partMap[item.key()] = util.json2String(item.value()); //item.value().dump(0);
  //         map<std::string, std::string> partSubMap;
  //         if (jsonObject[i][item.key()].is_object()) {partSubMap = getJsonObjectInfo(jsonObject[i][item.key()]);}
  //         partMap.insert(partSubMap.begin(), partSubMap.end());
  //       }
  //       partMap["part_id"] = partId;
  //       partInfoMap[count] = partMap;
  //       count++;
  //     }
  //   }
  //   return partInfoMap;
  // }

  // private: map<std::string, std::string> getJsonObjectInfo(json jsonSubObject) {
  //   map<std::string, std::string> partSubMap;
  //   for (const auto& item : jsonSubObject.items()) {
  //     partSubMap[item.key()] = util.json2String(item.value()); //item.value().dump(0);
  //   }
  //   return partSubMap;
  // }

  // public: void getKeyboardInfo() {
  // }

  private: void addPartConfig(map<int, map<string, string>> partConfigMap) {
    int itemPartConfigSize = allPartConfigMap.size();
    for (int i = 0; i < partConfigMap.size(); ++i) {
      allPartConfigMap[itemPartConfigSize + i] = partConfigMap[i];
    }
  }




  
};

vector<string> HardwareInfoService::part_list_item_groups;