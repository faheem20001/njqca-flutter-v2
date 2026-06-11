#include <iostream>
#include <string>

using namespace std;

class CMOSErrorCheck: public V2Service {

    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;

    public:void CMOSErrortest(string stockinfoID,string partId, string partconfigid, string item_id,string created_by){
      map<string,string> parametrmap;
      map<string,string> fieldmap; 
      cout<<"\nCMOS ERROR Check Running..."<<endl;
      // string comsErrorcommand="powershell -Command \"(Get-WinEvent -LogName System | Where-Object {$_.ProviderName -like '*Time-Service*' -and ($_.LevelDisplayName -eq 'Warning' -or $_.LevelDisplayName -eq 'Error') } | Measure-Object).Count\"";
      std::string script =
      "powershell -NoProfile -Command \""
      "$installDate = (Get-CimInstance -ClassName Win32_OperatingSystem).InstallDate; "
      "$currentDate = Get-Date; "
      "$criticalEvents = Get-WinEvent -LogName System -MaxEvents 100 -ErrorAction SilentlyContinue | "
      "Where-Object { "
      "($_.ProviderName -like '*Time-Service*') -and "
      "($_.Level -eq 1 -or $_.LevelDisplayName -eq 'Error') -and "
      "$_.TimeCreated -ge $installDate "
      "}; "
      "if ($criticalEvents) { "
      "Write-Host \\\"FAILED : (CRITICAL: Found $($criticalEvents.Count) time service errors since OS installation)\\\"; "
      "exit 0; "
      "}; "
      "$ntpEvents = Get-WinEvent -LogName System -ErrorAction SilentlyContinue | "
      "Where-Object { "
      "$_.ProviderName -eq '*Time-Service*' -and "
      "$_.TimeCreated -ge $installDate -and "
      "$_.TimeCreated -le $currentDate "
      "} | Sort-Object TimeCreated; "
      "$consecutiveDnsErrors = 0; "
      "$previousWas134 = $false; "
      "foreach ($event in $ntpEvents) { "
      "if ($event.Id -eq 158) { continue }; "
      "if ($event.Id -eq 34) { "
      "Write-Host \\\"CRITICAL: DNS error detected at $($event.TimeCreated)\\\"; "
      "exit 0; "
      "} "
      "}; "
      "Write-Host \\\"PASSED: (All time service checks completed successfully)\\\"; "
      "exit 0; "
      "\"";   
  

      string result=util.executeTerminal(script);
      cout<<result<<endl;

      // result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
      // result=util.trim(result);
      try{
        if (result.length() > 140) {
          result = result.substr(result.length() - 140); 
        }
      parametrmap["CMOS_result"]=result;
  
      if(result.find("PASSED")!=std::string::npos){
        parametrmap["CMOSError_Check_Status"]="PASS";
        // parametrmap["CMOSError_Count"]=result;
      }else{
        parametrmap["CMOSError_Check_Status"]="FAIL"; 
        // parametrmap["CMOSError_Count"]=result;
      }
      }catch(const std::exception& e) {
        cout<<"Unble to get CMOS status:"<<result<<endl;
      }


      fieldmap["stock_info_id"]=stockinfoID;
      fieldmap["part_id"]=partId;
      fieldmap["part_config_id"]=partconfigid;
      fieldmap["item_id"]=item_id;
      fieldmap["created_by"]=created_by;
      partresultservice.updateBulkPartResult(parametrmap,fieldmap);
      iqcservice.ChecksResultSync("CMOS Check");
      cout<<"\nCMOSError Check Completed..."<<endl;
    }
      public:    bool runCMOSTest(string result){
      std::vector<std::string>CMOSCheckInfo;
      CMOSCheckInfo.push_back("CMOSError_Check_Status");
      CMOSCheckInfo.push_back("CMOSError_Count");

      for(const std::string& bluetoothcheck:CMOSCheckInfo){
          if(result.find(bluetoothcheck)!=std::string::npos){
              return true;
              break;
          }
      }
      return false;

  }
};