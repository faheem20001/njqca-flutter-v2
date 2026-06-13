#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include <algorithm> // For std::remove, std::isspace
#include <cctype>    // For std::isspace
#include <filesystem> // For path checking
using namespace std;
class Password: public V2Service {
    Util util;
public:
    map<string,string> readPasswordFromUSB() {
        map<string, string> passwordmap;
        json deviceIDs = getDeviceIDs();
        // cout<<"device IDs:"<<deviceIDs.dump(2)<<endl;
        string username, password;

        if (deviceIDs.empty()) {
            std::cout << "No removable drives detected."<< std::endl;
            return passwordmap;
        }

        for (const auto& deviceID : deviceIDs) {
            // std::cout << "Checking device: " << deviceID << std::endl;

            // cout<<"device ID:"<<deviceID["DeviceID"]<<endl;
            string device=deviceID["DeviceID"].get<string>();
            std::string devicePath =  device+ "\\NJQCApassword.txt";  
            // std::cout << "Looking for file at: " << devicePath << std::endl;

            // Check if the file exists using std::filesystem
            if (std::filesystem::exists(devicePath)) {
                // std::cout << "File exists: " << devicePath << std::endl;

                // Try opening the file
                std::ifstream file(devicePath);
                if (file.is_open()) {
                    std::getline(file, username);
                    std::getline(file, password);
                    file.close();

                    if (!username.empty() && !password.empty()) {
                        std::cout << "Password file read successfully from " << deviceID << std::endl;
                        // return true; 
                    } else {
                        std::cout << "Password file found but contains invalid or empty data." << std::endl;
                    }
                } else {
                    std::cout << "Could not open the file: " << devicePath << std::endl;
                }
            } 
        }
        passwordmap["username"] = username;
        passwordmap["password"] = password;
        return passwordmap;
        // std::cout << "No valid password file found on any USB drives." << std::endl;
        // return false;
    }

private:
    json getDeviceIDs() {
        // std::vector<std::string> deviceIDs;

        // FILE* pipe = _popen("wmic logicaldisk get DeviceID", "r");
        // if (!pipe) {
        //     std::cerr << "Failed to execute WMIC command." << std::endl;
        //     return deviceIDs;
        // }

        // char buffer[128];
        // bool skipHeader = true;

        // while (fgets(buffer, sizeof(buffer), pipe)) {
        //     std::string line(buffer);

        //     // Remove newline and carriage return characters
        //     line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        //     line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        //     if (line.empty() || skipHeader) {
        //         skipHeader = false; // Skip the first line (header)
        //         continue;
        //     }

        //     // Remove spaces from the DeviceID to prevent extra spaces in the file path
        //     line.erase(std::remove(line.begin(), line.end(), ' '), line.end());

        //     // if (line.back() == ':') {
        //     //     line.pop_back(); 
        //     // }

        //     // std::cout << "Device ID: " << line << std::endl;
        //     deviceIDs.push_back(line);
        // }
        // _pclose(pipe);
        const string disk_command= "powershell -Command \"Get-WmiObject -Class Win32_logicaldisk | Format-List DeviceID\"";
        string result=util.executeTerminal(disk_command);
        result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
        std::istringstream inputstream(result);
        // cout<<"\nCPU Info"<<"Started.."<<CMD_CPU<<"\n";
         json jsonData= util.ExcetuteTerminalJSonArray1(inputstream);
        //  cout<<jsonData.dump(2)<<endl;
        

        return jsonData;
    }
};


