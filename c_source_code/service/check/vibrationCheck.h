#include <iostream>
#include <string>
namespace fs = std::filesystem;


using namespace std;

class VibrationCheck: public V2Service {

    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;
    GetFromERPService getfromERP;
    HDDSentinelCheck hdd;

    public:
      static string vibration_results;
      static string qi_reference;
      static bool  runTest;
       
    public:void VibrationTest(string stockinfoID,string partId, string partconfigid,string item_id,string created_by){
        map<string,string> parametrmap;
        map<string,string> fieldmap; 
        string File_updated_name;
        cout<<"\nVibration Check Running..."<<endl;
        parametrmap["Vibration_test_status"]="PASS";

        if(runTest){
            parametrmap["qi_reference"]=qi_reference;
        }
        else{
           std::istringstream stream(vibration_results);
            std::string line;

            while (std::getline(stream, line)) {
                if (line.find("Vibration") != std::string::npos) {
                    if (line.find("FAIL") != std::string::npos) {
                        parametrmap["Vibration_test_status"]="FAIL" ;
                    } else {
                        parametrmap["Vibration_test_status"]="PASS" ;
                    }
                        if(line.length()>140){
                            line=line.substr(0,140);
                        }
                    parametrmap["Vibration_results"] = line; 

                } 
                else if (line.find("Temperature") != std::string::npos) {
                    if (line.find("FAIL") != std::string::npos) {
                        parametrmap["Temperature_test_result"] = "FAIL";
                    } else {
                        parametrmap["Temperature_test_result"] = "PASS";
                    }
                    if(line.length()>140){
                            line=line.substr(0,140);
                        }
                    parametrmap["Temperature_results"] = line; 
                }
            }

            
            const char* userProfile = std::getenv("USERPROFILE");
            if (userProfile != nullptr) {
                std::string path(userProfile);
                std::string fullPath = path + "\\Desktop\\vibration_sensor_data.csv";
                bool fileExists = hdd.file_exist(fullPath);
                if(fileExists){
                    string File_updated_name=getfromERP.updateFile2ERp(fullPath.c_str());
                    if(File_updated_name!=""){
                        parametrmap["vibration_result_File_Updated_name"]=File_updated_name;
                    }else{
                        parametrmap["vibration_result_File_Updated_name"]="Not able to update the file";
                    }
                }else{
                    parametrmap["vibration_result_File_Updated_name"]="File Not Found";
                }
            }   
        }    
      
        // cout<<cameraresult<<endl;
        fieldmap["stock_info_id"]=stockinfoID;
        fieldmap["part_id"]=partId;
        fieldmap["part_config_id"]=partconfigid;
        fieldmap["item_id"]=item_id;
        fieldmap["created_by"]=created_by;
        partresultservice.updateBulkPartResult(parametrmap,fieldmap);
        iqcservice.ChecksResultSync("Vibration Check");
        cout<<"\nVibration Check Completed..."<<endl;
    }

    public:    bool runVibrationTest(string result){
        std::vector<std::string>cameraCheckInfo;
        cameraCheckInfo.push_back("Vibration_test_status");
        

        for(const std::string& bluetoothcheck:cameraCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
};
string VibrationCheck::vibration_results;
string VibrationCheck::qi_reference;
bool VibrationCheck::runTest=false;