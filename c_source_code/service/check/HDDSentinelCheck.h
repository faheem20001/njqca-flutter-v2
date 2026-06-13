#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
#include <stdexcept>
#include <cstdint>
namespace fs = std::filesystem;

class HDDSentinelCheck: public V2Service {

    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;
    
    GetFromERPService getfromERP;
    NjsettingsService njsettings;
    public:
     static int passing_health;
     static int dignostic_passing_health;
     static string qi_name;
     static string heavy_load_result;
     static bool run_test;
     static bool is_component;
     static int heavy_load_temp;
     static bool run_y_cruncher_test;
     static string y_cruncher_qi_name;

     map<string,string> parametrmap;

    public: int HDDSentinel_test(string stockinfoID,string partId,string partconfigid, string item_id,string created_by,string parameters,map<int,map<string,string>>config_data){
        cout<<"\nHDD Sentinel Check Running....."<<endl;
        system("powershell -Command \"Stop-Process -Name HDSentinel -Force -ErrorAction SilentlyContinue\"");
        Sleep(2000);
        deletefile("Hard Disk Sentinel/HDSentinel_6.40 PRO_report.txt");
        string removetask="cmd /c cscript \"Hard Disk Sentinel\\removehds7.vbs\"";
        string reportcommand="start \"\" \"Hard Disk Sentinel\\HDSentinel.exe\" /REPORT";
        string KillProcess="powershell -Command \"Stop-Process -Name HDSentinel -Force -ErrorAction SilentlyContinue\"";
        map<string,string> fieldmap; 
        fieldmap["stock_info_id"]=stockinfoID;
        fieldmap["part_id"]=partId;
        fieldmap["part_config_id"]=partconfigid;
        fieldmap["item_id"]=item_id;
        fieldmap["created_by"]=created_by;
        // string burnincommand="burn.exe";
        int return_value=0;
        // string SerialNo;
        // map<std::string, std::string> systemInfoMap;
        // const string CMD_SYSTEM= "powershell -Command \"Get-WmiObject -Class Win32_computersystemproduct\"";
        // // const string CMD_SYSTEM = "wmic csproduct get /format:list";
        // json jsonObject  = util.executeTerminalJSON(CMD_SYSTEM,":");
    
        // for (const auto& item : jsonObject.items()) {
        //   //Get product details
        //     if (item.key() == "IdentifyingNumber") {SerialNo = item.value();
            
        //     break;
        //     }
        // }
        
        // // string result=util.executeTerminal("powershell.exe -Command \"& '.\\Hard Disk Sentinel\\HDSentinel.exe' /REPORT\"");
        // // cout<<reportcommand<<endl;
        // string jsonresult=util.executeTerminal(burnincommand);
        // std::replace(jsonresult.begin(), jsonresult.end(), '\'', '"');
        // // cout<<jsonresult<<endl;
        // parametrmap["heavy_load_status"]="File Not Found";
        // json result=json::parse(jsonresult);
        // if(!result.empty()){
        // parametrmap["heavy_load_status"]="PASS";
        
        // if (result.contains("System Info") && result["System Info"].contains("BIOS Serial Number")) {
        //     if(SerialNo==result["System Info"]["BIOS Serial Number"]){
        //         cout<<"Bio serial number matched report"<<endl;
        //     }
        //     else{
        //         parametrmap["Burning_status"]="FAIL";
        //         parametrmap["Burning_error"]="Bio serial number not matched in the report";
        //         cout<<"Bio serial number not matched in the report"<<"system serial number:"<<SerialNo<<" report serial number:"<<result["System Info"]["BIOS Serial Number"]<<endl;
        //     }
        // } 
        // if(parametrmap["Burning_status"]=="PASS"){
            
        // if (result.contains("System Info")) {
        //     checkTemperatures(result["System Info"]);
        // }
        // }
        // if(parametrmap["Burning_status"]=="PASS"){
        //           if (result.contains("Test Results")) {
        //     checkTestResults(result["Test Results"]);
        // }  
        // }

        // }
        if(parameters.find("heavy_load_status")!=string::npos){
            if (!run_test) {
                cout << "\nHeavy Load Test checking..." << endl;

                const char* userProfile = std::getenv("USERPROFILE");
                if (userProfile != nullptr) {
                    std::string oldReportPath = std::string(userProfile) + "\\Desktop\\HeavyLoad_Report.csv";

                    try {
                        if (fs::exists(oldReportPath)) {
                            fs::remove(oldReportPath);
                            cout << "Old HeavyLoad report deleted before fresh run: " << oldReportPath << endl;
                        }
                    } catch (const std::exception& e) {
                        cout << "Unable to delete old HeavyLoad report: " << e.what() << endl;
                    }
                }

                heavy_load_result = util.executeTerminal("run_heavy_load.exe");
            }
            heavy_load_result.erase(std::remove(heavy_load_result.begin(), heavy_load_result.end(), '\n'), heavy_load_result.end());

            cout<<"heavy load status:"<<heavy_load_result<<endl;
            if(run_test){
                parametrmap["heavy_load_status"]="PASS";
                parametrmap["QI_Reference"]=qi_name;
            }
            if(parametrmap.find("heavy_load_status")==parametrmap.end()){
                const char* userProfile = std::getenv("USERPROFILE");
                if (userProfile != nullptr) {
                    std::string path(userProfile);
                    std::string fullPath = path + "\\Desktop\\HeavyLoad_Report.csv";
                    bool fileExists = file_exist(fullPath);
                    if(fileExists){
                    GetFromERPService getfromERP;
                    string File_updated_name=getfromERP.updateFile2ERp(fullPath.c_str());
                    if(File_updated_name!=""){
                        parametrmap["heavy_load_File_Updated_name"]=File_updated_name;
                    }else{
                        parametrmap["heavy_load_File_Updated_name"]="Not able to update the file";
                    }
                    }else{
                        parametrmap["heavy_load_File_Updated_name"]="File Not Found";
                    }
                }
                int current_temp=0;
                string result_temp=extractInteger_only(heavy_load_result);
                if (result_temp.length()>0){
                    current_temp=stoi(result_temp);
                }
                parametrmap["heavy_load_actual_temperature"] = to_string(current_temp);

                if(heavy_load_result.length()>140){
                    heavy_load_result=heavy_load_result.substr(0,140);
                }
                parametrmap["heavy_load_result"]=heavy_load_result;
            }
            return_value= partresultservice.updateBulkPartResult(parametrmap,fieldmap);
            parametrmap.clear();
        }
        if(parameters.find("HDD_Sentinel_Status")!=string::npos){
                
            parametrmap["Health"]="Not Found";
            bool RunLoop=true;
            system(reportcommand.c_str());
            auto start = std::chrono::steady_clock::now();
            while(RunLoop){
                if(fs::exists("Hard Disk Sentinel/HDSentinel_6.40 PRO_report.txt")){
                    Sleep(3000);
                    RunLoop=false;
                    break;
                }
                Sleep(2000);
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
                if (duration.count() > 120) {
                    RunLoop= false;
                }
            }
            parametrmap["HDD_Sentinel_Screen_shot"]=take_screen_shot();
            // this_thread::sleep_for(chrono::minutes(1));
            // Sleep(11000);
            cout<<"\nKilling the Process..."<<endl;
            system(KillProcess.c_str());
            map<int , map<string, string>> HddData= data();
            // cout<<"size:"<<HddData.size()<<endl;        
            // exit(0);
            map<int, map<string, string>> ::iterator object = config_data.begin();
            while(object!=config_data.end()){
                map<string, string> partdata = object->second;
                string partname=partdata["njhat_item_name"];        
                if(partname=="Storage"){
                    if(!HddData.empty()){
                        for (int item=0 ;item<HddData.size();item++){
                            if(HddData[item]["Device Type"].find("Fixed")!=std::string::npos && !is_component){
                                int hdd_size_mb;
                                try{
                                 hdd_size_mb=stoi(extractInteger(HddData[item]["Total Size"]));}
                                catch(const std::exception& e) {
                                    cout << "Error while getting storage size from hdd sentinel file: " << e.what() << endl;
                                    continue;
                                }
                                int hdd_size_gb;
                                int act_size;
                                if(hdd_size_mb){
                                   hdd_size_gb=(hdd_size_mb*1.048576)/1000;
                                }
                                try{
                                act_size=stoi(extractInteger(partdata["attr_03"]));}
                                catch(const std::exception& e) {
                                    cout << "Error while getting storage size from device: " << e.what() << endl;
                                    continue;
                                }
                                if(abs(hdd_size_gb-act_size)>5){
                                    continue;
                                } 
                                fieldmap["part_config_id"]=partdata["part_config_id"];                               
                                parametrmap["Health"]=extractInteger(HddData[item]["Health"]);
                                parametrmap["Performance"]=extractInteger(HddData[item]["Performance"]);
                                parametrmap["Temperature"]=extractInteger(HddData[item]["Current Temperature"]);
                                parametrmap["Status"]=HddData[item]["status"];
                            }else if(HddData[item]["Device Type"].find("Removable")!=std::string::npos && is_component ){
                                if(extractInteger(HddData[item]["Health"]).empty()){
                                    continue;
                                }
                                parametrmap["Health"]=extractInteger(HddData[item]["Health"]);
                                parametrmap["Performance"]=extractInteger(HddData[item]["Performance"]);
                                parametrmap["Temperature"]=extractInteger(HddData[item]["Current Temperature"]);
                                parametrmap["Status"]=HddData[item]["status"];
                                
                            }
                        }
                    }
                    if(parametrmap["Health"]!="Not Found"){
                        int health=stoi(parametrmap["Health"]);
                        updateScore("Health",parametrmap["Health"],passing_health,fieldmap);
                        parametrmap.erase("Health");
                        
                        if(health>=dignostic_passing_health){
                            if( parametrmap["Status"].find("bad sectors")!=std::string::npos){
                                int badsectors=0;
                                try{
                                badsectors=stoi( util.extractInteger(parametrmap["Status"]));
                                //  cout<<"badsectors:"<<badsectors<<endl;
                                }catch( exception &e){
                                cout<<"Not able to fetch bad sector count"<<endl;
                                }
                            if(badsectors>5){                    
                            parametrmap["HDD_Sentinel_Status"]="FAIL";
                        }else{
                            parametrmap["HDD_Sentinel_Status"]="PASS"; 
                        }
                        }else{
                            parametrmap["HDD_Sentinel_Status"]="PASS"; 
                        }
                        }else{
                            parametrmap["HDD_Sentinel_Status"]="FAIL";
                        }
                        return_value= partresultservice.updateBulkPartResult(parametrmap,fieldmap);
                        parametrmap.clear();
                    }
                    
                }
                ++object;
            }
            // if(!HddData.empty()){
            // for(auto items:HddData){
            //     for(auto item:items.second){                                
            //         if(item.first=="Device Type" && item.second.find("Fixed")!=std::string::npos){
            //             parametrmap["Health"]=extractInteger(item["Health"]);
            //             parametrmap["Performance"]=extractInteger(HddData["Performance"]);
            //             parametrmap["Temperature"]=extractInteger(HddData["Current Temperature"]);
            //             parametrmap["Status"]=HddData["status"];
                        
            //         }
            //     }
            // }
            // }

            // if(!HddData.empty()){
            //     for (int item=0 ;item<HddData.size();item++){
            //         if(HddData[item]["Device Type"].find("Fixed")!=std::string::npos && !is_component){
            //             parametrmap["Health"]=extractInteger(HddData[item]["Health"]);
            //             parametrmap["Performance"]=extractInteger(HddData[item]["Performance"]);
            //             parametrmap["Temperature"]=extractInteger(HddData[item]["Current Temperature"]);
            //             parametrmap["Status"]=HddData[item]["status"];
            //         }else if(HddData[item]["Device Type"].find("Removable")!=std::string::npos && is_component ){
            //             if(extractInteger(HddData[item]["Health"]).empty()){
            //                 continue;
            //             }
            //             parametrmap["Health"]=extractInteger(HddData[item]["Health"]);
            //             parametrmap["Performance"]=extractInteger(HddData[item]["Performance"]);
            //             parametrmap["Temperature"]=extractInteger(HddData[item]["Current Temperature"]);
            //             parametrmap["Status"]=HddData[item]["status"];
                        
            //         }
            //     }
            // }
           

            // if(parametrmap["Health"]!="Not Found"){
            //     int health=stoi(parametrmap["Health"]);
            //     updateScore("Health",parametrmap["Health"],passing_health,fieldmap);
            //     parametrmap.erase("Health");
                
            //     if(health>=dignostic_passing_health){
            //         if( parametrmap["Status"].find("bad sectors")!=std::string::npos){
            //             int badsectors=0;
            //             try{
            //             badsectors=stoi( util.extractInteger(parametrmap["Status"]));
            //             //  cout<<"badsectors:"<<badsectors<<endl;
            //             }catch( exception &e){
            //             cout<<"Not able to fetch bad sector count"<<endl;
            //             }
            //         if(badsectors>5){                    
            //         parametrmap["HDD_Sentinel_Status"]="FAIL";
            //     }else{
            //         parametrmap["HDD_Sentinel_Status"]="PASS"; 
            //     }
            //     }else{
            //         parametrmap["HDD_Sentinel_Status"]="PASS"; 
            //     }
            //     }else{
            //         parametrmap["HDD_Sentinel_Status"]="FAIL";
            //     }
            // }

        }
 
        // }
        iqcservice.ChecksResultSync("HDDSentinel Check");    
        string removestrin=util.executeTerminal(removetask);
        // cout<<removestrin<<endl;
        cout<<"\nHDDSentinel check is Completed..."<<endl;
        return return_value;
        
    }
    // y_cruncher_test METHOD:

    public: int y_cruncher_test(string stockinfoID, string partId, string partconfigid, 
                                string item_id, string created_by, string parameters, 
                                map<int,map<string,string>> config_data) {
        
        map<string,string> fieldmap; 
        fieldmap["stock_info_id"] = stockinfoID;
        fieldmap["part_id"] = partId;
        fieldmap["part_config_id"] = partconfigid;
        fieldmap["item_id"] = item_id;
        fieldmap["created_by"] = created_by;
        
        int return_value = 0;
        string report_file_path = "";
        
        if(parameters.find("y_cruncher_status") != string::npos) {
            // cout << "run_y_cruncher_test ======== " << run_y_cruncher_test << endl;
            
            if (!run_y_cruncher_test) {
                cout << "\nY-Cruncher Test executing..." << endl;
                
                // Get current executable directory
                char buffer[MAX_PATH];
                GetModuleFileNameA(NULL, buffer, MAX_PATH);
                string exe_path(buffer);
                string exe_dir = exe_path.substr(0, exe_path.find_last_of("\\/"));
                
                // Construct paths
                string y_cruncher_folder = exe_dir + "\\y-cruncher v0.8.7.9547b";
                string y_cruncher_exe = y_cruncher_folder + "\\y-cruncher.exe";
                try {
                    for (const auto& entry : fs::directory_iterator(y_cruncher_folder)) {
                        if (entry.is_regular_file()) {
                            string filename = entry.path().filename().string();

                            if (
                                filename.find("Pi") != string::npos &&
                                filename.find(".txt") != string::npos
                            ) {
                                fs::remove(entry.path());
                                cout << "Old y-cruncher report deleted before fresh run: "
                                    << entry.path().string() << endl;
                            }
                        }
                    }
                } catch (const exception& e) {
                    cout << "Unable to delete old y-cruncher reports: " << e.what() << endl;
                }
                cout << "Y-Cruncher path: " << y_cruncher_exe << endl;
                
                // Check if y-cruncher exists
                if(!file_exist(y_cruncher_exe)){
                    cout << "Error: y-cruncher.exe not found at: " << y_cruncher_exe << endl;
                    parametrmap["y_cruncher_status"] = "FAIL";
                    parametrmap["y_cruncher_error"] = "y-cruncher.exe not found";
                    parametrmap["y_cruncher_File_Updated_name"] = "Executable Not Found";
                    return_value = partresultservice.updateBulkPartResult(parametrmap, fieldmap);
                    parametrmap.clear();
                    iqcservice.ChecksResultSync("Y-Cruncher Check");
                    cout << "\nY-Cruncher test is Completed..." << endl;
                    return return_value;
                }
                
                // Execute y-cruncher with automatic settings (no user interaction)
                // skip-warnings: Skip all warnings
                // bench: Run benchmark mode  
                // 500m: Test with 500 million digits
                string y_cruncher_command = "cd /d \"" + y_cruncher_folder + "\" && y-cruncher.exe skip-warnings bench 500m";
                cout << "Executing: " << y_cruncher_command << endl;
                system(y_cruncher_command.c_str());
                
                // Wait for completion (500m takes about 1-2 minutes on average systems)
                cout << "Waiting for y-cruncher to complete (approx. 1 minutes)..." << endl;
                Sleep(60000);  // Wait 2 minutes
                
                // Find the generated report file (both validation and benchmark files)
                try {
                    // First, try to find validation file (Pi - YYYYMMDD-HHMMSS.txt)
                    for (const auto& entry : fs::directory_iterator(y_cruncher_folder)) {
                        if (entry.is_regular_file()) {
                            string filename = entry.path().filename().string();
                            // Look for validation file pattern: "Pi - 20260212-173238.txt"
                            if (filename.find("Pi - ") != string::npos && filename.find(".txt") != string::npos) {
                                report_file_path = entry.path().string();
                                cout << "Found validation report: " << report_file_path << endl;
                                break;
                            }
                        }
                    }
                    
                    // If validation file not found, try benchmark file pattern
                    if (report_file_path.empty()) {
                        for (const auto& entry : fs::directory_iterator(y_cruncher_folder)) {
                            if (entry.is_regular_file()) {
                                string filename = entry.path().filename().string();
                                // Look for any Pi-related text file
                                if (filename.find("Pi") != string::npos && filename.find(".txt") != string::npos) {
                                    report_file_path = entry.path().string();
                                    cout << "Found benchmark report: " << report_file_path << endl;
                                    break;
                                }
                            }
                        }
                    }
                } catch (const exception& e) {
                    cout << "Error finding y-cruncher report: " << e.what() << endl;
                }
            }
            
            if (run_y_cruncher_test) {
                parametrmap["y_cruncher_status"] = "PASS";
                parametrmap["QI_Reference"] = y_cruncher_qi_name;
            } else if (!report_file_path.empty()) {
                // Upload file to ERP
                GetFromERPService getfromERP;
                string File_updated_name = getfromERP.updateFile2ERp(report_file_path.c_str());
                
                if (File_updated_name != "") {
                    parametrmap["y_cruncher_File_Updated_name"] = File_updated_name;
                    parametrmap["y_cruncher_status"] = "PASS";
                } else {
                    parametrmap["y_cruncher_File_Updated_name"] = "Not able to update the file";
                    parametrmap["y_cruncher_status"] = "FAIL";
                }
            } else {
                parametrmap["y_cruncher_status"] = "FAIL";
                parametrmap["y_cruncher_File_Updated_name"] = "File Not Found";
            }
            
            return_value = partresultservice.updateBulkPartResult(parametrmap, fieldmap);
            parametrmap.clear();
            iqcservice.ChecksResultSync("Y-Cruncher Check");
            cout << "\nY-Cruncher test is Completed..." << endl;
        }
        return return_value;
    }

    private : void updateScore(string param,string healthscore,int conditionValue,map<string,string>fieldMap){
    
    fieldMap["param"]=param;
    fieldMap["result"]=healthscore;
    
    try{
      
      if(stoi(healthscore)>=conditionValue){
       fieldMap["score"]="0";
    }else if(stoi(healthscore)>=dignostic_passing_health){
      fieldMap["score"]="1";
    }else{
        fieldMap["score"]="41"; 
    }
  }catch(const std::exception& e) {
      cout<<"\n Error While calculating Score for Storage: "<<e.what()<<healthscore<<endl;
    }
    partresultservice.updatePartResult(fieldMap);
  } 


  public: map<int , map<string, string>> data() {
    std::map<std::string, std::string> resultData;
    map<int , map<string, string>> stockData;
    int count=0;
    std::string filePath = "Hard Disk Sentinel/HDSentinel_6.40 PRO_report.txt";
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return stockData;
    }

    std::string line;
    bool inSummarySection = false;
    std::vector<std::string> diskSummary;
    while (std::getline(file, line)) {
        if (line.find("Hard Disk Summary") != std::string::npos) {
            inSummarySection = true;
            continue;  
        }
        
        if (line.find("The status of the solid state") != std::string::npos) {
            inSummarySection = false;
            diskSummary.push_back(line);           
            // break;  
        }else if(line.find("bad sectors") != std::string::npos){
            inSummarySection = false;
            diskSummary.push_back(line);           
            // break;  
        }else if(line.find("Problems occurred")!=std::string::npos){
            inSummarySection=false;
            diskSummary.push_back(line);
            // break;
        }else if(line.find("hard disk status")!=std::string::npos){
            inSummarySection=false;
            diskSummary.push_back(line);
            // break;
        }else if(line.find("S.M.A.R.T. attribute(s)")!=std::string::npos){
            inSummarySection=false;
            diskSummary.push_back(line);
            
        }

        if (inSummarySection) {
            diskSummary.push_back(line);
            // cout<<line;
        }
    }

    file.close();
    
    if (!diskSummary.empty()) {
       
        for (const auto& line : diskSummary) { 
            if (line.find("The status of the solid state") != std::string::npos || line.find("bad sectors") != std::string::npos || line.find("Problems occurred")!=std::string::npos || line.find("hard disk status")!=std::string::npos || line.find("S.M.A.R.T. attribute(s)")!=std::string::npos){ 
             string status=line;
             if(status.length()>140){
                status=status.substr(0,140);
             }
             resultData["status"] = status;
             stockData[count]=resultData;
             count++;
             resultData.clear();
             
            }

            size_t delimiterPos = line.find(':');
            if (delimiterPos != std::string::npos && delimiterPos != 0 && delimiterPos != line.size() - 1) {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);
                key.erase(std::remove(key.begin(), key.end(), '.'), key.end());
                key = util.trim(key);
                value =util.trim(value);
                if (!key.empty() && !value.empty()) {
                    resultData[key] = value;
                }
            }
        }
         
    } else {
        std::cout << "\nNo Fixed Disk found in the report." << std::endl;
    }

    return stockData;
}

private:string extractInteger(std::string str) {
    std::string numberstr;
    for(char ch :str){
      if(isdigit(ch)){
        numberstr+=ch;
      }
    }
    return numberstr;
}

public: void deletefile(string filepath){
        fs::path filePath(filepath);

    try {
       
        if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
            
            fs::remove(filePath);
            std::cout << "File deleted successfully.\n";
        } else {
            std::cout << "The specified .txt file does not exist.\n";
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

}

    public:    bool runHDDTest(string result){
        std::vector<std::string>HDDCheckInfo;
        HDDCheckInfo.push_back("HDD_Sentinel_Status");
        HDDCheckInfo.push_back("heavy_load_status");
        HDDCheckInfo.push_back("y_cruncher_status");
        // driverCheckInfo.push_back("");

        for(const std::string& bluetoothcheck:HDDCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }

    public:void update_HDD_passingHealth(string department,json result){
    // json result=njsettings.getNjSettingsvalues();
    for(const auto&item: result){
   
      if(item["department"]==department ){
        if(item["parameter"]=="passing_Health")
        passing_health=stoi(item["value"].get<string>());
    
      }    else if(item["parameter"]=="dignostic_passing_Health"){
        dignostic_passing_health=stoi(item["value"].get<string>());
      }
        else if(item["parameter"]=="heavy_load_temperature"){
         heavy_load_temp=stoi(item["value"].get<string>());
      }
      
    }
    
  }
  void checkTemperatures(const json& systemInfo) {
    try{
    for (auto [key, value] : systemInfo.items()) {
        if (key.find("Temperature") != string::npos) {  // Check temperature keys
            // Extract numeric temperature value
            size_t pos = value.get<string>().find('C');
            if (pos != string::npos) {
                double temp = stod(value.get<string>().substr(0, pos));  // Convert to double
                if (temp > 90.0) {
                    parametrmap["Burning_Temperature"] = value.get<string>();
                    parametrmap["Burning_status"] = "FAIL";
                    break;
                }
            }
        }
    }
}catch(exception& e){
    cout<<"Error in checkTemperatures "<<e.what()<<endl;
}
}
void checkTestResults(const json& testResults) {
    // cout << "Failed Tests:\n";
    for (auto& [key, test] : testResults.items()) {
        if (test.contains("error") && test["error"] != "0") {  // Check if error is not 0
            parametrmap["Burning_status"] = "FAIL";
            parametrmap["Burning_error"] = test["error"];
            break;
        }
    }
}
  public:bool file_exist(string filepath){
    fs::path filePath(filepath);
    if(fs::exists(filePath) && fs::is_regular_file(filePath)){
        return true;
    }
    return false;
}

  public:string extractInteger_only(std::string str) {
    std::string numberstr;
    for(char ch :str){
      if(isdigit(ch)){
        numberstr+=ch;
      }else if(ch=='.'){
        break;
      }
    }
    return numberstr;
  }
    std::string sanitize_utf8(const std::string& input) {
    std::string output;
    for (unsigned char c : input) {
        if ((c >= 0x20 && c <= 0x7E) || c == '\n' || c == '\r' || c == '\t') {
            output += c;  
        }     
    }
    return output;
    }
    
    std::string take_screen_shot(){
        std::string screenshot_cmd ="powershell -command \"& {Add-Type -AssemblyName System.Windows.Forms; $bmp = New-Object System.Drawing.Bitmap([System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Width, [System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Height); $graphics = [System.Drawing.Graphics]::FromImage($bmp); $graphics.CopyFromScreen(0, 0, 0, 0, $bmp.Size); $bmp.Save('HddSentinel_screen_shot.png', [System.Drawing.Imaging.ImageFormat]::Png); $graphics.Dispose(); $bmp.Dispose()}\"";
        util.executeTerminal(screenshot_cmd);
        std::string file_path="HddSentinel_screen_shot.png";
        if (std::filesystem::exists(file_path)) {
            std::string File_updated_name= getfromERP.updateFile2ERp("HddSentinel_screen_shot.png");      
            if(File_updated_name!=""){
                return File_updated_name;
            }
        }
        return "";
    }
};
int HDDSentinelCheck::passing_health = 80;
int HDDSentinelCheck::heavy_load_temp = 95;
int HDDSentinelCheck::dignostic_passing_health=50;
bool HDDSentinelCheck::run_test=false;
bool HDDSentinelCheck::is_component=false;
string HDDSentinelCheck::qi_name;
string HDDSentinelCheck::heavy_load_result;
bool HDDSentinelCheck::run_y_cruncher_test = false;
string HDDSentinelCheck::y_cruncher_qi_name;
