
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <sys/stat.h>
#include "../service/NJsettinsService.h"
using namespace std;



size_t WriteToFile(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;}

bool executeterminalCommand(const std::string& command) {
    return system(command.c_str()) == 0;}

class LanCheck : public V2Service{
    private:
       IQCService IQCservice;
       Util util;
       PartResultService partresultservice;
       NjsettingsService njset;
       static string file_location;
       
       string time;
       public:
       static bool lan_port_available;
       

    public: string getFileNameFromUrl(const std::string& url) {
        std::string fileName;
        size_t pos = url.find_last_of("/\\");
        if (pos != std::string::npos) {
            fileName = url.substr(pos + 1);
        } else {
            fileName = url; // Fallback if no slashes are found
        }
        return fileName;
    }

 public:
    bool downloadFile(const std::string url, std::string outputFileName) {
        CURL* curl;
        CURLcode res;
        FILE* fp;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        outputFileName = "Temp_Data/" + outputFileName;
        auto start = std::chrono::steady_clock::now();
        if (curl) {
            fp = fopen(outputFileName.c_str(), "wb");
            if (!fp) {
                std::cerr << "Failed to open file for writing: " << outputFileName << std::endl;
                return false;
            }

            // Set the URL
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); // Enable SSL verification
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L); // Verify hostname
            curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem"); // Path to cacert.pem

            // Write the data to the file
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToFile);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

            // Perform the request
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                fclose(fp);
                return false;
            }

            // Clean up
            fclose(fp);
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        time = to_string(elapsed_seconds.count()) + "Sec";
        return true;
    }

    public: string download(const std::string& url,  std::string& outputFileName){
       string file_transfer_time="0";
       bool result= downloadFile(url,outputFileName);
       if(result){file_transfer_time=time;
        time="";}
       return file_transfer_time;
    }

    public: std::string urlEncode(const std::string& value) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;

        for (char c : value) {
            // Keep alphanumeric and other accepted characters as is
            if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~' ||
                c == '/' || c == ':' || c == '?' || c == '&' || c == '=' || c == '+' || c == '#') {
                escaped << c;
            } else {
                // Percent-encode all other characters
                escaped << '%' << std::setw(2) << std::uppercase << static_cast<int>(static_cast<unsigned char>(c));
            }
        }
        return escaped.str();
    }


    // public: string getLANinterface(){
    //     // Get ethernet name
    //     std::string ethernet_name_cmd = "nmcli dev status | grep 'ethernet' | awk '{print $1}'";
    //     std::string ethernet_interfacename = util.executeTerminal(ethernet_name_cmd);

    //     if (ethernet_interfacename.empty()){
    //        std:: cout<< "\nNot able to find the Ethernet interface name.....";
    //     }
    //     ethernet_interfacename.erase(std::remove(ethernet_interfacename.begin(), ethernet_interfacename.end(), '\n'), ethernet_interfacename.end());
    //     return ethernet_interfacename;
    // }

    public: bool wifiON() {
        // PowerShell command to enable the Wi-Fi adapter in Windows
        string command = "powershell -Command \"Get-NetAdapter | Where-Object { $_.Name -like 'Wi*' } | ForEach-Object { Enable-NetAdapter -Name $_.Name -Confirm:$false }\"";
        if (executeterminalCommand(command)) {
            std::cout << "\nWi-Fi turned on successfully......" << std::endl;
            Sleep(6000);  // Sleep for 6 seconds (6000 milliseconds)
            return true;
        }
        return false;
    }
    public: bool  wifiOFF() {
        // PowerShell command to disconnect from the current Wi-Fi network in Windows
        string command = "powershell -Command \"Get-NetAdapter | Where-Object { $_.Name -like 'Wi*' } | ForEach-Object { Disable-NetAdapter -Name $_.Name -Confirm:$false }\"";
        if (executeterminalCommand(command)) {
            std::cout << "\nWi-Fi disconnected successfully......" << std::endl;
            Sleep(3000); 
            return true;
        }
        return false;
    }


  private:
        bool isLanCablePlugged() {
            std::string cmd = "powershell -Command \"(Get-NetAdapter | Where-Object {$_.Name -like '*Ethernet*'}).Status\"";
            std::string ifconfigOutput = util.executeTerminal(cmd);
            // std::cout << "Result of LAN status: " << ifconfigOutput << std::endl;
            if (ifconfigOutput.find("Up") != std::string::npos) {
                return true;  
            }
            return false;  
        }

    public: void update_LAN_fiile(string department,json njsetinfo){
        // json njsetinfo=njset.getNjSettingsvalues();
        for(const auto& item: njsetinfo){
         if( item["parameter"]=="file_location" && item["department"]==department){
            file_location=item["value"].get<string>();
            break;
         }
        }
    }
    
    public:
    float getFileSize(const std::string& url) {
        CURL* curl;
        CURLcode res;
        curl_off_t fileSize = 0;  // Initialize fileSize to 0

        // Initialize cURL globally
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            // Set the URL
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            // Enable SSL verification
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
            // Specify path to CA certificate
            curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");

            // Follow redirects if any
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

            // Specify that we want to fetch the headers only
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            res = curl_easy_perform(curl);

            // Check if the request was successful
            if (res == CURLE_OK) {
                // Get the content length
#if LIBCURL_VERSION_NUM >= 0x073700  // 7.55.0 and later
                res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &fileSize);
#else
                res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &fileSize);
#endif

                // Check if retrieving content length was successful
                if (res == CURLE_OK) {
                    // Convert bytes to megabytes
                    double fileSizeMB = static_cast<double>(fileSize) / 1048576;

                    std::cout << "Retrieved file size in megabytes: " << std::fixed << std::setprecision(2) << fileSizeMB << " MB" << std::endl;
                    return fileSizeMB;
                }
                else {
                    std::cerr << "Failed to get content length: " << curl_easy_strerror(res) << std::endl;
                }
            }
            else {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }

            // Clean up
            curl_easy_cleanup(curl);
        }
        else {
            std::cerr << "Failed to initialize cURL." << std::endl;
        }
        curl_global_cleanup();
        return fileSize;
    }
       public:
        int LAN_test(string stockinfoID, string partId, string partconfigid, string item_id, string created_by) {
            cout<<"\nLAN Check Running... "<<std::flush<<endl;
            map<string, string> paramtrmap;
            map<string, string> fieldidmap;
            // wifiOFF();
            int return_value = 0;
        
            if(lan_port_available){
                
                bool pluged = isLanCablePlugged(); 
                // if (!pluged) {
                // cout<<"\033[33m\nAre You Sure You inserted the LAN Cable...[y/n]?\033[0m" <<flush;
                // util.generatePopup("Are You Sure You inserted the LAN Cable?");
                //     // cout << "\nAre You Sure You inserted the LAN Cable...[y/n]?" << endl;
                //     // confirm();
                //     pluged = isLanCablePlugged();
                // }
                if(!pluged){
                    while(true){
                    bool status =util.generatePopup("LAN cable not connected", "Please connect the LAN cable then Click Retry. if LAN cable not able to connect Click Not Working?");
                    if(!status){
                        if(isLanCablePlugged()){
                        pluged=true;
                        break;
                        }
                    }else{
                        pluged=false;
                        break;
                    }
                    }
                }
                if (pluged) {
                   string filename = getFileNameFromUrl(file_location);
                    string fileUrl = urlEncode(file_location);
                    bool res = downloadFile(fileUrl, filename);
                    string file_transfer_time = time;
                    time = "";

                    if (file_transfer_time != "0") {
                        paramtrmap["LAN_File_status"] = "File Downloaded";
                        paramtrmap["LAN_Time_taken"] = file_transfer_time;
                        paramtrmap["LAN_status"] = "Working";
                        paramtrmap["LAN_Plug_status"] = "cable connected";
                        int filesize = getFileSize(fileUrl);
                        //std::cout << "get file size ====== " << getFileSize << std::endl;
                        paramtrmap["LAN_File_size"] = to_string(filesize)+" MB";
                    
                    }else{
                        paramtrmap["LAN_File_status"] = "file not Downloaded";
                        paramtrmap["LAN_status"] = "Working";
                        paramtrmap["LAN_Plug_status"] = "cable connected";
                        // paramtrmap["LAN_File_size"] = "0";
                    }

                }
                else {
                    cout << "\nLAN cable not connected...." << endl;
                    paramtrmap["LAN_Plug_status"] = "cable not connected";
                    paramtrmap["LAN_status"] = "Not Working";
                    // paramtrmap["LAN_File_status"] = "file not Downloaded";
                }
            }else{
                paramtrmap["LAN_status"] = "Not Available";
            }

            // wifiON();
            fieldidmap["stock_info_id"] = stockinfoID;
            fieldidmap["part_id"] = partId;
            fieldidmap["part_config_id"] = partconfigid;
            fieldidmap["item_id"] = item_id;
            fieldidmap["created_by"] = created_by;

            return_value = partresultservice.updateBulkPartResult(paramtrmap, fieldidmap);
            IQCservice.ChecksResultSync("LAN Check");

            cout << "\nLAN Check Completed........\n"<<std::flush;
            return return_value;
        }

    public:string byteconversion(double filesize){
        string size;
            size=to_string(filesize/1048576)+" MB";
            //  else {size=to_string(filesize/1024)+" KB";}
        return size;
    }

    private:  bool confirm() {
    string confirm;
    map<string, string> validConfirmInfo = { {"Yes", "0"}, {"yes", "1"}, {"YES", "2"}, {"y", "3"}, {"Y", "4"} };
    do {
        cout<<"\nPlease confirm:";
        cin >> confirm;
    } while (validConfirmInfo.find(confirm) == validConfirmInfo.end());
    return true;
  
  }

      public:    bool runLANTest(string result){
        std::vector<std::string>LANCheckInfo;
        LANCheckInfo.push_back("LAN_status");
        LANCheckInfo.push_back("LAN_Plug_status");

        // driverCheckInfo.push_back("Error_Drivers_Count");

        for(const std::string& bluetoothcheck:LANCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
};

string LanCheck::file_location;
bool LanCheck::lan_port_available=true;