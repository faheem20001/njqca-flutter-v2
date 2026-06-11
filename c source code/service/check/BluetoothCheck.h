#include <iostream>
#include <../lib/freshweb/emit.h>
#include <string>
#include <map>
#include <bluetoothapis.h> 
#pragma comment(lib, "Bthprops.lib")
using namespace std;


class BluetoothCheck: public V2Service {

  private:
    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;
    GetFromERPService getfromERP;
    Constant constant;

  public:
    void BluetoothTest(string stockinfoID, string partId,  string item_id, string created_by) {
        nj_emit({{"event","test_start"},{"test","Bluetooth_status"},{"label","Bluetooth Test"},{"part","Bluetooth"}});
        
        int return_value;
        map<string, string> parametrmap;
        map<string, string> fieldmap;

        // Bluetooth scanning code for Windows using BluetoothFindFirstDevice and BluetoothFindNextDevice
        // BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams;
        // BLUETOOTH_DEVICE_INFO deviceInfo;
        // searchParams.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
        // searchParams.fReturnAuthenticated = TRUE;
        // searchParams.fReturnRemembered = TRUE;
        // searchParams.fReturnUnknown = TRUE;
        // searchParams.fReturnConnected = TRUE;
        // searchParams.hRadio = NULL;
        // searchParams.cTimeoutMultiplier = 6;

        // deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);

        // HBLUETOOTH_DEVICE_FIND hFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);

        // if (hFind == NULL) {
        //     cout << "Bluetooth status result: Not connected" << endl;
        //     // cout<<"\033[33m\nBluetooth is not connected. Please connect the Bluetooth and press (1) to continue or (2) Skip the Bluetooth check? Enter your choice: \033[0m" <<flush;
        //     util.generatePopupOK("Please Turn on the Bluetooth and scan then click OK to continue?");
        
        //         // cout << "Attempting to turn on Bluetooth..." << endl;
                
        //         // Command to enable Bluetooth (Windows-specific)
        //         // string bluetoothON_command = "Start-Process ms-settings:bluetooth";
        //         // util.executeTerminal(bluetoothON_command);
                
        //         // Recheck Bluetooth connection
        //         hFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
        //         if (hFind != NULL) {
        //             cout << "Bluetooth connection request completed successfully." << endl;
        //             parametrmap["Bluetooth_status"] = "Working";  // Update status to Working
        //         } else {
        //             cout << "Failed to connect to Bluetooth." << endl;
        //             parametrmap["Bluetooth_status"] = "Not Working";
        //         }
            
        // } else {
        //     bool devicesFound = false;  // Flag to track if any devices are found
        //     do {
        //         // Check if device name is valid before printing
        //         if (deviceInfo.szName[0] != '\0') {
        //             cout << "Found Bluetooth device: " << deviceInfo.szName << endl;
        //         } else {
        //             cout << "Found a Bluetooth device with no name." << endl;
        //         }

        //         // Convert the Bluetooth address to a human-readable format
        //         ULONGLONG btAddr = deviceInfo.Address.ullLong;
        //         printf("Device Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
        //             (int)(btAddr >> 40) & 0xFF,
        //             (int)(btAddr >> 32) & 0xFF,
        //             (int)(btAddr >> 24) & 0xFF,
        //             (int)(btAddr >> 16) & 0xFF,
        //             (int)(btAddr >> 8) & 0xFF,
        //             (int)(btAddr) & 0xFF);

        //         devicesFound = true;  // At least one device is found
        //         parametrmap["Bluetooth_status"] = "Working";  // Update status to Working
        //     } while (BluetoothFindNextDevice(hFind, &deviceInfo));

        //     // This block is only reached if no devices were found in the loop
        //     if (!devicesFound) {
        //         cout << "No Bluetooth devices found." << endl;
        //         parametrmap["Bluetooth_status"] = "Not Working";  // Update status if no devices found
        //     }
        // }

        // BluetoothFindDeviceClose(hFind);
        // Emit interactive_needed — Flutter shows Bluetooth test screen
        nj_emit({{"event","interactive_needed"},{"test","Bluetooth_status"},
                  {"label","Bluetooth Test"},
                  {"instruction","Enable Bluetooth, pair a device, then click Done"}});
        bool bt_passed = nj_poll_ack();
        if (bt_passed) {
            parametrmap["Bluetooth_status"] = "Working";
        } else {
            parametrmap["Bluetooth_status"] = "Not Working";
        }

        // Field map to store details
        fieldmap["stock_info_id"] = stockinfoID;
        fieldmap["part_id"] = partId;
        // fieldmap["part_config_id"] = partconfigid;
        fieldmap["item_id"] = item_id;
        fieldmap["created_by"] = created_by;

        // Part result service to update results
        return_value = partresultservice.updateBulkPartResult(parametrmap, fieldmap);
        // Sync result with IQC service
        iqcservice.ChecksResultSync("Bluetooth Check");
        nj_emit({{"event","test_done"},{"test","Bluetooth_status"},
                  {"status",parametrmap["Bluetooth_status"]=="Working" ? "pass" : "fail"},
                  {"score",parametrmap["Bluetooth_status"]=="Working" ? 100 : 0}});
    }

    bool BluetoothTestfor(){
        BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams;
        BLUETOOTH_DEVICE_INFO deviceInfo;
        searchParams.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
        searchParams.fReturnAuthenticated = TRUE;
        searchParams.fReturnRemembered = TRUE;
        searchParams.fReturnUnknown = TRUE;
        searchParams.fReturnConnected = TRUE;
        searchParams.hRadio = NULL;
        searchParams.cTimeoutMultiplier = 6;

        deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);

        HBLUETOOTH_DEVICE_FIND hFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);

        if (hFind == NULL) {
            // cout<<"\033[33m\nBluetooth is not connected. Please connect the Bluetooth and press (1) to continue or (2) Skip the Bluetooth check? Enter your choice: \033[0m" <<flush;
            nj_emit({{"event","test_progress"},{"test","Bluetooth_status"},{"msg","Bluetooth not detected. Waiting for user..."}});
        
                hFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
                if (hFind != NULL) {
                    return true;                      // Update status to Working
                } else {
                    return false;
                }
            
        } else {
            bool devicesFound = false; 
            do {
                // Check if device name is valid before printing
                if (deviceInfo.szName[0] != '\0') {
                    cout << "Found Bluetooth device: " << deviceInfo.szName << endl;
                } 

                // Convert the Bluetooth address to a human-readable format
                ULONGLONG btAddr = deviceInfo.Address.ullLong;
                printf("Device Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    (int)(btAddr >> 40) & 0xFF,
                    (int)(btAddr >> 32) & 0xFF,
                    (int)(btAddr >> 24) & 0xFF,
                    (int)(btAddr >> 16) & 0xFF,
                    (int)(btAddr >> 8) & 0xFF,
                    (int)(btAddr) & 0xFF);

                devicesFound = true;  // At least one device is found
            } while (BluetoothFindNextDevice(hFind, &deviceInfo));

            if (devicesFound) {
                return true;                      // Update status to Working
            }
        }

        BluetoothFindDeviceClose(hFind);

        return false;
    }
    public:    bool runBlutoothTest(string result){
        std::vector<std::string>blutoothCheckInfo;
        blutoothCheckInfo.push_back("Bluetooth_status");

        for(const std::string& bluetoothcheck:blutoothCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
};