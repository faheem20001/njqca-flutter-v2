#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <../service/check/BluetoothCheck.h>
#include <../service/check/StorageCheck.h>
#include <../service/check/wifiCheck.h>
#include <../service/check/BatteryCheck.h>
#include <../service/check/HDDSentinelCheck.h>
#include <../service/check/DriverCheck.h>

#include <../service/check/RamSpeedCheck.h>
#include <../service/check/CPUTempCheck.h>
#include <../service/check/CMOSErrorCheck.h>
#include <../service/check/CompleteBatteryCheck.h>
#include <../service/check/SystemCheck.h>
#include <../service/check/performanceCheck.h>
// #include <../service/check/CPUFanSpeedCheck.h>
#include <../service/check/HDMIandVGACheck.h>
#include <../service/check/LidCheck.h>
#include <../service/check/CameraCheck.h>
#include <../service/check/fingerprintCheck.h>
#include <../service/check/hotkeys.h>
#include <../service/check/AudioSimilarityCheck.h>
#include <../service/check/password.h>
#include <../service/check/AdapterTest.h>
#include <../service/check/TypeC_chargerCheck.h>
#include <../service/check/USBcheck.h>
#include <../service/check/product_key.h>
#include <../service/check/GeekBenchTest.h>
#include <../service/check/vibrationCheck.h>
#include <../service/check/TouchPadButton_Check.h>
#include <mutex>
#include <../lib/freshweb/emit.h>

using namespace std;

class PartTestService: public V2Service {

  private:
    const string CMD_STORAGE = "smartctl -A -j /dev/nvme0n1";
    BluetoothCheck bluetoothcheck;
    StorageCheck storageCheck;
    WifiCheck wificheck;
    LanCheck lancheck;
    BatteryCheck batterycheck;
    HDDSentinelCheck hddsentinelcheck;
    DriverCheck drivercheck;
    SystemCheck systemcheck;
    LidCheck lidcheck;
    CPUTempCheck tempcheck;
    RAMspeedCheck RamspeedCheck;
    CMOSErrorCheck cmosErrorcheck;
    IQCService iqcService;
    Constant constant;
    GetFromERPService getfromERP;
    DocType doctype;
    PerformanceCheck performance;
    CameraCheck cameracheck;
    GeekBenchCheck geekbenchcheck;
    FingerprintCheck fingerprintcheck;
    AdapterCheck Adaptercheck;  
    PartResultService partresultservice;

    // IQCService iqcService;
    // Constant constant;
    // CPUFanSpeedCheck FanCheck;
    USBcheck usb;
    HDMI_VGA_PortCheck HDMI_VGA;
    PartConfigService partconfigservice;
    map<int,map<string,string>> mapingfilds;
    std::thread CPUfanThread;
    std::thread performanceThread;
    HotkeysCheck hotkeys;
    SpeakerTest speakercheck;
    TypeC_chargerCheck typeccheck;
    ProductKeyCheck productkeycheck;
    VibrationCheck vibrationcheck;
    std::mutex io_mutex;
    public:
    static string loggedUser;
    static string selected_inspection_type;

  public: int runAllTest() {
    logInfo("Run test...");
    return storageCheck.getTestResult(0,0,0,0);
  }
  std::string getParameter() {
    json jsonData;

    std::map<std::string, std::string> requestMap;
    jsonData ["user"]=loggedUser;
    jsonData["bios_serial_no"]=iqcService.bios_serial_no;
    jsonData["selected_inspection_type"]=selected_inspection_type;
    // cout<<loggedUser<<endl;
    std::string response = iqcService.getParameterList(constant.API_PARAMETER_URL, "post", requestMap, to_string(jsonData));
    cout << "Parameters To RUN the Test:"<< response <<endl;

    return response;
  }

   void StartAllChecks(string stockinfoId){

    int disk_count=0;
    string result=getParameter();
    // result="Performance_status";
    // result= "y_cruncher_status";

    update_features();    // json cascading_result=get_cascading_results();
      int battery_count=0;
      int RAMcount=0;
      mapingfilds= partconfigservice.getPartConfigList(stockinfoId);
      for(const auto& partsdata:mapingfilds){
        for(const auto&partdata: partsdata.second){
          if(partdata.first=="njhat_item_name" && partdata.second=="Battery"){
            battery_count+=1;
          }
        }
      }
      bool run_battery_test=update_battery_result(stockinfoId,"11","1",result);
      if(battery_count!=0 and run_battery_test ){
        nj_emit({{"event","test_start"},{"test","Battery_status"},{"label","Battery Test"},{"part","Battery"}});
        batterycheck.Battery_test(mapingfilds,battery_count,result);
        nj_emit({{"event","test_done"},{"test","Battery_status"},{"status","done"}});
      }
        
      
      
      // performance.PerformanceTest(stockinfoId,"5","1");

      map<int, map<string, string>> ::iterator it = mapingfilds.begin();
       
      while(it!=mapingfilds.end()){
        map<int, map<string, string>> partInfoList;
        map<string, string> itemPartname = it->second;
        string partname = itemPartname["njhat_item_name"];


        // cout<<partname<<endl;
        if(partname=="RAM" && hddsentinelcheck.runHDDTest(result)){
              nj_emit({{"event","test_start"},{"test","y_cruncher_status"},{"label","Y-Cruncher"},{"part","RAM"}});
              hddsentinelcheck.y_cruncher_test(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"],result,mapingfilds);
              nj_emit({{"event","test_done"},{"test","y_cruncher_status"},{"status","done"}});
            }
            
        if(partname=="CPU" && geekbenchcheck.runGeekBenchTest(result)){
          nj_emit({{"event","test_start"},{"test","GeekBench_status"},{"label","GeekBench"},{"part","CPU"}});
          geekbenchcheck.GeekBenchTest(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["item_id"],itemPartname["created_by"]);
          nj_emit({{"event","test_done"},{"test","GeekBench_status"},{"status","done"}});
        }
        // if(partname=="Battery"){cmosErrorcheck.CMOSErrortest(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"]);}
        if(partname=="Storage" && hddsentinelcheck.runHDDTest(result) && disk_count==0){
          disk_count+=1;
          nj_emit({{"event","test_start"},{"test","HDD_Sentinel_Status"},{"label","HDD Sentinel"},{"part","Storage"}});
          hddsentinelcheck.HDDSentinel_test(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"],result,mapingfilds);
          nj_emit({{"event","test_done"},{"test","HDD_Sentinel_Status"},{"status","done"}});
          }
        if(partname=="RAM" && RAMcount==0){
          if ( RamspeedCheck.runRAMTest(result)){
            nj_emit({{"event","test_start"},{"test","RAM_speed_status"},{"label","RAM Speed"},{"part","RAM"}});
            RamspeedCheck.RAMspeedTest(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"],mapingfilds);
            nj_emit({{"event","test_done"},{"test","RAM_speed_status"},{"status","done"}});
          }
          RAMcount+=1;
          if(performance.runperformanceTest(result)){
            nj_emit({{"event","test_start"},{"test","Performance_status"},{"label","Performance"},{"part","RAM"}});
            performance.PerformanceTest(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["item_id"]);
            nj_emit({{"event","test_done"},{"test","Performance_status"},{"status","done"}});
          }
        }
        // if(partname=="CPU"){tempcheck.perform_temp_check(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"]);}
        
        if(partname=="Motherboard"){
        productkeycheck.Product_keyTest(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"]);
        if(vibrationcheck.runVibrationTest(result)){
          vibrationcheck.VibrationTest(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"]);
        }

        if(speakercheck.runSpeakerTest(result)){
          system("start ms-settings:sound");
          util.generatePopupOK("Please adjust the settings (Select external microphone and default speaker) for Speaker Test Then click OK?");
          speakercheck.Speaker(itemPartname["stock_info_id"],"23",itemPartname["item_id"]);
        }
        if(cameracheck.runCameraTest(result)){
         cameracheck.CameraTest(itemPartname["stock_info_id"],"19",itemPartname["item_id"],itemPartname["created_by"]);
        }
        // if(cmosErrorcheck.runCMOSTest(result)){
        // cmosErrorcheck.CMOSErrortest(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"]);
        // }
        if(systemcheck.runsoftwareTest(result)){
          nj_emit({{"event","test_start"},{"test","System_status"},{"label","System Check"},{"part","Motherboard"}});
          systemcheck.systemsoftwareTest(itemPartname["stock_info_id"],"22",itemPartname["item_id"],itemPartname["created_by"],result);
          nj_emit({{"event","test_done"},{"test","System_status"},{"status","done"}});
        }
        // if(performance.runperformanceTest(result)){
        //   performance.PerformanceTest(itemPartname["stock_info_id"],"22",itemPartname["item_id"]);
        // }
        // CPUfanThread=std::thread(&CPUFanSpeedCheck::FanSpeedTest,&FanCheck,itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"]);
        if(HDMI_VGA.runHDMITest(result)){
          nj_emit({{"event","test_start"},{"test","HDMI_Status"},{"label","HDMI / VGA Test"},{"part","Motherboard"}});
          HDMI_VGA.HDMI_VGA_PortTest(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"],result);
          nj_emit({{"event","test_done"},{"test","HDMI_Status"},{"status","done"}});
        }
        if(usb.runUSBTest(result)){
          nj_emit({{"event","test_start"},{"test","USB_status"},{"label","USB Port Test"},{"part","Motherboard"}});
          usb.USBport_test(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["item_id"],itemPartname["created_by"]);
          nj_emit({{"event","test_done"},{"test","USB_status"},{"status","done"}});
        }
        if(hotkeys.runHotkeyTest(result)){
          nj_emit({{"event","test_start"},{"test","Hotkeys_status"},{"label","Hotkeys"},{"part","Motherboard"}});
          hotkeys.HotkeyTest(itemPartname["stock_info_id"],"22",itemPartname["item_id"],itemPartname["created_by"]);
          nj_emit({{"event","test_done"},{"test","Hotkeys_status"},{"status","done"}});
        }
        if(drivercheck.runDriverTest(result)){
          nj_emit({{"event","test_start"},{"test","Driver_status"},{"label","Driver Check"},{"part","Motherboard"}});
          drivercheck.drivertest(itemPartname["stock_info_id"],"22",itemPartname["item_id"],itemPartname["created_by"]);
          nj_emit({{"event","test_done"},{"test","Driver_status"},{"status","done"}});
        }

        if(lancheck.runLANTest(result)){
          nj_emit({{"event","test_start"},{"test","LAN_status"},{"label","LAN Check"},{"part","Motherboard"}});
          lancheck.LAN_test(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"]);
          nj_emit({{"event","test_done"},{"test","LAN_status"},{"status","done"}});
        }
        }
        if(partname=="WIFI" && wificheck.runwifiTest(result)){
          nj_emit({{"event","test_start"},{"test","Wifi_status"},{"label","WiFi Check"},{"part","WIFI"}});
          wificheck.wifi_test(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"]);
          nj_emit({{"event","test_done"},{"test","Wifi_status"},{"status","done"}});
        }
        // if(partname=="Network"){lancheck.LAN_test(itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"]);}
      //    if(partname=="Battery"){
      //  t=std::thread(&BatteryCheck::Battery_test, &batterycheck,itemPartname["stock_info_id"],itemPartname["stock_part_id"],itemPartname["part_config_id"],itemPartname["item_id"],itemPartname["created_by"],itemPartname["attr_12"]);}
        // if(partname=="Motherboard"&&count==0){usb.USBport_test(itemPartname["stock_info_id"],"16",itemPartname["item_id"],itemPartname["created_by"]);
        // count++;
        // }
        if(partname=="Bluetooth" && bluetoothcheck.runBlutoothTest(result)){
          nj_emit({{"event","test_start"},{"test","Bluetooth_status"},{"label","Bluetooth Test"},{"part","Bluetooth"}});
          bluetoothcheck.BluetoothTest(itemPartname["stock_info_id"],"15",itemPartname["item_id"],itemPartname["created_by"]);
          nj_emit({{"event","test_done"},{"test","Bluetooth_status"},{"status","done"}});
        }
        ++it;
      }
      // if(CPUfanThread.joinable()){CPUfanThread.join();} 
      batterycheck.joindischargeThread(); 
      if(Adaptercheck.runAdapterTest(result))  {
        nj_emit({{"event","test_start"},{"test","Adapter_status"},{"label","Adapter Test"},{"part","Motherboard"}});
        Adaptercheck.AdapterTest(stockinfoId,"24","1");
        nj_emit({{"event","test_done"},{"test","Adapter_status"},{"status","done"}});
      }
      // if(performance.runperformanceTest(result)){
      //   systemcheck.performBatteryDischargeCheck(stockinfoId,"5","1");
      // }

      if(typeccheck.runTypeC_check(result)){
        nj_emit({{"event","test_start"},{"test","TypeC_status"},{"label","Type-C Charger"},{"part","Motherboard"}});
        typeccheck.TypeC_test(stockinfoId,"5","1");
        nj_emit({{"event","test_done"},{"test","TypeC_status"},{"status","done"}});
      }
      
      if(fingerprintcheck.runFingerTest(result)){
          nj_emit({{"event","test_start"},{"test","Fingerprint_status"},{"label","Fingerprint"},{"part","Motherboard"}});
          fingerprintcheck.FingerprintTest(stockinfoId,"5","1");
          nj_emit({{"event","test_done"},{"test","Fingerprint_status"},{"status","done"}});
      }

      if(lidcheck.runLIDTest(result)){
          lidcheck.Lid_test(stockinfoId,"5","1");
      }

      if(result.find("Battery_charge_percentage_status") != std::string::npos){
        nj_emit({{"event","test_start"},{"test","Battery_charge_percentage_status"},{"label","Battery Charge %"},{"part","Battery"}});
        batterycheck.battery_charge_test(stockinfoId,"11","1");
        nj_emit({{"event","test_done"},{"test","Battery_charge_percentage_status"},{"status","done"}});
      }
    }  

  private: map<std::string, std::string> getJsonObjectInfo(json jsonSubObject) {
    map<std::string, std::string> partSubMap;
    for (const auto& item : jsonSubObject.items()) {
      partSubMap[item.key()] = item.value().dump(0);
    }
    return partSubMap;
  }

  public:void  startNJPULSE_Test(){
    bool runLoop=true;
    Sleep(50000);
    while(runLoop){
      
    
    string response="";
    json jsonObject;
    map<string, string> requestMap;
    requestMap["filters"] ="[[\"name\", \"=\", \"" + iqcService.config_updated_name + "\"]]";
    requestMap["filters"]=util.encodeURL(requestMap["filters"]);
    requestMap["fields"] = "[\"qi_name\"]";
    // std::string QI_Url = constant.apiBaseURL+"api/resource/NJQCA?fields=[\"qi_name\"]&filters=[[\"name\", \"=\", \"" + iqcService.config_updated_name + "\"]]";
    // cout<<QI_Url<<endl;
    // jsonObject["parent"]=iqcService.config_updated_name;
    response =getfromERP.api2ERP(constant.API_DOC_URL+doctype.IQC,requestMap);
    logInfo(response);
    cout<<response<<endl;
    if(response!=""){
      jsonObject = json::parse(response);
      cout<<jsonObject.dump(0)<<endl;
      json dataarray=jsonObject["data"];
      if(dataarray["qi_name"]){
        continue;
        
      } else{
          break;
      }
  }
  }
  }
   
  void update_features(){
    int USBPort_count=0;
    bool vga_available=false;
    bool hdmi_available=true;
    bool sdcard_available=false;
    bool typec_available=false;
    bool dp_available=false;
    bool lan_available=true;
    int TypeC_count=0;
    string windows_version="";

    json jsonData_feature;
    string port_number="";  
    string type_c_no="";
    string item_wizard="";
    string model="";
    vector<string> ERP_model_list;
    string bios_Serial_no=iqcService.bios_serial_no;
    // string bios_Serial_no="pf1";
    item_wizard=get_iw(bios_Serial_no,"");
    if (item_wizard==""){
        ERP_model_list=model_list();
        model=get_model_name(ERP_model_list);
        item_wizard=get_iw("",model);
        if(item_wizard.empty()){
          while(true){
            bool status=util.generatePopup("Item Wizard Not Found","If you want to Change the Model click Retry else Click Not Working?");
            if(status){
              break;
            }else{
              model=get_model_name(ERP_model_list);
              item_wizard=get_iw("",model);
              if(item_wizard!=""){
                break;
              }
            }
          }
          
        }
      }
      // cout<<"model size:"<<ERP_model_list.size()<<endl;
      // cout<<"Item Wizard:"<<item_wizard<<endl;
      // cout<<"model:"<<model<<endl;


      if(item_wizard!=""){
      jsonData_feature=get_features_data(item_wizard);
      for(int j=0;j<jsonData_feature.size();j++){
      try{
      if(jsonData_feature[j]["features_attributes"]=="USB"){
          port_number=jsonData_feature[j]["values"];
          USBPort_count=stoi(port_number);
      }
      else if(jsonData_feature[j]["features_attributes"]=="VGA"){
          if(jsonData_feature[j]["availability"]=="Yes"){
            vga_available=true;
          }
      }
      else if(jsonData_feature[j]["features_attributes"]=="HDMI"){
          if(jsonData_feature[j]["availability"]!="Yes"){
            hdmi_available=false;
          }
      }
      else if(jsonData_feature[j]["features_attributes"]=="SD CARD SLOT"){
          if(jsonData_feature[j]["availability"]=="Yes"){
            sdcard_available=true;
          }
      }
      else if(jsonData_feature[j]["features_attributes"]=="Thunderbolt - TYPE C"){
          if(jsonData_feature[j]["availability"]=="Yes"){
            // typec_available=true;
            type_c_no=jsonData_feature[j]["values"];
            TypeC_count+=stoi(type_c_no);
            type_c_no="";
          }
      }
      else if(jsonData_feature[j]["features_attributes"]=="DP"){
          if(jsonData_feature[j]["availability"]=="Yes"){
            dp_available=true;
          }
      }
      else if(jsonData_feature[j]["features_attributes"]=="LAN PORT"){
          if(jsonData_feature[j]["availability"]!="Yes"){
            lan_available=false;
          }
      }
      else if(jsonData_feature[j]["features_attributes"]=="TYPE C"){
          if(jsonData_feature[j]["availability"] =="Yes"){
            type_c_no=jsonData_feature[j]["values"];
            TypeC_count+=stoi(type_c_no);
            type_c_no="";
          }
      }
      else if(jsonData_feature[j]["features_attributes"]=="Windows Type"){
          if(jsonData_feature[j]["availability"] =="Yes"){
            windows_version=jsonData_feature[j]["values"];           
          }
      }
      }catch(exception &e){
          cout<<"ERROR for features_attribute: "<<e.what()<<endl;
      }     
     }
    }
    productkeycheck.Selected_item_wizard=item_wizard;
    usb.Selected_model=model;
    usb.USBPort_count=USBPort_count;
    HDMI_VGA.VGA_available=vga_available;
    HDMI_VGA.HDMI_available=hdmi_available;
    HDMI_VGA.DP_available= dp_available;
    HDMI_VGA.sd_card_available=sdcard_available;
    typeccheck.tunderbolt_available=typec_available;
    typeccheck.no_of_typeC_port=TypeC_count;
    lancheck.lan_port_available=lan_available;
    systemcheck.windows_version=windows_version;
  }

  vector<string> model_list(){
      vector<string> ERP_model_list;
      std::map<std::string, std::string> requestMap;
      json manufacturer_model;
      manufacturer_model["manufacturer_model"]=iqcService.model_name; 
      string model_list_response=iqcService.getParameterList(constant.API_model_list, "PUT", requestMap, to_string(manufacturer_model));
      if(model_list_response!=""){
        try{
          json model_list=json::parse(model_list_response);
          if(model_list.at("message")!=""){
            model_list=model_list.at("message");
            for(int i=0;i<model_list.size();i++){
              ERP_model_list.push_back(model_list[i]["model_name"]);
            }
          }
        }catch(const exception& e){
          cout<<"Error while getting the model list"<<e.what()<<model_list_response<<endl;
        }
      }
      return ERP_model_list;
    }
    
   string get_model_name(vector<string>& ERP_model_list){
     string model;
     model=util.selected_option(ERP_model_list);
     return model;
   }
   
   string get_iw(string bios_serial_no="",string model=""){
    string iw;
    std::map<std::string, std::string> requestMap;
    json input;
    input["model"]=model; 
    input["bios_serial_no"]=bios_serial_no;
    string iw_result=iqcService.getParameterList(constant.API_item_wizard, "PUT", requestMap, to_string(input));
    if(iw_result!=""){
      try{
        json iw_response=json::parse(iw_result);
        if(iw_response.at("message")!=""){
          iw=iw_response.at("message");
        }
      }catch(const exception& e){
        cout<<"Error while getting Item Wizard"<<e.what()<<iw_result<<endl;
      }
    }
    return iw;
   }

  private: json get_features_data(string iw){
    json features_data;
    std::map<std::string, std::string> requestMap;
    json input;
    input["iw"]=iw; 
    string iw_result=iqcService.getParameterList(constant.API_features, "PUT", requestMap, to_string(input));
    if(iw_result.empty()){
       Sleep(2000);
       iw_result=iqcService.getParameterList(constant.API_features, "PUT", requestMap, to_string(input));
    }
    cout<<"Features data: "<<iw_result<<endl;
    if(iw_result!=""){
      try{
        json iw_response=json::parse(iw_result);
        if(iw_response.at("message")!=""){
          features_data=iw_response.at("message");
        }
      }catch(const exception& e){
        cout<<"Error while getting the features data"<<e.what()<<iw_result<<endl;
      }
    }
    return features_data;
  }
  public : bool update_battery_result(string stock_info_id, string part_id, string item_id,string parameters){
    map<string, string> paramtrmap;
    map<string, string> fieldidmap;
    bool return_value=true;

    if(parameters.find("Battery_detection_status")!=std::string::npos){
        const string command = "powershell -Command \" Get-WmiObject -Class Win32_Battery | Format-List Caption,Description,DeviceID,DesignVoltage,Name\"";
        string result=util.executeTerminal(command);
        result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
        std::istringstream inputstream(result);
        json jsonData= util.ExcetuteTerminalJSonArray1(inputstream);
        if (result.empty()|| jsonData.size() == 0) {
           paramtrmap["Battery_detection_status"] = "FAIL";
           paramtrmap["Charging_port_status"] = "PASS";
           return_value=false;      
        }else{
          paramtrmap["Battery_detection_status"] = "PASS";
        }
        fieldidmap["stock_info_id"] = stock_info_id;
        fieldidmap["part_id"] = part_id;
        fieldidmap["item_id"] = item_id;
        partresultservice.updateBulkPartResult(paramtrmap, fieldidmap);
        iqcService.ChecksResultSync("Battery detction Check");
        
    }
    return return_value;

  }
  
};
string PartTestService::loggedUser;
string PartTestService::selected_inspection_type;

