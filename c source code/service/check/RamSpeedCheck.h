#include <iostream>
#include <string>
#include <map>

using namespace std;

class RAMspeedCheck : public V2Service {
    IQCService iqcservice;
    Util util; 
    PartResultService partresultservice; 

    public:
    int RAMspeedTest(string stockinfoID, string partId, string partconfigid, string item_id, string created_by,map<int,map<string,string>>config_data) {
        cout << "\nRam Speed checking..." << endl;
        int return_value = 0;
        map<string, string> parametermap;
        map<string, string> fieldmap;
        vector<string> ram_speed;
        int no_of_ram=0;
        map<int, map<string, string>> ::iterator it = config_data.begin();
        while(it!=config_data.end()){
        map<string, string> partdata = it->second;
        string partname=partdata["njhat_item_name"];
        if(partname=="RAM"){
            if(!partdata["attr_02"].empty()){
                no_of_ram++;
                parametermap["RAM_Speed"+to_string(no_of_ram)] = partdata["attr_02"];
                ram_speed.push_back(partdata["attr_02"]);
            }

        }
        ++it;
        }
        bool are_all_values_same = std::all_of(ram_speed.begin(), ram_speed.end(), 
    [&](const std::string& speed) { return speed == ram_speed[0]; });

        if (are_all_values_same) {
            parametermap["Multiple_RAM_Speed_Status"] = "PASS";
        } else {
            parametermap["Multiple_RAM_Speed_Status"] = "FAIL";
        }
      
        fieldmap["stock_info_id"] = stockinfoID;
        fieldmap["part_id"] = partId;
        fieldmap["part_config_id"] = partconfigid;
        fieldmap["item_id"] = item_id;
        fieldmap["created_by"] = created_by;   
        return_value = partresultservice.updateBulkPartResult(parametermap, fieldmap);
        iqcservice.ChecksResultSync("Battery Check");
        cout<<"\nRam Speed check Completed..."<<endl;
        return return_value;
    }
    // public:
    // int RAMspeedTest(map<int,map<string,string>>config_data) {
    //     cout << "\nRam Speed checking..." << endl;
    //     int return_value = 0;
    //     map<string, string> parametermap;
    //     map<string, string> fieldmap;
    //     vector<string> ram_speed;
    //     int no_of_ram=0;
    //     map<int, map<string, string>> ::iterator it = config_data.begin();
    //     while(it!=config_data.end()){
    //     map<string, string> partdata = it->second;
    //     string partname=partdata["njhat_item_name"];
    //     if(partname=="RAM"){
    //         fieldmap["stock_info_id"]=partdata["stock_info_id"];
    //         fieldmap["stock_part_id"]=partdata["stock_part_id"];
    //         fieldmap["part_config_id"]=partdata["part_config_id"];
    //         fieldmap["item_id"]=partdata["item_id"];
    //         fieldmap["created_by"]=partdata["created_by"];

    //         if(!partdata["attr_02"].empty()){
    //             no_of_ram++;
    //             parametermap["RAM_Speed"+to_string(no_of_ram)] = partdata["attr_02"];
    //             ram_speed.push_back(partdata["attr_02"]);
    //         }

    //     }
    //     ++it;
    //     }
    //     bool are_all_values_same = std::all_of(ram_speed.begin(), ram_speed.end(), 
    // [&](const std::string& speed) { return speed == ram_speed[0]; });

    //     if (are_all_values_same) {
    //         parametermap["Multiple_RAM_Speed_Status"] = "PASS";
    //     } else {
    //         parametermap["Multiple_RAM_Speed_Status"] = "FAIL";
    //     }
      
    //     // fieldmap["stock_info_id"] = stockinfoID;
    //     // fieldmap["part_id"] = partId;
    //     // fieldmap["part_config_id"] = partconfigid;
    //     // fieldmap["item_id"] = item_id;
    //     // fieldmap["created_by"] = created_by;   
    //     return_value = partresultservice.updateBulkPartResult(parametermap, fieldmap);
    //     iqcservice.ChecksResultSync("Battery Check");
    //     cout<<"\nRam Speed check Completed..."<<endl;
    //     return return_value;
    // }

      public:    bool runRAMTest(string result){
        std::vector<std::string>RAMCheckInfo;
        RAMCheckInfo.push_back("Multiple_RAM_Speed_Status");
        // RAMCheckInfo.push_back("TabSwitchTime");
        // LIDCheckInfo.push_back("HDMI_Status");

        // driverCheckInfo.push_back("Error_Drivers_Count");

        for(const std::string& bluetoothcheck:RAMCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
};