#include <iostream>
#include <string>


using namespace std;

class GeekBenchCheck: public V2Service {

    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;
    GetFromERPService getfromERP;
    public:void GeekBenchTest(string stockinfoID,string partId, string item_id,string created_by){
        map<string,string> parametrmap;
        map<string,string> fieldmap; 
        string File_updated_name;
        string web_scrap_command="web_scrap.exe -url ";
        // string geekbench_command="Geekbench6\\geekbench6.exe > GeekBench_Result.txt";
        cout<<"\nGeekBench Check Running..."<<endl;
        // util.executeTerminal(geekbench_command);
        char _buf[MAX_PATH];
        GetModuleFileNameA(NULL, _buf, MAX_PATH);
        string _exe_dir = string(_buf).substr(0, string(_buf).find_last_of("\\/"));
        string cmd = "\"" + _exe_dir + "\\Geekbench 5\\geekbench5.exe\"";
        SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, TRUE};
        HANDLE hR, hW;
        CreatePipe(&hR, &hW, &sa, 0);
        SetHandleInformation(hR, HANDLE_FLAG_INHERIT, 0);
        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        si.hStdOutput = hW;
        si.hStdError  = hW;
        si.dwFlags    = STARTF_USESTDHANDLES;
        PROCESS_INFORMATION pi = {};
        char cmd_buf[2048];
        strncpy(cmd_buf, cmd.c_str(), sizeof(cmd_buf));
        CreateProcessA(NULL, cmd_buf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
        CloseHandle(hW);
        string output = "";
        char gbuf[4096];
        DWORD bytes;
        while(ReadFile(hR, gbuf, sizeof(gbuf)-1, &bytes, NULL) && bytes > 0){
            gbuf[bytes] = '\0';
            output += gbuf;
            cout << gbuf;
            cout.flush();
        }
        CloseHandle(hR);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        ofstream f("GeekBench_Result.txt");
        if(f.is_open()){ f << output; f.close(); }
        std::string filePath = "GeekBench_Result.txt";
        if (std::filesystem::exists(filePath)) {
           File_updated_name= getfromERP.updateFile2ERp("GeekBench_Result.txt");      
           parametrmap["GeekBench_File_Updated_name"]=File_updated_name;
        }
        string result_url=get_url("GeekBench_Result.txt");
        cout<<"\nGeekBench URL: "<<result_url<<endl;
        if (result_url!="") {       
            parametrmap["GeekBench_URL"]=result_url;
            string get_score=util.executeTerminal(web_scrap_command+result_url);
            cout<<"\nGeekBench Score: "<<get_score<<endl;
            if(get_score!=""){
                json get_score_json=json::parse(get_score);
                parametrmap["single_core_score"]=get_score_json["single_core_score"];
                parametrmap["multi_core_score"]=get_score_json["multi_core_score"];
            }
        }        
        // cout<<cameraresult<<endl;
        fieldmap["stock_info_id"]=stockinfoID;
        fieldmap["part_id"]=partId;
        // fieldmap["part_config_id"]=partconfigid;
        fieldmap["item_id"]=item_id;
        fieldmap["created_by"]=created_by;
        partresultservice.updateBulkPartResult(parametrmap,fieldmap);
        iqcservice.ChecksResultSync("GeekBench Check");
        cout<<"\nGeekBench Check Completed..."<<endl;
    }

    public:    bool runGeekBenchTest(string result){
        std::vector<std::string>cameraCheckInfo;
        cameraCheckInfo.push_back("single_core_score");
        cameraCheckInfo.push_back("multi_core_score");

        for(const std::string& bluetoothcheck:cameraCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }

    public: string get_url(string filePath){
        string url="";
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open the file." << filePath << std::endl;
            return url;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.find("https://browser.geekbench.com/") != std::string::npos) {
                url=util.trim(line);
                break;
            }
            
        }
        file.close();
        return url;
    }
};