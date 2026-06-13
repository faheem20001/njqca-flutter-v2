#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <chrono>
#include <ctime>
#include <locale>
#include <codecvt>
#ifdef max
#undef max
#endif


using namespace std;
std::string narrow(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}

class SystemCheck : public V2Service {
    IQCService iqcservice;
    Util util; 
    PartResultService partresultservice; 
    NjsettingsService njsetting;
    LanCheck lan;
    GetFromERPService getfromERP;
    CompleteBatteryCheck completeBatteryCheck;
    HDDSentinelCheck hdd;
    map<string, string> parametrmap;
    
    static string NjwallpaperFilename ;
    static string nj_care_version;
    const string CMD_BATTERY = "powershell -Command \"Get-WmiObject -Class Win32_Battery | Format-List BatteryStatus,EstimatedChargeRemaining\"";
    public:
    static string windows_version; 
    int systemsoftwareTest(string stockinfoID, string partId, string item_id, string created_by,string parameters) {
        cout << "\nSystem configuration checking..." << endl;
        //  std::cout << "\033[32mThis text is green!\033[0m" << std::endl;
        int return_value = 0;
        
        map<string, string> fieldmap;
        std::string OfficeActivationcommand = "cscript \"C:\\Program Files\\Microsoft Office\\Office16\\OSPP.VBS\" /dstatus | findstr /C:\"LICENSE STATUS\" /C:\"REMAINING GRACE\" ";
        // Execute the command and capture the output
        if(parameters.find("Office_Activation_Status")!=std::string::npos){
        string Officestatus = util.executeTerminal(OfficeActivationcommand); // Captures HDMI and VGA status
        // cout << "Status Output: " << Officestatus << endl;
        if(Officestatus.find("OOB_GRACE") != std::string::npos || Officestatus.find("LICENSED") != std::string::npos) {
           parametrmap["Office_Activation_Status"] = "PASS";
        }else{
            parametrmap["Office_Activation_Status"] = "FAIL";
        }
        Officestatus=util.trim(Officestatus);
        if(Officestatus.length()>140){
            Officestatus=Officestatus.substr(0,140);
        }
        parametrmap["office_activation_days"]=Officestatus;    }    // // parametrmap["Office_Activation_Status"] = "FAIL";
        // if(parametrmap["Office_Activation_Status"]!="PASS"){
        //     string activate="ohookOffice_Activation.cmd";
        //     string activatecommand="cmd /c start \"\" \""+activate+"\"";
        //     util.executeTerminal(activatecommand);
        // }
        // Officestatus = util.executeTerminal(OfficeActivationcommand); // Captures HDMI and VGA status
        // // cout << "Status Output: " << Officestatus << endl;
        // if(Officestatus.find("1") != std::string::npos || Officestatus.find("2") != std::string::npos) {
        //    parametrmap["Office_Activation_Status"] = "PASS";
        // }else{
        //     parametrmap["Office_Activation_Status"] = "FAIL";
        // }

        // std::string WindowsActivationcommand = "wmic path SoftwareLicensingProduct where \"PartialProductKey is not null and Name like '%Windows%'\" get LicenseStatus";
        if(parameters.find("Windows_Activation_Status")!=std::string::npos){
            util.executeTerminal("sc start sppsvc");
            Sleep(5000);
        std::string WindowsActivationcommand = "cscript //nologo slmgr.vbs /xpr";
        for(int i=1;i<4;i++){
            string windowsactivationstatus = util.executeTerminal(WindowsActivationcommand);
            // cout << "Status Output: " << windowsactivationstatus << endl;
            if(windowsactivationstatus.find("permanently activated") != std::string::npos) {
            parametrmap["Windows_Activation_Status"] = "PASS";
             
            }else{
                parametrmap["Windows_Activation_Status"] = "FAIL";
            }
            if(windowsactivationstatus.length()>140){
                windowsactivationstatus=windowsactivationstatus.substr(0,140);
            }
            parametrmap["windows_activation_results_"+std::to_string(i)]=windowsactivationstatus;
            if(parametrmap["Windows_Activation_Status"]=="PASS"){
                break;
            }
            Sleep(5000);
        }

        }
        // parametrmap["Windows_Activation_Status"] = "FAIL";
        // if(parametrmap["Windows_Activation_Status"]=="FAIL"){
        //     string activate="W10DigitalActivation_x64.exe";
        //     string activatecommand="cmd /c start \"\" \""+activate+"\"";
        //     util.executeTerminal(activatecommand);
        // }
        // windowsactivationstatus = util.executeTerminal(WindowsActivationcommand); // Captures HDMI and VGA status
        // // cout << "Status Output: " << windowsactivationstatus << endl;
        // if(Officestatus.find("1") != std::string::npos) {
        //    parametrmap["Windows_Activation_Status"] = "PASS";
        // }else{
        //     parametrmap["Windows_Activation_Status"] = "FAIL";
        // }


        if (parameters.find("Windows_Installation_Date_Status")!=std::string::npos){
        string windowsinstallationdtaecommand="powershell.exe -Command \"((Get-Date) - (Get-CimInstance -ClassName Win32_OperatingSystem).InstallDate).Days\"";
        string windowsinstallationdtae="powershell.exe -Command \"(Get-CimInstance -ClassName Win32_OperatingSystem).InstallDate\"";
        string os_installed_date=util.executeTerminal(windowsinstallationdtae);
        parametrmap["Windows_Installation_Date"] = os_installed_date;
        string windowsinstallationdate=util.executeTerminal(windowsinstallationdtaecommand);
        int duration=0;
        try{
            duration=std::stoi(windowsinstallationdate);
        }catch(const std::exception& e){
            cout<<"Unable to get Windwos installation date:"<<windowsinstallationdate<<e.what()<<endl;
        }
        // util.trim(windowsinstallationdate);
        // cout<<"windowsinstallationdate"<<windowsinstallationdate<<endl;
        if(duration>180){
            parametrmap["Windows_Installation_Date_Status"] = "FAIL";
        }else{
            parametrmap["Windows_Installation_Date_Status"] = "PASS";
        }
        }

        string cpudata="powershell.exe -Command \"(Get-WmiObject -Class Win32_Processor).Name\""; 
        string cpu=util.executeTerminal(cpudata);
        string Genration;
        cpu=util.trim(cpu);
        map<int, std::string>splitStringMap = util.splitString(cpu, " ");
        if(cpu.find("AMD") == std::string::npos) {
        map<int, std::string>cpuTypeStringMap = util.splitString(splitStringMap[2], "-");
        Genration=util.extractInteger(cpuTypeStringMap[1]);

        if(cpu.find("11th Gen") != std::string::npos){
            Genration = "11200";
        }
        // int endPos = 2;
        // if (cpuTypeStringMap[1].length() == 5) {endPos = 1;}
        //   Genration = cpuTypeStringMap[1].substr(0, endPos);
        
        }else{
            
        //     map<int, std::string>cpuTypeStringMap;
        // if(splitStringMap[1].find("-") != std::string::npos){
        //     cpuTypeStringMap= util.splitString(splitStringMap[1], "-");
        //     Genration = cpuTypeStringMap[1].substr(0, 1);
        // }else if(splitStringMap[2].find("-") != std::string::npos){
        //     cpuTypeStringMap= util.splitString(splitStringMap[2], "-");
        //     Genration = cpuTypeStringMap[1].substr(0, 1);
        // }else{
        //     Genration = splitStringMap[4].substr(0, 1);
        // }
        Genration="3200";
        }
        int GenrationInt=0;
        try{
            GenrationInt = std::stoi(Genration)/1000;
            cout<<"\nGenration: "<<GenrationInt<<endl;
        }catch(const std::exception& e){
            std::cerr << "Invalid input For Genration: " << e.what() << Genration<<std::endl;
        }
        
        if(parameters.find("Boot_Mode_Status")!=std::string::npos){
        string Bootmodecommand="bcdedit";
        string Bootmode=util.executeTerminal(Bootmodecommand);
        if(GenrationInt > 2 &&Bootmode.find("winload.efi") != std::string::npos || GenrationInt<=2 &&Bootmode.find("winload.efi")== std::string::npos){
            parametrmap["Boot_Mode_Status"] = "PASS";
        }else{
            parametrmap["Boot_Mode_Status"] = "FAIL";
        }
        }

        if(parameters.find("Windows_Version_Status")!=std::string::npos){
            cout<<"Windows Version"<<windows_version<<endl;
            string WindowsVersioncommand="systeminfo | find \"OS Name\""; 
            string WindowsVersion=util.executeTerminal(WindowsVersioncommand);
            if((windows_version.find("11") != std::string::npos && windows_version.find("11") != std::string::npos )||(windows_version.find("10") != std::string::npos && WindowsVersion.find("10")!=std::string::npos)){
                parametrmap["Windows_Version_Status"] = "PASS";
            }else{  
                parametrmap["Windows_Version_Status"] = "FAIL";
            }
            if (windows_version.length()>149){
                windows_version=windows_version.substr(0,149);
            }
            parametrmap["Windows_Version"] = WindowsVersion;
            parametrmap["Required_Windows_Version"] = windows_version;
        }
        
        if(parameters.find("Secure_Boot_Status")!=std::string::npos){
            string secureBoot="powershell.exe -Command \"Confirm-SecureBootUEFI\""; 
            string secureBootstatus=util.executeTerminal(secureBoot);
            if(secureBootstatus.find("False") != std::string::npos){
                parametrmap["Secure_Boot_Status"] = "PASS";
            }else{  
                parametrmap["Secure_Boot_Status"] = "FAIL";
            }
        }

        if(parameters.find("OS_version_Status")!=std::string::npos){
        string OSversionCommand="powershell.exe -Command \"(Get-ComputerInfo).OSDisplayVersion\"";
        string OSversion=util.executeTerminal(OSversionCommand);
        OSversion=util.trim(OSversion);
        parametrmap["OS_Version"] = OSversion;

        if((windows_version.find("24H2")!=string::npos && OSversion.find("24H2")==std::string::npos) ||(windows_version.find("22H2")!=string::npos && OSversion.find("22H2")==std::string::npos) ||(windows_version.find("20H2")!=string::npos && OSversion.find("20H2")==std::string::npos)){
            parametrmap["OS_version_Status"]="FAIL";
        }else{
            parametrmap["OS_version_Status"]="PASS";
        }}

        if(parameters.find("Password_Expire_Status")!=std::string::npos){
        string passwordexpirecommand="powershell.exe -Command \"(net accounts) -match 'Maximum password age' | Out-String\"";
        string passwordexpire=util.executeTerminal(passwordexpirecommand);
        string runpassword="net accounts /maxpwage:unlimited";
        if(passwordexpire.find("Unlimited") != std::string::npos){   
            parametrmap["Password_Expire_Status"] = "PASS";
        }else{  
            parametrmap["Password_Expire_Status"] = "FAIL";
        }
        if(parametrmap["Password_Expire_Status"] != "PASS"){
            util.executeTerminal(runpassword);
        }
        
        passwordexpire=util.executeTerminal(passwordexpirecommand);
        if(passwordexpire.find("Unlimited") != std::string::npos){   
            parametrmap["Password_Expire_Status"] = "PASS";
        }else{  
            parametrmap["Password_Expire_Status"] = "FAIL";
        }
        }
        // if(parameters.find("BSOD_Files_Status")!=std::string::npos){

        
        if(checkMinidumpFiles("C:\\Windows\\Minidump\\")){
            parametrmap["BSOD_Files_Status"] = "FAIL";
            json error_info=get_bsod_info("C:\\Windows\\Minidump\\");
            // cout<<error_info.dump(2)<<endl;
            for(int i=1;i<=error_info.size();i++){
                if(error_info[i-1].contains("Crash Time")){parametrmap["BSOD_Crash_Time_"+to_string(i)] = error_info[i-1]["Crash Time"];}
                if(error_info[i-1].contains("Bug Check String")){parametrmap["BSOD_ERROR_String_"+to_string(i)] = error_info[i-1]["Bug Check String"];}
                if(error_info[i-1].contains("Bug Check Code")){parametrmap["BSOD_ERROR_code_"+to_string(i)] = error_info[i-1]["Bug Check Code"];}
                if(error_info[i-1].contains("Caused By Driver")){parametrmap["BSOD_Caused_By_Driver_"+to_string(i)] = error_info[i-1]["Caused By Driver"];}
            }
        }else{
            parametrmap["BSOD_Files_Status"] = "PASS";
        }
        
        // }

        if(parameters.find("NJ_App_Installation_Status")!=std::string::npos){
        string NJAppInstallationcommand="powershell.exe -Command \"Test-Path -Path '%USERPROFILE%\\AppData\\Local\\Programs\\NJ App'\"";
        string NJAppInstallationstatus=util.executeTerminal(NJAppInstallationcommand);
        // cout<<"\nNJAppInstallationstatus: "<<NJAppInstallationstatus<<endl;
        if(NJAppInstallationstatus.find("True") != std::string::npos){
            parametrmap["NJ_App_Installation_Status"] = "PASS";
        }else{
            parametrmap["NJ_App_Installation_Status"] = "FAIL"; 
        }
    }

        if(parameters.find("NJ_Wallpaper_Status")!=std::string::npos){
        string NJWallpaper="powershell.exe -Command \"(Get-ItemProperty -Path 'HKCU:\\Control Panel\\Desktop').Wallpaper\"";
        string NJWallpaperstatus=util.executeTerminal(NJWallpaper);
        NJWallpaperstatus=util.trim(NJWallpaperstatus);
        NJWallpaperstatus=lan.getFileNameFromUrl(NJWallpaperstatus);
        // cout<<"njwallapaper"<<NJWallpaperstatus<<endl;
        parametrmap["NJ_wallpaper_name"]=NJWallpaperstatus;
        parametrmap["NJ_Wallpaper_Status"] = "FAIL";
        if(!NjwallpaperFilename.empty()){
        if(NjwallpaperFilename.find(NJWallpaperstatus) != std::string::npos){
            parametrmap["NJ_Wallpaper_Status"] = "PASS";
        }
        }
       }

        if(parameters.find("NJcare_App_Version_Status")!=std::string::npos){
        string njcareappversioncommand =
    "powershell.exe -Command \"[System.Diagnostics.FileVersionInfo]::GetVersionInfo("
    "'C:\\Program Files (x86)\\NJcare\\unins000.exe'"
    ").ProductVersion\"";

        string njcareappversionstatus = util.executeTerminal(njcareappversioncommand);
        // cout << "NJ Care App Version : " << njcareappversionstatus << endl;
        
       

        // Checking if the version is fetched correctly
        if (njcareappversionstatus.find(nj_care_version) != string::npos) {
        parametrmap["NJcare_App_Version_Status"] = "PASS";
        } else {
        parametrmap["NJcare_App_Version_Status"] = "FAIL";
        }

        if(parametrmap["NJcare_App_Version_Status"] != "PASS"){
            string installcommand="NJCare.exe /SILENT /NOCANCEL";
            util.executeTerminal(installcommand);
            njcareappversionstatus = util.executeTerminal(njcareappversioncommand);
            if (njcareappversionstatus.find(nj_care_version) != string::npos) {
                parametrmap["NJcare_App_Version_Status"] = "PASS";
                } else {
                parametrmap["NJcare_App_Version_Status"] = "FAIL";
            }
        }
        if(njcareappversionstatus.length()>140){
            parametrmap["NJcare_App_Version"] = njcareappversionstatus.substr(0, 140);
            parametrmap["NJcare_App_Version_Status"] = "FAIL";

        }else{
            parametrmap["NJcare_App_Version"] = njcareappversionstatus;
        }
        
        }

       if(parameters.find("Disk_Unallocated_Status")!=std::string::npos){
            std::string Script = 
            "powershell -NoProfile -Command \""
            "$disks = Get-Disk | Select-Object Number, Size, AllocatedSize; "
            "$threshold = 1GB; "
            "foreach ($disk in $disks) { "
            "    $unallocatedSpace = $disk.Size - $disk.AllocatedSize; "
            "    if ($unallocatedSpace -gt $threshold) { "
            "        Write-Host \\\"Unallocated Space for Disk $($disk.Number): $([math]::Round($unallocatedSpace / 1GB, 2)) GB\\\"; "
            "    } "
            "}\"";

            string result=util.executeTerminal(Script);
            cout<<result;

            if(result.empty()){
                parametrmap["Disk_Unallocated_Status"]="PASS";
            }else{
                parametrmap["Disk_Unallocated_Status"]="FAIL";
                parametrmap["Disk_Unallocated_space"]=result;
            }
    
       }

        fieldmap["stock_info_id"] = stockinfoID;
        fieldmap["part_id"] = partId;
        // fieldmap["part_config_id"] = partconfigid;
        fieldmap["item_id"] = item_id;
        fieldmap["created_by"] = created_by;
   
       
        return_value = partresultservice.updateBulkPartResult(parametrmap, fieldmap);

        // Sync the check result with IQC service
        iqcservice.ChecksResultSync("System Check"); 

        cout << "\nSystem check is Completed..." << endl;
        return return_value;
    }

    bool checkMinidumpFiles(const std::string& directoryPath) {
    try {
       
        if (!fs::exists(directoryPath)) {
            std::cerr << "Directory does not exist: " << directoryPath << std::endl;
            return false;
        }

        if (!fs::is_directory(directoryPath)) {
            std::cerr << "Path is not a directory: " << directoryPath << std::endl;
            return false;
        }

        // Iterate through files in the directory
        int file_count=0;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry)) {
                string file_path=entry.path().string();
                string updated_name= getfromERP.updateFile2ERp(file_path.c_str());
                file_count+=1;
                if(updated_name!=""){
                    parametrmap["BSOD_dump_file_"+to_string(file_count)]="https://erp.newjaisa.com/app/file/"+updated_name;
                }else{
                    parametrmap["BSOD_dump_file_"+to_string(file_count)]="Not able to upload file";
                }
                // std::cout << entry.path().string() << std::endl;
                // auto fileSize = fs::file_size(entry);
                // cout<<"File Size : "<<fileSize<<endl;
                // if (fileSize > 0) {
                //     std::cout << "Non-empty file found: " << entry.path().string()
                //               << " (Size: " << fileSize << " bytes)" << std::endl;
                   
                // }
            }
        }
        if(file_count==0){return false;}


    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return true;
}

json get_bsod_info(const std::string& directoryPath) {
    json jsonArray = json::array();
    map<string, string> resultData;
    string file_path = "BSOD_Report.txt";
    string blue_screen_error = "BlueScreenView.exe /LoadDump \"C:\\Windows\\Minidump\" /stext \"BSOD_Report.txt\"";
    util.executeTerminal(blue_screen_error);
    Sleep(5000);

    std::wifstream file(file_path);
    file.imbue(std::locale(file.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));
    
    if (!file.is_open()) {
        wcerr << L"Failed to open the file." << endl;
        return jsonArray;
    }

    std::wstring line;
    
    while (std::getline(file, line)) {
        string string_line=narrow(line);
        if (string_line.find("=====") != std::string::npos) {
          if(!resultData.empty()){
            json jsonObject = resultData; 
            resultData.clear();
            jsonArray.push_back(jsonObject);
        }
        continue;
        }

        size_t delimiterPos = string_line.find(':');
        if (delimiterPos != std::string::npos) {
            std::string key = string_line.substr(0, delimiterPos);
            std::string value = string_line.substr(delimiterPos+1);
            key=util.trim(key);
            value=util.trim(value);
            resultData[key] = value;
        }
    }
    if(!resultData.empty()){
        json jsonObject = resultData; 
        jsonArray.push_back(jsonObject);
    }

    file.close();
    hdd.deletefile(file_path);
    return jsonArray;
}
// bool isInstallationDateOlderThanSixMonths(const std::string& installDateString) {
//     // Define a regular expression to extract the date in MM/DD/YYYY format
//     std::regex dateRegex(R"((\d{1,2})/(\d{1,2})/(\d{4}))");
//     std::smatch match;

//     // Search for date in the input string
//     if (std::regex_search(installDateString, match, dateRegex)) {
//         int month = std::stoi(match[1].str());
//         int day = std::stoi(match[2].str());
//         int year = std::stoi(match[3].str());

//         // Create a tm struct for the installation date
//         std::tm installDate = {};
//         installDate.tm_year = year - 1900;  
//         installDate.tm_mon = month - 1;     
//         installDate.tm_mday = day;

//         // Convert install date to time_t
//         std::time_t installTime = std::mktime(&installDate);

//         if (installTime == -1) {
//             std::cerr << "Error: Failed to convert installation date to time_t." << std::endl;
//             return false;
//         }

//         std::time_t currentTime = std::time(nullptr);

//         double durationInSeconds = std::difftime(currentTime, installTime);

//         const int secondsPerDay = 60 * 60 * 24;
//         int durationInDays = static_cast<int>(durationInSeconds / secondsPerDay);
//         // cout<<"\nduration days:"<<durationInDays<<endl;
//         if (durationInDays > 180) {
//             return false;  
//         } else {
//             return true; 
//         }
//     } else {
//         std::cerr << "Error: Date format not found in the input string." << std::endl;
//         return false;
//     }
// }

// bool isInstallationDateOlderThanSixMonths(const std::string& installDateString) {
    
//     std::tm installDate = {};
//     std::istringstream dateStream(installDateString);

//     dateStream.ignore(std::numeric_limits<std::streamsize>::max(), ' '); 
//     dateStream >> std::get_time(&installDate, "%B %d, %Y");              

//     if (dateStream.fail()) {
//         std::cerr << "Error: Failed to parse installation date." << std::endl;
//         return false;
//     }

//     installDate.tm_hour = 0; 
//     installDate.tm_min = 0;
//     installDate.tm_sec = 0;

//     std::time_t installTime = std::mktime(&installDate);
//     if (installTime == -1) {
//         std::cerr << "Error: Failed to convert installation date to time_t." << std::endl;
//         return false;
//     }

//     // Get the current date (ignoring time)
//     std::time_t currentTime = std::time(nullptr);
//     std::tm* currentDate = std::localtime(&currentTime);
//     currentDate->tm_hour = 0;
//     currentDate->tm_min = 0;
//     currentDate->tm_sec = 0;
//     currentTime = std::mktime(currentDate);

//     if (currentTime == -1) {
//         std::cerr << "Error: Failed to get the current date as time_t." << std::endl;
//         return false;
//     }

//     // Calculate the difference in days
//     const int secondsPerDay = 60 * 60 * 24;
//     int durationInDays = static_cast<int>(std::difftime(currentTime, installTime) / secondsPerDay);
//     // cout<<"no of days:"<<durationInDays<<endl;
   
//     if (durationInDays > 180) {
//         return false; 
//     } else {
//         return true;
//     }
// }

public:void updateNJwallpaperFilename(string department,json result){
    // json result=njsetting.getNjSettingsvalues();
    for(const auto&item: result){
      if(item["department"]==department && item["parameter"]=="njwallpaper_filename"){
        NjwallpaperFilename=item["value"].get<string>();
      }else if(item["parameter"]=="NJ_care_version"){
        nj_care_version=item["value"].get<string>();
      }
    }
  }

//   public:void updateNJwallpaperFilename(string department,json result){
//     // json result=njsetting.getNjSettingsvalues();
//     for(const auto&item: result){
//       if(item["department"]==department && item["parameter"]=="njwallpaper_filename"){
//         NjwallpaperFilename=item["value"].get<string>();
 
//         break;
//       }
//     }
//   }
void performBatteryDischargeCheck(string stockinfoID, string partId, string item_id) {
    cout<<"\nDischarging Test Running..."<<endl;
    BatteryCheck battery;
    const string cmd_battery1 = "powershell -Command \" Get-WmiObject -Class Win32_Battery | Format-List BatteryStatus\""; 
    while(true){
        if(battery.isdischarging(cmd_battery1) || battery.isdischarging2(cmd_battery1)){
            break;
        }else{
          util.generatePopupOK("pls remove power cable for discharging Check and then Click OK");
          
        }
    }

    double total_cpu_idle = 0.0;
    double total_discharge_idle = 0.0;
    double total_cpu_heavy = 0.0;
    double total_discharge_heavy = 0.0;
    map<string, string> paramtrmap;
    map<string, string> fieldmap;

    fieldmap["stock_info_id"] = stockinfoID;
    fieldmap["part_id"] = partId;
    fieldmap["item_id"] = item_id;
    // fieldmap["created_by"] = created_by;

    for (int i = 0; i <= 1; i++) {
        this_thread::sleep_for(chrono::minutes(1));
        int battery_percentage = completeBatteryCheck.Battery_percentage(CMD_BATTERY);
        string discharging_time = completeBatteryCheck.getCurrentTime();
        string CPU_load = completeBatteryCheck.cpuLoad_percentage();
        string dischaging_rate = completeBatteryCheck.discharge_rate();

        try {
            // Only add to totals if strings are not empty and can be converted
            if (!CPU_load.empty()) {
                total_cpu_idle += stod(CPU_load);
            }
            if (!dischaging_rate.empty()) {
                total_discharge_idle += stod(dischaging_rate);
            }
        } catch (const std::invalid_argument& e) {
            cout << "Invalid number format in CPU load or discharge rate: " << e.what() << endl;
        }
        
        paramtrmap[discharging_time] = to_string(battery_percentage);
        // paramtrmap["Test_Condition"] = "IDLE";
        completeBatteryCheck.update_data(paramtrmap, fieldmap, CPU_load, dischaging_rate);
        paramtrmap.clear();
    }

    cout<<"Discharging Test with Load Running..."<<endl;
    for (int i = 0; i <= 1; i++) {
               
        this_thread::sleep_for(chrono::minutes(1));
    
        int battery_percentage = completeBatteryCheck.Battery_percentage(CMD_BATTERY);
        string discharging_time = completeBatteryCheck.getCurrentTime();
        string CPU_load = completeBatteryCheck.cpuLoad_percentage();
        string dischaging_rate = completeBatteryCheck.discharge_rate();

        try {
            if (!CPU_load.empty()) {
                total_cpu_heavy += stod(CPU_load);
            }
            if (!dischaging_rate.empty()) {
                total_discharge_heavy += stod(dischaging_rate);
            }
        } catch (const std::invalid_argument& e) {
            cout << "Invalid number format in CPU load or discharge rate: " << e.what() << endl;
        }
    
        // Store battery percentage by timestamp
        paramtrmap[discharging_time] = to_string(battery_percentage);
        // paramtrmap["Test_Condition"] = "HEAVY_LOAD";
    
        completeBatteryCheck.update_data(paramtrmap, fieldmap, CPU_load, dischaging_rate);
        paramtrmap.clear();
    }


    // Calculate and update averages
    double avg_cpu_idle = total_cpu_idle / 2.0;
    double avg_discharge_idle = total_discharge_idle / 2.0;
    double avg_cpu_heavy = total_cpu_heavy / 2.0;
    double avg_discharge_heavy = total_discharge_heavy / 2.0;

    paramtrmap["Average_CPU_Load_IDLE"] = to_string(avg_cpu_idle);
    paramtrmap["Average_Discharge_Rate_IDLE"] = to_string(avg_discharge_idle);
    paramtrmap["Average_CPU_Load_HEAVY"] = to_string(avg_cpu_heavy);
    paramtrmap["Average_Discharge_Rate_HEAVY"] = to_string(avg_discharge_heavy);
    
    completeBatteryCheck.update_data(paramtrmap, fieldmap);
 
}
    public:    bool runsoftwareTest(string result){
        std::vector<std::string>softwareCheckInfo;
        softwareCheckInfo.push_back("Boot_Mode_Status");
        softwareCheckInfo.push_back("Windows_Installation_Date_Status");
        softwareCheckInfo.push_back("Office_Activation_Status");
        softwareCheckInfo.push_back("Windows_Activation_Status");
        softwareCheckInfo.push_back("OS_Version");
        softwareCheckInfo.push_back("Secure_Boot_Status");
        softwareCheckInfo.push_back("Windows_Version_Status");
        softwareCheckInfo.push_back("NJ_App_Installation_Status");
        softwareCheckInfo.push_back("BSOD_Files_Status");
        softwareCheckInfo.push_back("Password_Expire_Status");
        softwareCheckInfo.push_back("NJ_Wallpaper_Status");
        softwareCheckInfo.push_back("NJcare_App_Version_Status");
        softwareCheckInfo.push_back("Disk_Unallocated_Status");
        
        
    

        for(const std::string& bluetoothcheck:softwareCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }

    public:    bool rundischargingTest(string result){
        std::vector<std::string>softwareCheckInfo;
        softwareCheckInfo.push_back("Average_CPU_Load_IDLE");
        softwareCheckInfo.push_back("Average_Discharge_Rate_IDLE");
        softwareCheckInfo.push_back("Average_CPU_Load_HEAVY");
        softwareCheckInfo.push_back("Average_Discharge_Rate_HEAVY");

        for(const std::string& bluetoothcheck:softwareCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
};
string SystemCheck::NjwallpaperFilename ;
string SystemCheck::nj_care_version;
string SystemCheck::windows_version;