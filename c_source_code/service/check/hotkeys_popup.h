#define byte unsigned char

#include <comdef.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <wbemidl.h>

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
   
    
    const string CMD_brightness = "powershell -Command \"(Get-WmiObject -Namespace root/WMI -Class WmiMonitorBrightness).CurrentBrightness\"";
    const string CMD_sound_mute = "powershell -Command \"(New-Object -ComObject WScript.Shell).SendKeys([char]173)\"";
    public:
    int HotkeyTest(string stockinfoID, string partId, string item_id, string created_by) {
        cout << "\nHotkeys  checking..." << endl;
        int return_value = 0;
        // map<string, string> parametrmap;
        map<string, string> fieldmap;
        // util.generatePopupOK("Please Start the Hotkeys Test");
        // sound_mouted=IsMuted();
        // // cout<<"sound_mouted"<<sound_mouted<<endl;
        // if(sound_mouted){
        //     // util.generatePopupOK("Please Unmute the Sound and then click OK?");
        //     util.executeTerminal(CMD_sound_mute);
        // }else{
        //     // util.generatePopupOK("Please Mute the Sound and then click OK?");
        //     util.executeTerminal(CMD_sound_mute);
        // }
        // if(sound_mouted==IsMuted()){
        //     parametrmap["Sound_MuteKey_status"]="Not Working";
        // }else{
        //     parametrmap["Sound_MuteKey_status"]="Working";
        // }

        sound_mouted = IsMuted();
        util.executeTerminal(CMD_sound_mute);
        
        // Small delay for the mute toggle to take effect
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
        if (sound_mouted == IsMuted()) {
            parametrmap["Sound_MuteKey_status"] = "Not Working";
        } else {
            parametrmap["Sound_MuteKey_status"] = "Working";
        }
    
        // findSoundmuteKey();

        
        currentVolume=GetCurrentVolume();
        // cout<<"currentVolume"<<currentVolume<<endl;
        if(currentVolume==0){
            util.generatePopupOK("Please increase  Volume to greater than 0");
            currentVolume=GetCurrentVolume();
        }else{
            util.generatePopupOK("Please decrease the Volume and then click OK?");
        }
        if(currentVolume>GetCurrentVolume()){
            parametrmap["Volume_DownKey_status"]="Working";
        }else{
            parametrmap["Volume_DownKey_status"]="Not Working";
        }
        // findvolumeKeyDecrease();



        maxVolume=GetCurrentVolume();
        // cout<<"maxVolume"<<maxVolume<<endl;
        if(maxVolume==100){
            util.generatePopupOK("Please decrease the Volume to less than 100");
            maxVolume=GetCurrentVolume();
        }else{
            util.generatePopupOK("Please increase the Volume and then click OK?");
        }
        // findvolumeKeyIncrease();
        if(maxVolume<GetCurrentVolume()){
            parametrmap["Volume_UpKey_status"]="Working";
        }else{
            parametrmap["Volume_UpKey_status"]="Not Working";
        }
        microphone_mouted=IsMicrophoneMuted();
        // cout<<"microphone_mouted"<<microphone_mouted<<endl;
        if(microphone_mouted){
            util.generatePopupOK("Please Unmute the Microphone and then click OK?"); 
        }else{
            util.generatePopupOK("Please Mute the Microphone and then click OK?");
        }
        // findmicrophoneKey();
        if(microphone_mouted==IsMicrophoneMuted()){
            parametrmap["Microphone_Key_status"]="Not Working";
        }else{
            parametrmap["Microphone_Key_status"]="Working"; 
        }

        currentBrightness=getBrigtness();
        // cout<<"currentBrightness"<<currentBrightness<<endl;
        if(currentBrightness==0){
            util.generatePopupOK("Please increase the brightness to greater than 0");
        }else{
            util.generatePopupOK("Please decrease the brightness and then click OK?");
        }
        // findbrightnessKeyDecrease();
        if(currentBrightness>getBrigtness()){
            parametrmap["Brightness_DownKey_status"]="Working";
        }else{
            parametrmap["Brightness_DownKey_status"]="Not Working";
        }
        
 
        maxBrightness=getBrigtness();
        // cout<<"maxBrightness"<<maxBrightness<<endl;
        if(maxBrightness==100){
            util.generatePopupOK("Please decrease the brightness to less than 100");
        }else{
            util.generatePopupOK("Please increase the brightness and then click OK?");
        }
        // findbrightnessKeyIncrease();

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
    bool IsMuted() {
        CoInitialize(nullptr);
        IMMDeviceEnumerator* deviceEnumerator = nullptr;
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, 
                        __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
        if (FAILED(hr)) return false;
    
        IMMDevice* defaultDevice = nullptr;
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
        if (FAILED(hr)) {
            deviceEnumerator->Release();
            return false;
        }
    
        IAudioEndpointVolume* endpointVolume = nullptr;
        hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, 
                                    nullptr, (void**)&endpointVolume);
        if (FAILED(hr)) {
            defaultDevice->Release();
            deviceEnumerator->Release();
            return false;
        }
    
        BOOL isMuted = FALSE;
        endpointVolume->GetMute(&isMuted);
    
        // Clean up resources
        endpointVolume->Release();
        defaultDevice->Release();
        deviceEnumerator->Release();
        CoUninitialize();
    
        return isMuted != FALSE;  // Convert BOOL to bool
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
   
   bool IsMicrophoneMuted() {
    CoInitialize(nullptr);
    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, 
                    __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    
    IMMDevice* defaultDevice = nullptr;
    deviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &defaultDevice);
    
    IAudioEndpointVolume* endpointVolume = nullptr;
    defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, 
                          nullptr, (void**)&endpointVolume);
    
    BOOL isMuted = FALSE;
    endpointVolume->GetMute(&isMuted);
    
    endpointVolume->Release();
    defaultDevice->Release();
    deviceEnumerator->Release();
    CoUninitialize();
    
    return isMuted;
    }
    private:float GetCurrentVolume() {
    CoInitialize(nullptr);
    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, 
                    __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    
    IMMDevice* defaultDevice = nullptr;
    deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    
    IAudioEndpointVolume* endpointVolume = nullptr;
    defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, 
                          nullptr, (void**)&endpointVolume);
    
    float currentVolume = 0.0f;
    endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
    
    endpointVolume->Release();
    defaultDevice->Release();
    deviceEnumerator->Release();
    CoUninitialize();
    
    return currentVolume * 100;
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
  void findSoundmuteKey(){
    // bool runloop=true;
    auto start = std::chrono::steady_clock::now();
    
    while(true){
        if(sound_mouted!=IsMuted()){
        parametrmap["Sound_MuteKey_status"] = "Working";   
        break; 
    }
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() > 3) {
        bool popupresult;
        if(sound_mouted){
        popupresult=util.generatePopup("No change in Sound Mute key, pls UNMUTE then click yes?");
        } else{
        popupresult=util.generatePopup("No change in Sound Mute key, pls MUTE then click yes?");
        }
        if(popupresult){
        if(sound_mouted==IsMuted()){
            popupresult=util.generatePopup("Are you sure Sound Mute key working?");
            if(popupresult){
            parametrmap["Sound_MuteKey_status"] = "Working"; 
            }else{
            parametrmap["Sound_MuteKey_status"] = "Not Working";
            }
        }else{
            parametrmap["Sound_MuteKey_status"] = "Working"; 
        }
        
        }else{
        parametrmap["Sound_MuteKey_status"] = "Not Working";
        }
          
        break;
        }
    }
  }
  void findmicrophoneKey(){
    // bool runloop=true;
    auto start = std::chrono::steady_clock::now();
    
    while(true){
        if(microphone_mouted!=IsMicrophoneMuted()){
        parametrmap["microphone_Key_status"] = "Working";   
        break; 
    }
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() > 3) {
        bool popupresult;
        if(microphone_mouted){
        popupresult=util.generatePopup("No change in Microphone Mute key, pls UNMUTE then click yes?"); 
        } else{
        popupresult=util.generatePopup("No change in Microphone Mute key, pls MUTE then click yes?");
        }
        
        if(popupresult){
        if(microphone_mouted==IsMicrophoneMuted()){
            popupresult=util.generatePopup("Are you sure Microphone Mute key working?");
            if(popupresult){
            parametrmap["Microphone_Key_status"] = "Working"; 
            }else{
            parametrmap["Microphone_Key_status"] = "Not Working";
            }
        }else{
            parametrmap["Microphone_Key_status"] = "Working";
        }
        }else{
        parametrmap["Microphone_Key_status"] = "Not Working";
        }
          
        break;
    }
  }
  }

  void findvolumeKeyDecrease(){
    // bool runloop=true;
    auto start = std::chrono::steady_clock::now();
    
    while(true){
        if(GetCurrentVolume()<currentVolume){
        parametrmap["Volume_DownKey_status"] = "Working";   
        break; 
    }
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() > 3) {
        bool popupresult=util.generatePopup("No change in volume Down key,pls decrease then click yes?");
        if(popupresult){
         if(!(GetCurrentVolume()<currentVolume)){
            popupresult=util.generatePopup("Are you sure volume Down key working?");
            if(popupresult){
            parametrmap["Volume_DownKey_status"] = "Working"; 
            }else{
            parametrmap["Volume_Downkey_status"] = "Not Working";
            }
        }else{
            parametrmap["Volume_DownKey_status"] = "Working";
        }
        }else{
        parametrmap["volume_DownKey_status"] = "Not Working";
        }  
        break;
    }
  }
  }
  void findvolumeKeyIncrease(){
    // bool runloop=true;
    auto start = std::chrono::steady_clock::now();
    
    while(true){
        if(GetCurrentVolume()>maxVolume){
        parametrmap["Volume_UpKey_status"] = "Working";   
        break; 
    }
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() > 3) {
        bool popupresult=util.generatePopup("No change in volume Up key,pls increase then click yes?");
        if(popupresult){
        if(!(GetCurrentVolume()>maxVolume)){
            popupresult=util.generatePopup("Are you sure volume Up key working?");
            if(popupresult){
            parametrmap["Volume_UpKey_status"] = "Working"; 
            }else{
            parametrmap["Volume_UpKey_status"] = "Not Working";
            }
        }else{
            parametrmap["Volume_UpKey_status"] = "Working";
        }
        }else{
        parametrmap["Volume_UpKey_status"] = "Not Working";
        }
  
        break;
    }
  }
  }
  void findbrightnessKeyIncrease(){
    // bool runloop=true;
    auto start = std::chrono::steady_clock::now();
    
    while(true){
        if(getBrigtness()>maxBrightness){
        parametrmap["Brightness_UpKey_status"] = "Working";   
        break; 
    }
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() > 3) {
        bool popupresult=util.generatePopup("No change in brightness Up key,pls increase then click yes?");
        if(popupresult){
        if(!(getBrigtness()>maxBrightness)){
            popupresult=util.generatePopup("Are you sure brightness Up key working?");
            if(popupresult){
            parametrmap["Brightness_UpKey_status"] = "Working"; 
            }else{
            parametrmap["Brightness_UpKey_status"] = "Not Working";
            }
        }else{
            parametrmap["Brightness_UpKey_status"] = "Working";
        }     
        }else{
        parametrmap["Brightness_UpKey_status"] = "Not Working";
        }
  
        break;
    }
  }
  }

  void findbrightnessKeyDecrease(){
    // bool runloop=true;
    auto start = std::chrono::steady_clock::now();
    
    while(true){
        if(getBrigtness()<  currentBrightness){
        parametrmap["Brightness_DownKey_status"] = "Working";   
        break; 
    }
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() > 4) {
        bool popupresult=util.generatePopup("No change in brightness Down key,pls decrease then click yes?");
        if(popupresult){
        if(!(getBrigtness()< currentBrightness)){
            popupresult=util.generatePopup("Are you sure brightness Down key working?");
            if(popupresult){
            parametrmap["Brightness_DownKey_status"] = "Working"; 
            }else{
            parametrmap["Brightness_DownKey_status"] = "Not Working";
            }
        }else{
            parametrmap["Brightness_DownKey_status"] = "Working";
        }
        }else{
        parametrmap["Brightness_DownKey_status"] = "Not Working";
        }
        break;
    }
  } 
  }
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