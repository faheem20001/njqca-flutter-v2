#include <iostream>
#include <string>
#include <map>

using namespace std;

class ProductKeyCheck : public V2Service {
    IQCService iqcservice;
    Util util; 
    PartResultService partresultservice;
    public:
    static string Selected_item_wizard;

public:
    int Product_keyTest(string stockinfoID, string partId, string partconfigid, string item_id, string created_by) {
        cout << "\nProdyct Key checking..." << endl;
        int return_value = 0;
        map<string, string> parametrmap;
        map<string, string> fieldmap;
        std::string command = "powershell.exe -Command \"(Get-WmiObject -query 'select * from SoftwareLicensingService').OA3xOriginalProductKey\"";
        std::string key_info = "cscript slmgr.vbs /dlv | findstr \"Key\"";
        std::string name= "cscript slmgr.vbs /dlv | findstr \"Name\"";
        std::string activation= "cscript slmgr.vbs /dlv | findstr \"Activation\"";
        // Execute the command and capture the output
        string status = util.executeTerminal(command); 
        if (status.length()>140){
            status=status.substr(0,139);
        }
        string key = util.executeTerminal(key_info);
        if (key.length()>140){
            key=key.substr(0,139);
        }
        string Name = util.executeTerminal(name);
        if (Name.length()>140){
            Name=Name.substr(0,139);
        }
        string Activation = util.executeTerminal(activation);
        if (Activation.length()>140){
            Activation=Activation.substr(0,139);
        } 
        parametrmap["Selected_item_wizard"]=Selected_item_wizard;
        parametrmap["Product_key"]=sanitize_utf8(status);
        parametrmap["Key_info"] =sanitize_utf8(key);
        parametrmap["Name"] = sanitize_utf8(Name);
        parametrmap["Activation_ID"] = sanitize_utf8(Activation);

        fieldmap["stock_info_id"] = stockinfoID;
        fieldmap["part_id"] = partId;
        fieldmap["part_config_id"] = partconfigid;
        fieldmap["item_id"] = item_id;
        fieldmap["created_by"] = created_by;

        // cout<<"\nupdating to db..."<<endl;
        // Update the part result with HDMI and VGA status
        return_value = partresultservice.updateBulkPartResult(parametrmap, fieldmap);
        // cout<<"\nupdateing to erp..."<<endl;
        // Sync the check result with IQC service
        iqcservice.ChecksResultSync("Produck Check");

        cout << "\nproduck key  is Completed..." << endl;
        return return_value;
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
    
};
string ProductKeyCheck::Selected_item_wizard;

