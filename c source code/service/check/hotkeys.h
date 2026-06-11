#define byte unsigned char

#include <comdef.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <wbemidl.h>
#include <windows.h>
#include <iostream>
using namespace std;

class HotkeysCheck : public V2Service {
    IQCService iqcservice;
    Util util; 
    PartResultService partresultservice; 
    map<string, string> parametrmap;
    bool sound_mouted ;
    bool microphone_mouted ;
    float currentVolume ;
    float maxVolume ;
    int currentBrightness;
    int maxBrightness;
    bool mute_key_driver_error=false;
    bool microphone_key_driver_error=false;
   
    
    const string CMD_brightness = "powershell -Command \"(Get-WmiObject -Namespace root/WMI -Class WmiMonitorBrightness).CurrentBrightness\"";
    const string CMD_sound_mute = "powershell -Command \"(New-Object -ComObject WScript.Shell).SendKeys([char]173)\"";
    const string CMD_volume_increase = "powershell -Command \"(New-Object -ComObject WScript.Shell).SendKeys([char]175)\"";
    const string CMD_volume_decrease = "powershell -Command \"(New-Object -ComObject WScript.Shell).SendKeys([char]174)\"";
    // const string CMD_volume_decrease = "powershell -Command \"(New-Object -ComObject WScript.Shell).SendKeys([char]174)\"";
public:
    int HotkeyTest(string stockinfoID, string partId, string item_id, string created_by) {
        cout << "\nHotkeys  checking..." << endl;
        int return_value = 0;
        // map<string, string> parametrmap;
        map<string, string> fieldmap;
        sound_mouted=IsMuted();
        if(!mute_key_driver_error){
                
            util.executeTerminal(CMD_sound_mute);
            Sleep(2000);

            if(sound_mouted==IsMuted()){
                parametrmap["Sound_MuteKey_status"]="Not Working";
            }else{
                parametrmap["Sound_MuteKey_status"]="Working";
            }
        }else{
            parametrmap["Sound_MuteKey_status"]="Not Working";
        }


        currentVolume=GetCurrentVolume();
        if(currentVolume!=-1.0f){


            if(currentVolume==0){
                // util.generatePopupOK("Please increase  Volume to greater than 0");
                util.executeTerminal(CMD_volume_increase);
                currentVolume=GetCurrentVolume();
            }
            // Sleep(5000);
            util.executeTerminal(CMD_volume_decrease);
            Sleep(2000); 
            // cout<<"currentVolume for decrease key final valume"<<GetCurrentVolume()<<endl;
            
            if(currentVolume>GetCurrentVolume()){
                parametrmap["Volume_DownKey_status"]="Working";
            }else{
                parametrmap["Volume_DownKey_status"]="Not Working";
            }
            


            maxVolume=GetCurrentVolume();
            // cout<<"maxVolume in initial volume"<<maxVolume<<endl;
            if(maxVolume==100){
                // util.generatePopupOK("Please decrease the Volume to less than 100");
                util.executeTerminal(CMD_volume_decrease);
                maxVolume=GetCurrentVolume();
            }
            util.executeTerminal(CMD_volume_increase);
            // cout<<"maxVolume in final volume"<<GetCurrentVolume()<<endl;
            
            if(maxVolume<GetCurrentVolume()){
                parametrmap["Volume_UpKey_status"]="Working";
            }else{
                parametrmap["Volume_UpKey_status"]="Not Working";
            }

        }else{
            parametrmap["Volume_DownKey_status"]="Not Working";
            parametrmap["Volume_UpKey_status"]="Not Working";
        }

        microphone_mouted=IsMicrophoneMuted();

        if(!microphone_key_driver_error){
                // util.generatePopupOK("Please Start the Hotkeys Test");
            // cout<<"microphone_mouted initial state"<<microphone_mouted<<endl;
            // Sleep(5000);
            toggleMicMuteOnce();
            Sleep(2000);
            // cout<<"microphone_mouted final state"<<IsMicrophoneMuted()<<endl;
            // if(microphone_mouted){
            //     // util.generatePopupOK("Please Unmute the Microphone and then click OK?"); 
            // }else{
            //     // util.generatePopupOK("Please Mute the Microphone and then click OK?");
            // }
            // findmicrophoneKey();
            if(microphone_mouted==IsMicrophoneMuted()){
                parametrmap["Microphone_Key_status"]="Not Working";
            }else{
                parametrmap["Microphone_Key_status"]="Working"; 
            }
        }else{
            parametrmap["Microphone_Key_status"]="Not Working";
        }
        currentBrightness=getBrigtness();

        // cout<<"currentBrightness in initial state"<<currentBrightness<<endl;
        // Sleep(5000);
       const string CMD_bright_decrease = "powershell -Command \"(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1,0)\"";
       const string CMD_bright_standard = "powershell -Command \"(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1,50)\"";
       const string CMD_bright_inc = "powershell -Command \"(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1,100)\"";
        if(currentBrightness==0){
           
            util.executeTerminal(CMD_bright_inc);
            currentBrightness=getBrigtness();

        }
        util.executeTerminal(CMD_bright_decrease);
        Sleep(2000);
        // cout<<"currentBrightness in final state"<<getBrigtness()<<endl;

        if(currentBrightness>getBrigtness()){
            parametrmap["Brightness_DownKey_status"]="Working";
        }else{
            parametrmap["Brightness_DownKey_status"]="Not Working";
        }
        
 
        maxBrightness=getBrigtness();
        // cout<<"maxBrightness in intial state"<<maxBrightness<<endl;
        // Sleep(5000);
        if(maxBrightness==100){
            util.executeTerminal(CMD_bright_standard);
            currentBrightness=getBrigtness();
            // util.generatePopupOK("Please decrease the brightness to less than 100");
        }
        // findbrightnessKeyIncrease();
        util.executeTerminal(CMD_bright_inc);
        Sleep(2000);
        // cout<<"maxBrightness in final state"<<getBrigtness()<<endl;
       if (maxBrightness<getBrigtness()){
            parametrmap["Brightness_UpKey_status"]="Working";
        }else{
            parametrmap["Brightness_UpKey_status"]="Not Working";
        }

        // util.executeTerminal(command);
        


        fieldmap["stock_info_id"] = stockinfoID;
        fieldmap["part_id"] = partId;
        // fieldmap["part_config_id"] = partconfigid;
        fieldmap["item_id"] = item_id;
        fieldmap["created_by"] = created_by;

        // Update the part result with HDMI and VGA status
        return_value = partresultservice.updateBulkPartResult(parametrmap, fieldmap);
      
      
        iqcservice.ChecksResultSync("Hotkeys Check");

        cout << "\nHotkeys check is Completed..." << endl;
        return return_value;
    }

//     bool IsMuted() {
//     CoInitialize(nullptr);
//     IMMDeviceEnumerator* deviceEnumerator = nullptr;
//     CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, 
//                     __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    
//     IMMDevice* defaultDevice = nullptr;
//     deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    
//     IAudioEndpointVolume* endpointVolume = nullptr;
//     defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, 
//                           nullptr, (void**)&endpointVolume);
    
//     BOOL isMuted = FALSE;
//     endpointVolume->GetMute(&isMuted);
    
//     endpointVolume->Release();
//     defaultDevice->Release();
//     deviceEnumerator->Release();
//     CoUninitialize();
    
//     return isMuted;
//    }
// bool IsMuted() {
//     CoInitialize(nullptr);
//     IMMDeviceEnumerator* deviceEnumerator = nullptr;
//     HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, 
//                     __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
//     if (FAILED(hr)) return false;

//     IMMDevice* defaultDevice = nullptr;
//     hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
//     if (FAILED(hr)) {
//         deviceEnumerator->Release();
//         return false;
//     }

//     IAudioEndpointVolume* endpointVolume = nullptr;
//     hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, 
//                                 nullptr, (void**)&endpointVolume);
//     if (FAILED(hr)) {
//         defaultDevice->Release();
//         deviceEnumerator->Release();
//         return false;
//     }

//     BOOL isMuted = FALSE;
//     endpointVolume->GetMute(&isMuted);

//     // Clean up resources
//     endpointVolume->Release();
//     defaultDevice->Release();
//     deviceEnumerator->Release();
//     CoUninitialize();

//     return isMuted != FALSE;  // Convert BOOL to bool
// }
bool IsMuted() {
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        std::cerr << "CoInitialize failed: " << std::hex << hr << std::endl;
        mute_key_driver_error=true;
        return false;
    }

    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
                          __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    if (FAILED(hr) || !deviceEnumerator) {
        std::cerr << "Failed to create IMMDeviceEnumerator: " << std::hex << hr << std::endl;
        CoUninitialize();
        mute_key_driver_error=true;
        return false;
    }

    IMMDevice* defaultDevice = nullptr;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    if (FAILED(hr) || !defaultDevice) {
        std::cerr << "Failed to get default audio endpoint: " << std::hex << hr << std::endl;
        deviceEnumerator->Release();
        CoUninitialize();
        mute_key_driver_error=true;
        return false;
    }

    IAudioEndpointVolume* endpointVolume = nullptr;
    hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER,
                                 nullptr, (void**)&endpointVolume);
    if (FAILED(hr) || !endpointVolume) {
        std::cerr << "Failed to activate endpoint volume: " << std::hex << hr << std::endl;
        defaultDevice->Release();
        deviceEnumerator->Release();
        CoUninitialize();
        mute_key_driver_error=true;
        return false;
    }

    BOOL isMuted = FALSE;
    hr = endpointVolume->GetMute(&isMuted);
    if (FAILED(hr)) {
        std::cerr << "Failed to get mute status: " << std::hex << hr << std::endl;
        isMuted = FALSE; 
    }
    endpointVolume->Release();
    defaultDevice->Release();
    deviceEnumerator->Release();
    CoUninitialize();

    return isMuted != FALSE;
}
   
//    bool IsMicrophoneMuted() {
//     CoInitialize(nullptr);
//     IMMDeviceEnumerator* deviceEnumerator = nullptr;
//     CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, 
//                     __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    
//     IMMDevice* defaultDevice = nullptr;
//     deviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &defaultDevice);
    
//     IAudioEndpointVolume* endpointVolume = nullptr;
//     defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, 
//                           nullptr, (void**)&endpointVolume);
    
//     BOOL isMuted = FALSE;
//     endpointVolume->GetMute(&isMuted);
    
//     endpointVolume->Release();
//     defaultDevice->Release();
//     deviceEnumerator->Release();
//     CoUninitialize();
    
//     return isMuted;
//     }
bool IsMicrophoneMuted() {
    try {
        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr)) throw std::runtime_error("CoInitialize failed");

        IMMDeviceEnumerator* deviceEnumerator = nullptr;
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
                              __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
        if (FAILED(hr)) throw std::runtime_error("CoCreateInstance failed");

        IMMDevice* defaultDevice = nullptr;
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &defaultDevice);
        if (FAILED(hr)) throw std::runtime_error("No default capture device");

        IAudioEndpointVolume* endpointVolume = nullptr;
        hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER,
                                     nullptr, (void**)&endpointVolume);
        if (FAILED(hr)) throw std::runtime_error("Failed to activate volume interface");

        BOOL isMuted = FALSE;
        hr = endpointVolume->GetMute(&isMuted);
        if (FAILED(hr)) throw std::runtime_error("Failed to get mute status");

        // Cleanup
        endpointVolume->Release();
        defaultDevice->Release();
        deviceEnumerator->Release();
        CoUninitialize();

        return isMuted;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        mute_key_driver_error=true;
        return false;
    }
}


    void toggleMicMuteOnce() {
        HRESULT hr;
        CoInitialize(NULL);  // Initialize COM
    
        IMMDeviceEnumerator* pEnumerator = NULL;
        IMMDevice* pDevice = NULL;
        IAudioEndpointVolume* pEndpointVolume = NULL;
    
        // Create device enumerator
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                              __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    
        if (FAILED(hr)) {
            std::cerr << "Failed to create device enumerator.\n";
            return;
        }
    
        // Get default microphone (capture device)
        hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pDevice);
        if (FAILED(hr)) {
            std::cerr << "Failed to get default audio endpoint.\n";
            pEnumerator->Release();
            return;
        }
    
        // Activate volume control interface
        hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pEndpointVolume);
        if (FAILED(hr)) {
            std::cerr << "Failed to activate audio endpoint volume.\n";
            pDevice->Release();
            pEnumerator->Release();
            return;
        }
    
        // Get current mute state
        BOOL bMute;
        pEndpointVolume->GetMute(&bMute);
    
        // Toggle mute state
        pEndpointVolume->SetMute(!bMute, NULL);
    
        std::cout << (bMute ? "Microphone unmuted.\n" : "Microphone muted.\n");
    
        // Cleanup
        pEndpointVolume->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
    }

//     private:float GetCurrentVolume() {
//     CoInitialize(nullptr);
//     IMMDeviceEnumerator* deviceEnumerator = nullptr;
//     CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, 
//                     __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    
//     IMMDevice* defaultDevice = nullptr;
//     deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    
//     IAudioEndpointVolume* endpointVolume = nullptr;
//     defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, 
//                           nullptr, (void**)&endpointVolume);
    
//     float currentVolume = 0.0f;
//     endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
    
//     endpointVolume->Release();
//     defaultDevice->Release();
//     deviceEnumerator->Release();
//     CoUninitialize();
    
//     return currentVolume * 100;
// }

private: float GetCurrentVolume() {
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        std::cerr << "CoInitialize failed: " << std::hex << hr << std::endl;
        return -1.0f;
    }

    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
                          __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    if (FAILED(hr) || !deviceEnumerator) {
        std::cerr << "Failed to create device enumerator: " << std::hex << hr << std::endl;
        CoUninitialize();
        return -1.0f;
    }

    IMMDevice* defaultDevice = nullptr;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    if (FAILED(hr) || !defaultDevice) {
        std::cerr << "Failed to get default endpoint: " << std::hex << hr << std::endl;
        deviceEnumerator->Release();
        CoUninitialize();
        return -1.0f;
    }

    IAudioEndpointVolume* endpointVolume = nullptr;
    hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER,
                                 nullptr, (void**)&endpointVolume);
    if (FAILED(hr) || !endpointVolume) {
        std::cerr << "Failed to activate endpoint volume: " << std::hex << hr << std::endl;
        defaultDevice->Release();
        deviceEnumerator->Release();
        CoUninitialize();
        return -1.0f;
    }

    float currentVolume = -1.0f;  // Default to -1 on failure
    hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
    if (FAILED(hr)) {
        std::cerr << "Failed to get master volume: " << std::hex << hr << std::endl;
        currentVolume = -1.0f;
    } else {
        currentVolume *= 100.0f; // Convert to percentage
    }

    // Clean up
    endpointVolume->Release();
    defaultDevice->Release();
    deviceEnumerator->Release();
    CoUninitialize();

    return currentVolume;
}
private:int getBrigtness(){
    int brightness = 0;
    string brightness_result=util.executeTerminal(CMD_brightness);
    try{
    brightness = stoi(brightness_result.c_str());}
    catch(exception & e)
    {
        cout<<"Unable to get the brightness: "<<e.what()<<endl;
    }
    return brightness;

}
//   void findSoundmuteKey(){
//     // bool runloop=true;
//     auto start = std::chrono::steady_clock::now();
    
//     while(true){
//         if(sound_mouted!=IsMuted()){
//         parametrmap["Sound_MuteKey_status"] = "Working";   
//         break; 
//     }
//     auto end = std::chrono::steady_clock::now();
//     auto diff = end - start;
//     if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() > 3) {
//         bool popupresult;
//         if(sound_mouted){
//         popupresult=util.generatePopup("No change in Sound Mute key, pls UNMUTE then click yes?");
//         } else{
//         popupresult=util.generatePopup("No change in Sound Mute key, pls MUTE then click yes?");
//         }
//         if(popupresult){
//         if(sound_mouted==IsMuted()){
//             popupresult=util.generatePopup("Are you sure Sound Mute key working?");
//             if(popupresult){
//             parametrmap["Sound_MuteKey_status"] = "Working"; 
//             }else{
//             parametrmap["Sound_MuteKey_status"] = "Not Working";
//             }
//         }else{
//             parametrmap["Sound_MuteKey_status"] = "Working"; 
//         }
        
//         }else{
//         parametrmap["Sound_MuteKey_status"] = "Not Working";
//         }
          
//         break;
//         }
//     }
//   }
//   void findmicrophoneKey(){
//     // bool runloop=true;
//     auto start = std::chrono::steady_clock::now();
    
//     while(true){
//         if(microphone_mouted!=IsMicrophoneMuted()){
//         parametrmap["microphone_Key_status"] = "Working";   
//         break; 
//     }
//     auto end = std::chrono::steady_clock::now();
//     auto diff = end - start;
//     if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() > 3) {
//         bool popupresult;
//         if(microphone_mouted){
//         popupresult=util.generatePopup("No change in Microphone Mute key, pls UNMUTE then click yes?"); 
//         } else{
//         popupresult=util.generatePopup("No change in Microphone Mute key, pls MUTE then click yes?");
//         }
        
//         if(popupresult){
//         if(microphone_mouted==IsMicrophoneMuted()){
//             popupresult=util.generatePopup("Are you sure Microphone Mute key working?");
//             if(popupresult){
//             parametrmap["Microphone_Key_status"] = "Working"; 
//             }else{
//             parametrmap["Microphone_Key_status"] = "Not Working";
//             }
//         }else{
//             parametrmap["Microphone_Key_status"] = "Working";
//         }
//         }else{
//         parametrmap["Microphone_Key_status"] = "Not Working";
//         }
          
//         break;
//     }
//   }
//   }

//   void findvolumeKeyDecrease(){
//     // bool runloop=true;
//     auto start = std::chrono::steady_clock::now();
    
//     while(true){
//         if(GetCurrentVolume()<currentVolume){
//         parametrmap["Volume_DownKey_status"] = "Working";   
//         break; 
//     }
//     auto end = std::chrono::steady_clock::now();
//     auto diff = end - start;
//     if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() > 3) {
//         bool popupresult=util.generatePopup("No change in volume Down key,pls decrease then click yes?");
//         if(popupresult){
//          if(!(GetCurrentVolume()<currentVolume)){
//             popupresult=util.generatePopup("Are you sure volume Down key working?");
//             if(popupresult){
//             parametrmap["Volume_DownKey_status"] = "Working"; 
//             }else{
//             parametrmap["Volume_Downkey_status"] = "Not Working";
//             }
//         }else{
//             parametrmap["Volume_DownKey_status"] = "Working";
//         }
//         }else{
//         parametrmap["volume_DownKey_status"] = "Not Working";
//         }  
//         break;
//     }
//   }
//   }
//   void findvolumeKeyIncrease(){
//     // bool runloop=true;
//     auto start = std::chrono::steady_clock::now();
    
//     while(true){
//         if(GetCurrentVolume()>maxVolume){
//         parametrmap["Volume_UpKey_status"] = "Working";   
//         break; 
//     }
//     auto end = std::chrono::steady_clock::now();
//     auto diff = end - start;
//     if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() > 3) {
//         bool popupresult=util.generatePopup("No change in volume Up key,pls increase then click yes?");
//         if(popupresult){
//         if(!(GetCurrentVolume()>maxVolume)){
//             popupresult=util.generatePopup("Are you sure volume Up key working?");
//             if(popupresult){
//             parametrmap["Volume_UpKey_status"] = "Working"; 
//             }else{
//             parametrmap["Volume_UpKey_status"] = "Not Working";
//             }
//         }else{
//             parametrmap["Volume_UpKey_status"] = "Working";
//         }
//         }else{
//         parametrmap["Volume_UpKey_status"] = "Not Working";
//         }
  
//         break;
//     }
//   }
//   }
//   void findbrightnessKeyIncrease(){
//     // bool runloop=true;
//     auto start = std::chrono::steady_clock::now();
    
//     while(true){
//         if(getBrigtness()>maxBrightness){
//         parametrmap["Brightness_UpKey_status"] = "Working";   
//         break; 
//     }
//     auto end = std::chrono::steady_clock::now();
//     auto diff = end - start;
//     if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() > 3) {
//         bool popupresult=util.generatePopup("No change in brightness Up key,pls increase then click yes?");
//         if(popupresult){
//         if(!(getBrigtness()>maxBrightness)){
//             popupresult=util.generatePopup("Are you sure brightness Up key working?");
//             if(popupresult){
//             parametrmap["Brightness_UpKey_status"] = "Working"; 
//             }else{
//             parametrmap["Brightness_UpKey_status"] = "Not Working";
//             }
//         }else{
//             parametrmap["Brightness_UpKey_status"] = "Working";
//         }     
//         }else{
//         parametrmap["Brightness_UpKey_status"] = "Not Working";
//         }
  
//         break;
//     }
//   }
//   }

//   void findbrightnessKeyDecrease(){
//     // bool runloop=true;
//     auto start = std::chrono::steady_clock::now();
    
//     while(true){
//         if(getBrigtness()<  currentBrightness){
//         parametrmap["Brightness_DownKey_status"] = "Working";   
//         break; 
//     }
//     auto end = std::chrono::steady_clock::now();
//     auto diff = end - start;
//     if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() > 4) {
//         bool popupresult=util.generatePopup("No change in brightness Down key,pls decrease then click yes?");
//         if(popupresult){
//         if(!(getBrigtness()< currentBrightness)){
//             popupresult=util.generatePopup("Are you sure brightness Down key working?");
//             if(popupresult){
//             parametrmap["Brightness_DownKey_status"] = "Working"; 
//             }else{
//             parametrmap["Brightness_DownKey_status"] = "Not Working";
//             }
//         }else{
//             parametrmap["Brightness_DownKey_status"] = "Working";
//         }
//         }else{
//         parametrmap["Brightness_DownKey_status"] = "Not Working";
//         }
//         break;
//     }
//   } 
//   }
    public:    bool runHotkeyTest(string result){
        std::vector<std::string>hotkeyCheckInfo;
        hotkeyCheckInfo.push_back("Microphone_Key_status");
        hotkeyCheckInfo.push_back("Brightness_UpKey_status");
        hotkeyCheckInfo.push_back("Volume_UpKey_status");
        hotkeyCheckInfo.push_back("Volume_DownKey_status");
        hotkeyCheckInfo.push_back("Sound_MuteKey_status");
        hotkeyCheckInfo.push_back("Brightness_DownKey_status");
        
                                       
        // driverCheckInfo.push_back("Error_Drivers_Count");

        for(const std::string& bluetoothcheck:hotkeyCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
};