#include <iostream>
#include <string>

using namespace std;

class FingerprintCheck: public V2Service {

    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;

    public:void FingerprintTest(string stockinfoID,string partId, string item_id){
        map<string,string> parametermap;
        map<string,string> fieldmap; 
        string fingerprintcommand="fingerprint.exe";
        cout<<"\nFingerprint Check Running..."<<endl;
        string fingerprintresult=util.executeTerminal(fingerprintcommand);
        if(fingerprintresult.find("Not Available") != std::string::npos){
            parametermap["FingerPrint_Status"]="Not Available";
        }
        else if(fingerprintresult.find("working")!=std::string::npos){
            parametermap["FingerPrint_Status"]="Working";
        }else{
            parametermap["FingerPrint_Status"]="Not Working";
        }

        if(parametermap["FingerPrint_Status"]=="Not Working"){
            while(true){
                bool status=util.generatePopup("Fingerprint Not Detected", "If you want retry Click Retry else Click Not Working?");
                if(status){
                  break;
                }else{
                   fingerprintresult=util.executeTerminal(fingerprintcommand);
                   if(fingerprintresult.find("Not Available") != std::string::npos){
                        parametermap["FingerPrint_Status"]="Not Available";
                        break;
                    }
                    else if(fingerprintresult.find("working")!=std::string::npos){
                        parametermap["FingerPrint_Status"]="Working";
                        break;
                    }
                }
            }
        }
        // cout<<cameraresult<<endl;
        fieldmap["stock_info_id"]=stockinfoID;
        fieldmap["part_id"]=partId;
        // fieldmap["part_config_id"]=partconfigid;
        fieldmap["item_id"]=item_id;
        // fieldmap["created_by"]=created_by;
        partresultservice.updateBulkPartResult(parametermap,fieldmap);
        iqcservice.ChecksResultSync("Fingerprint Check");
        cout<<"\nFingerprint Check Completed..."<<endl;
    }
    
    public:    bool runFingerTest(string result){
        std::vector<std::string>fingerCheckInfo;
        fingerCheckInfo.push_back("FingerPrint_Status");
        // driverCheckInfo.push_back("Error_Drivers_Count");

        for(const std::string& bluetoothcheck:fingerCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
};