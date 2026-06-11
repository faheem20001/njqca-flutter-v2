#include <iostream>
#include <string>
#include <map>
#include <regex>
using namespace std;

class PerformanceCheck : public V2Service {
    IQCService iqcservice;
    Util util; 
    PartResultService partresultservice; 
    CompleteBatteryCheck completebatterycheck;
    NjsettingsService njsettings;
    HDDSentinelCheck hdd;
    GetFromERPService getfromERP;
    Constant constant;
    map<string, string> parametrmap;
    static::string fileURL;
    bool url_data_found=false;
    std::thread sensorThread;
    std::thread screen_recorder_Thread;


public:
    static bool run_test;
    static string qi_name;
    void collect_winsat_memory_metrics() {
        try {
            cout << "\nWinSAT Memory Test checking..." << endl;

            string command =
                "powershell -NoProfile -ExecutionPolicy Bypass -Command "
                "\"$Output = winsat mem 2>&1; "
                "New-Item -ItemType Directory -Force -Path 'C:\\WindowsNJQCA\\Temp_Data' | Out-Null; "
                "$Lines = $Output | ForEach-Object { $_.ToString() }; "
                "$Lines | Out-File -Encoding UTF8 'C:\\WindowsNJQCA\\Temp_Data\\Memory_WinSAT_Result.txt'; "
                "$Bandwidth = ($Lines | Where-Object { $_ -match '^\\s*>\\s*Memory\\s+Performance\\s+[\\d.]+' } | Select-Object -First 1); "
                "Write-Output $Bandwidth\"";

            string winsat_output = util.executeTerminal(command);

            cout << "\nWinSAT Memory Output: " << winsat_output << endl;

            std::regex memRegex(R"(Memory\s+Performance\s+([\d.]+)\s+MB/s)");
            std::smatch match;

            if (std::regex_search(winsat_output, match, memRegex)) {
                string bandwidth = match[1].str();

                parametrmap["Memory Throughput Read Speed [MB/s]"] = bandwidth + " MB/s";
                parametrmap["Memory Throughput Write Speed [MB/s]"] = bandwidth + " MB/s";
                parametrmap["Memory Latency [ns]"] = "Not Available";

                cout << "Memory Throughput Read Speed [MB/s]: " << bandwidth << " MB/s" << endl;
                cout << "Memory Throughput Write Speed [MB/s]: " << bandwidth << " MB/s" << endl;
                cout << "Memory Latency [ns]: Not Available" << endl;
            } else {
                parametrmap["Memory Throughput Read Speed [MB/s]"] = "Not Found";
                parametrmap["Memory Throughput Write Speed [MB/s]"] = "Not Found";
                parametrmap["Memory Latency [ns]"] = "Not Available";

                cout << "WinSAT Memory Performance value not found" << endl;
            }

            string winsat_file_path = "C:\\WindowsNJQCA\\Temp_Data\\Memory_WinSAT_Result.txt";

            if (fs::exists(winsat_file_path)) {
                string uploaded_file = getfromERP.updateFile2ERp(winsat_file_path.c_str());

                if (uploaded_file != "") {
                    parametrmap["Memory_WinSAT_File_Updated_name"] = uploaded_file;
                } else {
                    parametrmap["Memory_WinSAT_File_Updated_name"] = "Not able to update the file";
                }
            } else {
                parametrmap["Memory_WinSAT_File_Updated_name"] = "File Not Found";
            }

        } catch (const exception& e) {
            cout << "Error while running WinSAT memory test: " << e.what() << endl;

            parametrmap["Memory Throughput Read Speed [MB/s]"] = "Error";
            parametrmap["Memory Throughput Write Speed [MB/s]"] = "Error";
            parametrmap["Memory Latency [ns]"] = "Not Available";
        }
    }
    int PerformanceTest(string stockinfoID, string partId, string item_id) {
        cout << "\nPerformance Test checking..." << endl;
        bool excel_found = true;
        bool word_found = true;

        std::string core_command = "powershell.exe -Command \"Get-WmiObject -Class Win32_Processor | Format-List name,NumberOfCores,NumberOfLogicalProcessors\"";
        std::string model="";
        std::string processor = "";
        std::string ram = "";
        std::string storage = "";

      std::map<std::string, std::string> requestMap;
      json manufacturer_model;
      manufacturer_model["bios_serial_no"]=iqcservice.bios_serial_no; 
      string model_list_response=iqcservice.getParameterList(constant.API_get_model_name, "PUT", requestMap, to_string(manufacturer_model));
      if(model_list_response!=""){
        try {
        json model_list = json::parse(model_list_response);

        // Frappe returns {"message": { ...your dict... }}
        if (model_list.contains("message")) {
            json message = model_list["message"];

            if (message.is_object()) {
                if (message.contains("error")) {
                    cout << "ERP Error: " << message["error"] << endl;
                } else {
                    model     = message.value("model", "");
                    processor = message.value("Processor Type", "");
                    ram       = message.value("RAM Capacity", "");
                    storage   = message.value("Storage Capacity", "");
                }
            } else if (message.is_string()) {
                cout << "Unexpected string response: " << message.get<std::string>() << endl;
            }
        }
    }catch(const exception& e){
          cout<<"Error while getting the model"<<e.what()<<model_list_response<<endl;
        }
      }
      cout<<"\nModel Name:"<<model;
      cout << "\nProcessor: " << processor;
      cout << "\nRAM: " << ram;
      cout << "\nStorage: " << storage << endl;
        // completebatterycheck.download_URL_file(fileURL, "performance.txt");

        int return_value = 0;
        string base_command =
            "cmd /c performance.exe "
            "-m \"" + model + "\" "
            "-p \"" + processor + "\" "
            "-r \"" + ram + "\" "
            "-s \"" + storage + "\" ";
        // string performnce_command="performance.exe";
        // string performnce_command="performance.exe -m \""+model+"\"";
        string performnce_command = base_command;
        cout<<"\nPerformnce command is :"<<performnce_command<<endl;
        string screen_recorder_command="screen_record.exe";
        string kill_screen_recorder="taskkill /im screen_record.exe /f";
        // string excel_rerun_command="\"performance.exe\" -m \""+model+"\" -excel";
        string excel_rerun_command =  base_command + "-excel";
        // string file_transfer_command="\"performance.exe\" -ft";
        string file_transfer_command="cmd /c performance.exe -ft";
        // string urls_command="\"performance.exe\" -m \""+model+"\" -urls";
        string urls_command = base_command + "-urls";
        string average_restart_command="restart.exe";
        string restart_command="runperformance.exe";
        string sensor_info="performance_monitor.exe";
        
        string KillProcess_sensors="cmd /c \"taskkill /im performance_monitor.exe /f\"";
        string KillProcess_open_monitor="cmd /c \"taskkill /im OpenHardwareMonitor.exe /f\"";
        std::string script = 
            "powershell -NoProfile -Command \""
            "function Get-AppPath($appExe) { "
            "    $path = $null; "
            "    $regPaths = @( "
            "        'HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\' + $appExe, "
            "        'HKLM:\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\App Paths\\' + $appExe "
            "    ); "
            "    foreach ($reg in $regPaths) { "
            "        $val = (Get-ItemProperty -Path $reg -ErrorAction SilentlyContinue).'Default'; "
            "        if ($val) { $path = $val; break } "
            "    }; "
            "    return $path "
            "} "
            "$excelPath = Get-AppPath 'excel.exe'; "
            "$wordPath = Get-AppPath 'winword.exe'; "
            "if ($excelPath -and $wordPath) { "
            "    Write-Output 'Excel_found: ' + $excelPath; "
            "    Write-Output 'Word_found: ' + $wordPath; "
            "    exit 0 "
            "} elseif ($excelPath) { "
            "    Write-Output 'Excel_found: ' + $excelPath; "
            "    exit 0 "
            "} elseif ($wordPath) { "
            "    Write-Output 'Word_found: ' + $wordPath; "
            "    exit 0 "
            "} "
            "$officeKeys = Get-ChildItem 'HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall' | "
            "Where-Object { $_.GetValue('DisplayName') -like '*Microsoft Office*' }; "
            "if ($officeKeys) { "
            "    foreach ($key in $officeKeys) { "
            "        $displayName = $key.GetValue('DisplayName'); "
            "        $version = $key.GetValue('DisplayVersion'); "
            "        Write-Output 'Word_found: ' + $displayName + ' - Version: ' + $version; "
            "        Write-Output 'Excel_found: ' + $displayName + ' - Version: ' + $version; "
            "    } "
            "} else { "
            "    Write-Output 'No MSI-based Office found.' "
            "} "
            "exit 0"
            "\"";

        string result=util.executeTerminal(script);

        if (result.find("Excel_found") == std::string::npos) {
            excel_found = false;
        }
        if (result.find("Word_found") == std::string::npos) {
            word_found = false;
        }
        if (result.length()>140){
            result=result.substr(0,140);
        }
        parametrmap["excel_installation_status"] = result;



        string offic_setup_path="C:\\Office_Setup\\office\\Setup.exe";
        fs::path offic_setup(offic_setup_path);

        fs::path delete_office("C:\\Office_Setup");

        const string install = "powershell -Command \"Start-Process -FilePath 'C:\\Office_Setup\\office\\Setup.exe' -ArgumentList '/SILENT' -WorkingDirectory 'C:\\Office_Setup\\office' -Wait\""; 
        if(!excel_found || !word_found){
            const string installer_command="office_installer.exe";
            cout<<"Installing Office..."<<endl;
            string installation_status=util.executeTerminal(installer_command);
            cout<<installation_status<<endl;
            if(installation_status.length()>140){
                installation_status=installation_status.substr(0,140);
            }
            parametrmap["office_setup_installation_status"] = installation_status;
            Sleep(2000);
           if(fs::exists(offic_setup)){
            util.executeTerminal(install);
           }
           fs::remove_all(delete_office);
        }

        map<string, string> fieldmap;
        // cout<<"run Test value:"<<run_test<<endl;
        // cout<<"qi name:"<<qi_name<<endl;
 
        if(run_test){
            parametrmap["Performance_status"] = "PASS";
            parametrmap["QI_Reference"] = qi_name;
        }else{ 
               
               string core_result=util.executeTerminal(core_command);
               std::istringstream iss(core_result);
               json core_data = util.ExcetuteTerminalJSonArray1(iss);
               if(core_data.size()>0){
                   parametrmap["No_of_Core"] = core_data[0]["NumberOfCores"];
                   parametrmap["No_of_thread"] = core_data[0]["NumberOfLogicalProcessors"];
                }
                // New: Capture WinSAT memory bandwidth
                collect_winsat_memory_metrics();
               sensorThread = std::thread([&]() {util.executeTerminal(sensor_info);});
               screen_recorder_Thread = std::thread([&]() {util.executeTerminal(screen_recorder_command);});
               util.executeTerminal(performnce_command);
               cout<<"Performance Test completed..."<<endl;
               system(KillProcess_open_monitor.c_str());
               system(KillProcess_sensors.c_str());
            //    system(kill_screen_recorder.c_str());
               Sleep(2000);
               if(sensorThread.joinable()){
                   sensorThread.join();
                }
            //    if(screen_recorder_Thread.joinable()){
            //        screen_recorder_Thread.join();
            //     }
               parse_csv_file();
               parseFile("Temp_Data/restart_times.txt");
               for (int i = 0; i < 3; i++) {
                //    if (parametrmap["Average_restart_time"]!="" && parametrmap["Excel_time"]!="" && parametrmap["storage_transfer_time"]!="" && url_data_found){ 
                //        break;
                //     }
                    if (parametrmap["Average_restart_time"]!=""&& parametrmap["storage_transfer_time"]!="" && url_data_found){ 
                       break;
                    }
                    if(parametrmap.find("Restart_1")==parametrmap.end()){
                        util.executeTerminal(restart_command);
                    }

                    if(parametrmap["Average_restart_time"].empty()){
                        util.executeTerminal(average_restart_command);
                    }

                    // if(parametrmap["Excel_time"].empty()){
                    //     util.executeTerminal(excel_rerun_command);
                    // }

                    if(parametrmap["storage_transfer_time"]==""){
                        util.executeTerminal(file_transfer_command);
                    }
                    if(!url_data_found){
                        util.executeTerminal(urls_command);
                    }
                    parseFile("Temp_Data/restart_times.txt");
                }
                system(kill_screen_recorder.c_str());
               if(screen_recorder_Thread.joinable()){
                   screen_recorder_Thread.join();
                }
               std::string filePath = "Temp_Data/restart_times.txt";
               std::string screen_record_file_path="Compact_Recording.mkv";             
                if (std::filesystem::exists(filePath)) {
                std::string File_updated_name= getfromERP.updateFile2ERp("Temp_Data/restart_times.txt");      
        
                if(File_updated_name!=""){
                    parametrmap["performance_File_Updated_name"]=File_updated_name;
                }else{
                    parametrmap["performance_File_Updated_name"]="Not able to update the file";
                }
                }
                else{
                    parametrmap["performance_File_Updated_name"]="File Not Found";
                }
                if (parametrmap.find("gaming_fps_log_path") != parametrmap.end()) {
                    std::string fps_path = parametrmap["gaming_fps_log_path"];
                    if (std::filesystem::exists(fps_path)) {
                        std::string uploaded = getfromERP.updateFile2ERp(fps_path.c_str());  // .c_str() added
                        parametrmap["gaming_fps_file_uploaded"] = uploaded != "" ? uploaded : "Upload failed";
                    } else {
                        parametrmap["gaming_fps_file_uploaded"] = "File Not Found";
                    }
                }

                if (parametrmap.find("gaming_system_log_path") != parametrmap.end()) {
                    std::string sys_path = parametrmap["gaming_system_log_path"];
                    if (std::filesystem::exists(sys_path)) {
                        std::string uploaded = getfromERP.updateFile2ERp(sys_path.c_str());  // .c_str() added
                        parametrmap["gaming_system_file_uploaded"] = uploaded != "" ? uploaded : "Upload failed";
                    } else {
                        parametrmap["gaming_system_file_uploaded"] = "File Not Found";
                    }
                }
                if (std::filesystem::exists(screen_record_file_path)) {
                convert_file();
                std::string screen_record_File_updated_name= getfromERP.updateFile2ERp("Compact_Recording.mp4");

                // cout<<"\nPerformance Check Result is: "<<screen_record_File_updated_name<<endl;
                if(screen_record_File_updated_name!=""){
                    hdd.deletefile("Compact_Recording.mp4");
                    hdd.deletefile("Compact_Recording.mkv");
                    parametrmap["Screen_record_File_Updated_name"]=screen_record_File_updated_name;
                }else{
                    parametrmap["Screen_record_File_Updated_name"]="Not able to update the file";
                }
                }
                else{
                    parametrmap["Screen_record_File_Updated_name"]="File Not Found";
                }
        }
        
        
        fieldmap["stock_info_id"] = stockinfoID;
        fieldmap["part_id"] = partId;
        // fieldmap["part_config_id"] = partconfigid;
        fieldmap["item_id"] = item_id;
        // fieldmap["created_by"] = created_by;

        return_value = partresultservice.updateBulkPartResult(parametrmap, fieldmap);

        hdd.deletefile("Temp_Data/restart_times.txt");
        iqcservice.ChecksResultSync("Performance Check");
       
        cout << "\nPerformance check is Completed..." << endl;
        return return_value;
    }


struct FileTransfer {
    std::string size;
    std::string time;
    std::string speed;
    std::string compression_time;
};

void parseFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filePath << std::endl;
        parametrmap["Performance_status"] = "File Not Found";
        return;
    }

    std::string line;
    std::string averageRestartInterval;
    std::vector<std::string> restartTimes;
    std::string excelTime;
    FileTransfer fileTransfer;
    std::string tabSwitchingTime;

    std::regex restartRegex(R"(restart \d+:(\d+):(\d+):(\d+))");
    std::regex avgRestartRegex(R"(Average Restart Interval: (\d+):(\d+):(\d+))");
    std::regex excelRegex(R"(excel,Total time for excel,([\d.]+))");
    std::regex fileSizeRegex(R"(file_transfer,Total file size,([\d.]+ GB))");
    std::regex fileTimeRegex(R"(file_transfer,Time taken for file transfer,([\d.]+ seconds))");
    std::regex fileSpeedRegex(R"(file_transfer,File transfer speed,([\d.]+ MB/s))");
    std::regex tabSwitchingRegex(R"(tab_switching,Total time taken for switching tabs,([\d.]+))");
    std::regex gamingFpsRegex(R"(gaming_fps_log,(.+))");
    std::regex gamingSystemRegex(R"(gaming_system_log,(.+))");
    std::regex compressionTimeRegex(
        R"(compression,Task Execution Time \(([\d]+ MB) Compression\) \[s\],([\d.]+ seconds))"
    );

    while (std::getline(file, line)) {
        try{
        std::smatch match;

        if (std::regex_search(line, match, avgRestartRegex)) {
            averageRestartInterval = match[1].str() + "h " + match[2].str() + "m " + match[3].str() + "s";
        } else if (std::regex_search(line, match, restartRegex)) {
            restartTimes.push_back(match[1].str() + "h " + match[2].str() + "m " + match[3].str() + "s");
        } else if (std::regex_search(line, match, excelRegex)) {
            excelTime = match[1].str() + " seconds";
        } else if (std::regex_search(line, match, fileSizeRegex)) {
            fileTransfer.size = match[1].str();
        } else if (std::regex_search(line, match, fileTimeRegex)) {
            fileTransfer.time = match[1].str();
        } else if (std::regex_search(line, match, fileSpeedRegex)) {
            fileTransfer.speed = match[1].str();
        }else if (std::regex_search(line, match, compressionTimeRegex)) {
            fileTransfer.compression_time = match[2].str();
        }else if (std::regex_search(line, match, tabSwitchingRegex)) {
            tabSwitchingTime = match[1].str() + " seconds";
        }else if(line.find("url,")!=std::string::npos){
            map<int, string> urlmap=util.splitString(line, ",");
            try{
                if(urlmap.size()>0){
                    url_data_found=true;
                    string url=urlmap[1];
                    if(url.length()>140){
                        url=url.substr(0,139);}
                    parametrmap[url]=urlmap[2];
                }
            }catch(exception & e){
                cout<<"ERROR parseing the restart file for URL"<<e.what()<<endl;
            }
        }
    }catch(exception & e){
        cout<<"ERROR parseing the restart file"<<e.what()<<endl;
    }

    }

    file.close();

    // Print extracted data
    // std::cout << "Average Restart Interval: " << averageRestartInterval << std::endl;
    parametrmap["Average_restart_time"]=averageRestartInterval;
    // std::cout << "Restart Times:" << std::endl;
    for (size_t i = 0; i < restartTimes.size(); ++i) {
        // std::cout << "  Restart " << (i + 1) << ": " << restartTimes[i] << std::endl;
        parametrmap["Restart_"+to_string(i+1)]=restartTimes[i];
    }
    // std::cout << "Excel Processing Time: " << excelTime << std::endl;
    parametrmap["Excel_time"]=excelTime;

    // std::cout << "  Size: " << fileTransfer.size << std::endl;
    parametrmap["storage_transfer_File_size"]=fileTransfer.size;

    // std::cout << "  Time Taken: " << fileTransfer.time << std::endl;
    parametrmap["storage_transfer_time"]=fileTransfer.time;

    // std::cout << "  Speed: " << fileTransfer.speed << std::endl;
    parametrmap["storage_transfer_speed"]=fileTransfer.speed;
    parametrmap["file_compression_task_execution_time"] = fileTransfer.compression_time;
    // // std::cout << "Tab Switching Time: " << tabSwitchingTime << std::endl;
    parametrmap["tab_switching_time"]=tabSwitchingTime;
}

void parse_csv_file(){
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile != nullptr) {
        std::string path(userProfile);
        std::string fullPath = path + "\\Desktop\\HardwareMonitor_Report.csv";
       std::ifstream file(fullPath);
        if (std::filesystem::exists(fullPath)) {
        std::string File_updated_name= getfromERP.updateFile2ERp(fullPath.c_str());      

        if(File_updated_name!=""){
            parametrmap["sensors_File_Updated_name"]=File_updated_name;
        }else{
            parametrmap["sensors_File_Updated_name"]="Not able to update the file";
        }
        }
        else{
            parametrmap["sensors_File_Updated_name"]="File Not Found";
            return;
        }


    if (!file.is_open()) {
        std::cerr << "Failed to open file\n";
        return;
    }

    std::string line;
    std::vector<std::string> headers;
    std::unordered_map<std::string, int> columnIndices;

    // Specify the columns we care about
    std::vector<std::string> targetColumns = {
        "Disk Read KB/s",
        "Disk Write KB/s",
        " | Load | Memory",
        " | Temperature | CPU Package",
        " | Load | CPU Total",
        "Charge Rate"
    };

    std::unordered_map<std::string, double> sums;
    std::unordered_map<std::string, int> counts;
    std::unordered_map<std::string, bool> columnExists;

    // Parse header line
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        int index = 0;
        while (std::getline(ss, token, ',')) {
            headers.push_back(token);
            for (const auto& col : targetColumns) {
                if (token == col) {
                    columnIndices[col] = index;
                    columnExists[col] = true;
                    break;
                }
            }
            index++;
        }
    }

    // Initialize columnExists for columns not found
    for (const auto& col : targetColumns) {
        if (columnExists.find(col) == columnExists.end()) {
            columnExists[col] = false;
        }
    }

    // Process data rows
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        std::vector<std::string> values;

        while (std::getline(ss, value, ',')) {
            values.push_back(value);
        }

        for (const string& col : targetColumns) {
            if (!columnExists[col]) continue;
            int idx = columnIndices[col];
            if (idx < values.size()) {
                try {
                    double val = std::stod(values[idx]);
                    sums[col] += val;
                    counts[col]++;
                } catch (...) {
                    // Skip invalid entries
                }
            }
        }
    }

    // Print average values
    std::cout << std::fixed << std::setprecision(2);
    for (const auto& col : targetColumns) {
        std::string cleanCol = col;
        cleanCol.erase(std::remove(cleanCol.begin(), cleanCol.end(), '|'), cleanCol.end());

        std::cout << "Average " << cleanCol << ": ";
        if (!columnExists[col] || counts[col] == 0) {
            parametrmap["Average "+cleanCol]="";
            std::cout << "\"\"\n";
        } else {
            string result = to_string(sums[col] / counts[col]);
            parametrmap["Average "+cleanCol]= sanitize_utf8(result);
            std::cout << sums[col] / counts[col] << "\n";
        }
    }
    file.close();
 }
}


public:    bool runperformanceTest(string result){
    std::vector<std::string>performanceCheckInfo;
    performanceCheckInfo.push_back("Performance_status");
    // performanceCheckInfo.push_back("TabSwitchTime");
    // LIDCheckInfo.push_back("HDMI_Status");

    // driverCheckInfo.push_back("Error_Drivers_Count");

    for(const std::string& bluetoothcheck:performanceCheckInfo){
        if(result.find(bluetoothcheck)!=std::string::npos){
            return true;
            break;
        }
    }
    return false;

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

    public: string convert_file(){
        string conversion_command="ffmpeg\\bin\\ffmpeg.exe -i \"Compact_Recording.mkv\" -vcodec libx264 -acodec aac -movflags +faststart -y \"Compact_Recording.mp4\"";
        util.executeTerminal(conversion_command);
        return "";
      
    }

    
};
string PerformanceCheck::fileURL;
string PerformanceCheck::qi_name;
bool PerformanceCheck::run_test=false;