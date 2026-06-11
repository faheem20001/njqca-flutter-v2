#include <iostream>
#include <string>
#include <map>
#include <vector>

using namespace std;


class AdapterCheck: public V2Service {

  private:
    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;
    GetFromERPService getfromERP;
    Constant constant;
    BatteryCheck battery;
    int minChargingSpeed;
    int maxChargingSpeed ;
    

  public:
    std::thread loopThread;
    void AdapterTest(string stockinfoID, string partId,  string item_id) {
        cout << "\nAdapter check is Running..." << endl;
        const string CMD_video="powershell -Command \"Start-Process -FilePath 'chrome.exe' -ArgumentList 'https://www.youtube.com/watch?v=7bOptq-NPJQ'\"";

        int return_value;
        map<string, string> parametrmap;
        map<string, string> fieldmap;
        string item_group=iqcservice.item_group;
        // vector<string> ranges;
        json input;
        input["bios_serial_no"]=iqcservice.bios_serial_no;
        int wattage=65;
        string adaptor_range="";
        if(item_group != "Laptop Adaptors"){
            adaptor_range=iqcservice.getParameterList(constant.API_adaptor_range, "post", parametrmap, to_string(input));
            // cout<<"adaptor_range: "<<adaptor_range<<endl;
            while(true){
                if(adaptor_range.empty()|| adaptor_range.find("No Data Found") != string::npos) {

                bool status = util.generatePopup("No Data Found For Adaptor","Please contact NPD Team then click retry?");
                if(status){
                    break;
                }else{
                    adaptor_range=iqcservice.getParameterList(constant.API_adaptor_range, "post", parametrmap, to_string(input));
                }
                }else{
                    break;
                }
                            
            }
            cout<<"adaptor_range: "<<adaptor_range<<endl;
            json data;
            
            try{
                data=json::parse(adaptor_range);
                // ranges.push_back(data["message"]);
                wattage=stoi(util.extractInteger(data["message"]));

                
            }catch(const exception& e){
                cout<<"Error While getting Adaptor range: "<<e.what()<<adaptor_range<<endl;            
            }
        }
        
        
        // if (!ranges.empty()) {
        //    set_min_max(ranges);            
        // }
        // cout<<"minChargingSpeed: "<<minChargingSpeed<<endl;
        // cout<<"maxChargingSpeed: "<<maxChargingSpeed<<endl;
        std::string Script =
        "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
        "$logFile = \\\"$env:USERPROFILE\\\\Desktop\\\\Battery_Monitor_Log.csv\\\"; "
        "if (!(Test-Path $logFile)) { "
        "    'Timestamp,Laptop Brand,Battery Percentage,Charging Status,Voltage (mV),Charge Rate (mW),Wattage (W),Current (mA),CPU Load (%),Issues' | "
        "    Out-File -Append -FilePath $logFile; "
        "} "
        // "$min_charging_speed = " + std::to_string(minChargingSpeed) + "; "
        // "$max_charging_speed = " + std::to_string(maxChargingSpeed) + "; "
        // "Start-Process 'https://silver.urih.com/'; "
        // "Start-Sleep -Seconds 5; "
        "Write-Host 'Monitoring Adapter and Battery Performance... Press Ctrl+C to stop.'; "
        "$startTime = Get-Date; "
        "$endTime = $startTime.AddMinutes(5); "
        "$computerSystem = Get-WmiObject -Class Win32_ComputerSystem; "
        "$laptopBrand = $computerSystem.Manufacturer; "
        "Write-Host \"Detected Laptop Brand: $laptopBrand\" -ForegroundColor Cyan; "
        "while ((Get-Date) -lt $endTime) { "
        "$battery = Get-WmiObject -Namespace 'root\\WMI' -Class 'BatteryStatus' |Where-Object { $_.Charging -eq $true -or $_.Discharging -eq $true } |Select-Object -First 1;"
        "$power = Get-WmiObject -Namespace 'root\\CIMV2' -Class 'Win32_Battery' | Select-Object -First 1; "
        "$adapterConnected = $power.BatteryStatus -ne 1; "
        "$batteryPercentage = $power.EstimatedChargeRemaining;"
        "$chargingStatus = $power.BatteryStatus;"
        "$voltage = [double]($battery.Voltage -as [double]); "
        "$chargeRate = [double]($battery.ChargeRate -as [double]); "
        "if ($voltage -gt 0) { $voltage_V = $voltage / 1000.0 } else { $voltage_V = 'Unknown' }; "
        "if ($chargeRate -gt 0) { $wattage = $chargeRate / 1000.0 } else { $wattage = 0 }; "
        "if ($voltage_V -is [double] -and $voltage_V -gt 0) { "
        "    $current_mA = [math]::Round(($chargeRate / $voltage_V), 2) "
        "} else { $current_mA = 'Unknown' }; "
        "$cpuLoad = (Get-WmiObject win32_processor | Measure-Object -Property LoadPercentage -Average).Average; "
        "$issues = @(); "
        "if ($batteryPercentage -eq 100 -and $chargeRate -eq 0) { "
        "    $statusText = 'Not Charging (Fully Charged)' "
        "} elseif ($chargingStatus -eq 2) { "
        "    $statusText = 'Charging' "
        "} elseif ($chargingStatus -eq 3) { "
        "    $statusText = 'Not Charging'; $issues += 'Adapter connected but not charging' "
        "} elseif ($chargingStatus -eq 4) { "
        "    $statusText = 'Charging Uncontrollable'; $issues += 'Unstable charging detected' "
        "} elseif ($chargingStatus -eq 1) { "
        "    $statusText = 'Not Charging'; $issues += 'Not Charging' "
        "} "
        "if ($voltage -gt 17200) { $issues += 'Overvoltage detected' }; "
        // "if ($laptopBrand -match 'Lenovo') { "
        // "    if ($chargeRate -eq 0 -and $statusText -eq 'Charging') { "
        // "        $issues += 'Charging but no power flow (Check battery threshold settings)' "
        // "    } "
        // "} else { "
        // "    if ($chargeRate -lt 5000 -and $statusText -eq 'Charging') { "
        // "        $issues += 'Low charging speed' "
        // "    } "
        // "} "
        "$logEntry = \\\"$(Get-Date),$laptopBrand,$batteryPercentage,$statusText,$voltage,$chargeRate,$wattage,$current_mA,$cpuLoad,$($issues -join '; ')\\\"; "

        "$logEntry | Out-File -Append -FilePath $logFile; "
        "Write-Host $logEntry -ForegroundColor Green; "
        "Start-Sleep -Seconds 20; "
        "}\"";


        const string cmd_battery1 = "powershell -Command \"Get-WmiObject -Class Win32_Battery |Format-List BatteryStatus,EstimatedChargeRemaining\""; 
        if(!battery.ischarging(cmd_battery1)){
            util.generatePopupOK("Connect Adapter for Adapter battery Check and Click OK");
        }

        // loopThread = std::thread([this]() { this->CPUTemUpLoop(); });
        util.executeTerminal(CMD_video);
        std::thread autopilotThread([]() {
            system("\"AutopilotCheck.exe\"");
        });

        autopilotThread.join();
        const char* userProfile = getenv("USERPROFILE");
        std::string autopilotPath = std::string(userProfile) + "\\Desktop\\Autopilot_Check_Result.txt";
        if (std::filesystem::exists(autopilotPath)) {
            std::cout << "Autopilot result found\n";
        } else {
            std::cout << "Autopilot result NOT found\n";
        }
         if (std::filesystem::exists(autopilotPath)) {
            cout<<"autopilot file updating to erp"<<endl;
            
            std::string uploadedName = getfromERP.updateFile2ERp(autopilotPath.c_str());
            cout<<"autopilot uploadedName: "<<uploadedName<<endl;
            if (uploadedName != "") {
                parametrmap["Autopilot_File_Uploaded"] = uploadedName;
            } else {
                parametrmap["Autopilot_File_Uploaded"] = "Upload failed";
            }
        } else {
            parametrmap["Autopilot_File_Uploaded"] = "File Not Found";
        }

        int initial_percentage=battery.Battery_percentage(cmd_battery1);
        cout<<"\nInitial percentage: "<<initial_percentage<<endl;
        std ::string Adapter_result=util.executeTerminal(Script);
        int final_percentage=battery.Battery_percentage(cmd_battery1);
        cout<<"\nFinal percentage: "<<final_percentage<<endl;
        parametrmap["initial_percentage"]=to_string(initial_percentage);
        parametrmap["final_percentage"]=to_string(final_percentage);
        string charging_gain_status="";
        if(item_group!="Laptop Adaptors"){
            cout<<"Wattage: "<<wattage<<endl;
            parametrmap["Repacked_adapter_wattage"]=to_string(wattage);
            charging_gain_status=checkChargingGain(wattage,initial_percentage,final_percentage);
            cout<<"charging_gain_status: "<<charging_gain_status<<endl;
        }
        // if(loopThread.joinable()) loopThread.join();
        std::istringstream adapterStream(Adapter_result);
        std::string line;
        std::vector<std::string> readings;

        while (std::getline(adapterStream, line)) {

            size_t pos = line.find(',');
            if(pos!=std::string::npos){
            std::string beforeComma = line.substr(0, pos);
            std::string afterComma = line.substr(pos + 1);
            if(afterComma.length()>140){
                afterComma=afterComma.substr(0,140);
            }
              parametrmap[beforeComma]=afterComma;
            }
        }
        cout<<Adapter_result<<endl;
        

        if(Adapter_result.find("Adapter connected but not charging")!=std::string::npos ||Adapter_result.find("Overvoltage detected")!=std::string::npos ||Adapter_result.find("Unstable charging detected")!=std::string::npos || Adapter_result.find("Not Charging")!=std::string::npos){
            parametrmap["Adapterbattery_status"]="FAIL";
        }else{
            if(charging_gain_status!=""){
                if(charging_gain_status=="Pass"){
                    parametrmap["Adapterbattery_status"]="PASS";
                }else{
                    parametrmap["Adapterbattery_status"]="FAIL";
                }
            }
        }
        // std::istringstream adapterStream(Adapter_result);
        // std::string line;
        // std::vector<std::string> readings;

        // while(std::getline(adapterStream, line)){
        //     size_t pos = line.find(',');
        //     if(pos != std::string::npos){
        //         std::string beforeComma = line.substr(0, pos);
        //         std::string afterComma = line.substr(pos + 1);
        //         parametrmap[beforeComma] = afterComma;
        //     }
        // }   

        fieldmap["stock_info_id"] = stockinfoID;
        fieldmap["part_id"] = partId;
       
        fieldmap["item_id"] = item_id;

       
        return_value = partresultservice.updateBulkPartResult(parametrmap, fieldmap);
      
        iqcservice.ChecksResultSync("Adappter Check");
        cout << "\nAdapter check is Completed..." << endl;
    }

    public:    bool runAdapterTest(string result){
        std::vector<std::string>runAdapterCheckInfo;

        runAdapterCheckInfo .push_back("Adapterbattery_status");

        for(const std::string& bluetoothcheck:runAdapterCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }

    public:void CPUTemUpLoop(){
    int fan1Speed=1;
    int  fan2Speed=1;
    int res;
    bool startloop=true;
    auto start = std::chrono::steady_clock::now();
    map<string, string>fan_speed;
    cout<<"\nRunning loop..."<<endl;
    while(startloop){
    //   res=fan1Speed*fan2Speed/fan2Speed;     
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
        if (duration.count()>60){
         startloop=false;
        }
    }
    }

    void set_min_max(std::vector<std::string> ranges){

        // Define the map for wattage ranges
        std::unordered_map<int, std::pair<int, int>> wattageRanges = {
            {45, {24300, 45000}},
            {65, {34100, 65000}},
            {90, {41400, 85000}},
            {130, {70200, 130000}},
            {170, {91800, 170000}}
        };

        std::vector<int> wattValues;

        for (const auto& item : ranges) {
            try {
                int watt = std::stoi(item); 
                if (wattageRanges.count(watt)) {
                    wattValues.push_back(watt);
                } else {
                    std::cerr << "Unknown wattage: " << watt << std::endl;
                }
            } catch (...) {
                std::cerr << "Invalid format: " << item << std::endl;
            }
        }

        if (wattValues.empty()) {
            std::cerr << "No valid wattage entries found.\n";
        }

        // Find lowest and highest watt value
        int minWatt = *std::min_element(wattValues.begin(), wattValues.end());
        int maxWatt = *std::max_element(wattValues.begin(), wattValues.end());

        // Extract the min/max values for those wattages
        minChargingSpeed = wattageRanges[minWatt].first;
        maxChargingSpeed = wattageRanges[maxWatt].second;
    
    }
     
    string checkChargingGain(int wattage, int initial_percentage, int final_percentage) {
    int gain = final_percentage- initial_percentage;
    int minGain = 0, maxGain = 0;

    string socRange;
    if (initial_percentage < 20) socRange = "0-20";
    else if (initial_percentage < 40) socRange = "20-40";
    else if (initial_percentage < 70) socRange = "40-70";
    else if (initial_percentage < 80) socRange = "70-80";
    else if (initial_percentage < 95) socRange = "80-95";
    else socRange = "95-100";

    if (socRange == "0-20") {
        if (wattage == 45) { minGain = 3; maxGain = 3; }
        else if (wattage == 65) { minGain = 4; maxGain = 7; }
        else if (wattage == 90) { minGain = 6; maxGain = 9; }
        else if (wattage == 130) { minGain = 7; maxGain = 12; }
        else if (wattage >= 150 && wattage <= 170) { minGain = 8; maxGain = 14; }
        else if (wattage == 200) { minGain = 9; maxGain = 16; }
    }
    else if (socRange == "20-40") {
        if (wattage == 45) { minGain = 2; maxGain = 4; }
        else if (wattage == 65) { minGain = 3; maxGain = 6; }
        else if (wattage == 90) { minGain = 5; maxGain = 8; }
        else if (wattage == 130) { minGain = 6; maxGain = 11; }
        else if (wattage >= 150 && wattage <= 170) { minGain = 7; maxGain = 13; }
        else if (wattage == 200) { minGain = 8; maxGain = 15; }
    }
    else if (socRange == "40-70") {
        if (wattage == 45) { minGain = 2; maxGain = 3; }
        else if (wattage == 65) { minGain = 4; maxGain = 5; }
        else if (wattage == 90) { minGain = 5; maxGain = 8; }
        else if (wattage == 130) { minGain = 6; maxGain = 9; }
        else if (wattage >= 150 && wattage <= 170) { minGain = 7; maxGain = 11; }
        else if (wattage == 200) { minGain = 8; maxGain = 13; }
    }
    else if (socRange == "70-80") {
        if (wattage == 45) { minGain = 1; maxGain = 2; }
        else if (wattage == 65) { minGain = 2; maxGain = 3; }
        else if (wattage == 90) { minGain = 3; maxGain = 5; }
        else if (wattage == 130) { minGain = 4; maxGain = 6; }
        else if (wattage >= 150 && wattage <= 170) { minGain = 5; maxGain = 7; }
        else if (wattage == 200) { minGain = 6; maxGain = 8; }
    }
    else if (socRange == "80-95") {
        if (wattage == 45) { minGain = 1; maxGain = 1; }
        else if (wattage == 65) { minGain = 1; maxGain = 2; }
        else if (wattage == 90) { minGain = 2; maxGain = 3; }
        else if (wattage == 130) { minGain = 2; maxGain = 4; }
        else if (wattage >= 150 && wattage <= 170) { minGain = 3; maxGain = 4; }
        else if (wattage == 200) { minGain = 3; maxGain = 5; }
    }
    else if (socRange == "95-100") {
        if (wattage == 45) { minGain = 0; maxGain = 0; }
        else if (wattage == 65) { minGain = 0; maxGain = 1; }
        else if (wattage == 90) { minGain = 1; maxGain = 1; }
        else if (wattage == 130) { minGain = 1; maxGain = 1; }
        else if (wattage >= 150 && wattage <= 170) { minGain = 1; maxGain = 2; }
        else if (wattage == 200) { minGain = 1; maxGain = 2; }
    }
    if (gain >= minGain)
        return "Pass";
    else
        return "Fail";
}
};