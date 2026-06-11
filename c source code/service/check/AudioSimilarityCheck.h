// #include <iostream>
// #include <string>
// #include <map>
// #include <regex>
// #include <stdexcept> // For catching exceptions

// using namespace std;

// class AudioSimilarityCheck : public V2Service {

//     IQCService iqcService;
//     Util utility;
//     PartResultService partResultService;
//     GetFromERPService getfromERP;
//     HotkeysCheck hotkeyscheck;
//     HDDSentinelCheck hdd;

// public: 
//     void AudioSimilarityTest(string stockinfoID, string partId, string item_id, string created_by) {
//         map<string, string> parameterMap;
//         map<string, string> fieldMap;
//         bool microphoneStatus = hotkeyscheck.IsMicrophoneMuted();
//         bool speakerStatus = hotkeyscheck.IsMuted();
//         if(microphoneStatus && speakerStatus) {
//             util.generatePopupOK("Please Unmute the Microphone and Speaker Then click OK?");
//         }else if(microphoneStatus) {
//             util.generatePopupOK("Please Unmute the Microphone Then click OK?");
//         }else if(speakerStatus) {
//             util.generatePopupOK("Please Unmute the Speaker Then click OK?");
//         }
        
//         string leftfilePath = "Temp_Data/recorded_audio_left.wav";
//         string rightfilePath = "Temp_Data/recorded_audio_right.wav";

//         hdd.deletefile(leftfilePath);
//         hdd.deletefile(rightfilePath);

//         string audioCommand = "audiocheck.exe";
//         cout << "\nSpeaker Check Running..." << endl;

//         // Execute the external command and get the output
//         string audioResult = utility.executeTerminal(audioCommand);
//         std::regex dbfspattern(R"(Left speaker dBFS:\s*([-+]?\d*\.?\d+))");

//         // Regular expressions to capture similarity for left and right speakers
//         regex leftSimilarityRegex(R"(Left speaker similarity:\s*(\d+))");
//         regex rightSimilarityRegex(R"(Right speaker similarity:\s*(\d+))");

//         // // Regular expressions to check if the speakers are working
//         // regex leftSpeakerWorkingRegex(R"(Left speaker working:\s*(yes|no))");
//         // regex rightSpeakerWorkingRegex(R"(Right speaker working:\s*(yes|no))");
//         // Regular expressions to check if the speakers are working (with True/False instead of yes/no)
//         regex leftSpeakerWorkingRegex(R"(Left speaker working:\s*(True|False))");
//         regex rightSpeakerWorkingRegex(R"(Right speaker working:\s*(True|False))");

//         int dbfs;
//         smatch match;

//         int leftSimilarity = 0;
//         int rightSimilarity = 0;
//         string leftSpeakerStatus = "FAIL"; // Default value is fail
//         string rightSpeakerStatus = "FAIL"; 
//         parameterMap["Microphone_Status"]="Not Working";

//         try{
//             if (regex_search(audioResult, match, dbfspattern)) {
//               dbfs=stoi(match[1]);
//               cout<<"DBFS value:"<<dbfs<<endl;
//               if(dbfs>-40 && audioResult.find("Mic is working")!=std::string::npos){
//                 parameterMap["Microphone_Status"]="Working";
//               }
//             }else{
//                 throw runtime_error("DBFS not found in the result.");
//             }
//         }catch(const exception& e){
//             cout<<"\n Error While calculating DBFS: "<<e.what()<<endl;
//         }

//         // Extract left speaker similarity with error handling
//         try {
//             if (regex_search(audioResult, match, leftSimilarityRegex)) {
//                 leftSimilarity = stoi(match[1]); // Convert to integer
//             } else {
//                 throw runtime_error("Left speaker similarity not found in the result.");
//             }
//         } catch (const exception& e) {
//             cout << "Error extracting left speaker similarity: " << e.what() << endl;
//             leftSimilarity = -1;  // Assign a default error value
//         }

//         // Extract right speaker similarity with error handling
//         try {
//             if (regex_search(audioResult, match, rightSimilarityRegex)) {
//                 rightSimilarity = stoi(match[1]); // Convert to integer
//             } else {
//                 throw runtime_error("Right speaker similarity not found in the result.");
//             }
//         } catch (const exception& e) {
//             cout << "Error extracting right speaker similarity: " << e.what() << endl;
//             rightSimilarity = -1;  // Assign a default error value
//         }

//         // Extract left speaker working status
//         try {
//             if (regex_search(audioResult, match, leftSpeakerWorkingRegex)) {
//                 leftSpeakerStatus = match[1] == "True" ? "PASS" : "FAIL"; // If "yes", set as pass
//             } else {
//                 throw runtime_error("Left speaker working status not found in the result.");
//             }
//         } catch (const exception& e) {
//             cout << "Error extracting left speaker status: " << e.what() << endl;
//         }

//         // Extract right speaker working status
//         try {
//             if (regex_search(audioResult, match, rightSpeakerWorkingRegex)) {
//                 rightSpeakerStatus = match[1] == "True" ? "PASS" : "FAIL"; // If "yes", set as pass
//             } else {
//                 throw runtime_error("Right speaker working status not found in the result.");
//             }
//         } catch (const exception& e) {
//             cout << "Error extracting right speaker status: " << e.what() << endl;
//         }
        
//         if (std::filesystem::exists(leftfilePath)) {
//           string leftFile_updated_name= getfromERP.updateFile2ERp("Temp_Data/recorded_audio_left.wav");      
//            parameterMap["left_speaker_File_Updated_name"]=leftFile_updated_name;
//         }

//         if (std::filesystem::exists(leftfilePath)) {
//           string rightFile_updated_name= getfromERP.updateFile2ERp("Temp_Data/recorded_audio_left.wav");      
//            parameterMap["right_speaker_File_Updated_name"]=rightFile_updated_name;
//         }
//         // Store similarity and working status values
//         parameterMap["Left_speaker_audio_similarity"] = to_string(leftSimilarity);
//         parameterMap["Right_speaker_audio_similarity"] = to_string(rightSimilarity);
//         parameterMap["Left_speaker_status"] = leftSpeakerStatus;
//         parameterMap["Right_speaker_status"] = rightSpeakerStatus;

//         // Populate fieldMap with test metadata
//         fieldMap["stock_info_id"] = stockinfoID;
//         fieldMap["part_id"] = partId;
//         fieldMap["item_id"] = item_id;
//         fieldMap["created_by"] = created_by;

//         // Update part results in the system
//         partResultService.updateBulkPartResult(parameterMap, fieldMap);

//         // Sync the result with the IQC service
//         iqcService.ChecksResultSync("Speaker Check");

//         cout << "\nAudio Similarity Check Completed..." << endl;
//     }
//     public:    bool runSpeakerTest(string result){
//         std::vector<std::string>batteryCheckInfo;
//         batteryCheckInfo.push_back("Left_speaker_status");
//         // batteryCheckInfo.push_back("percentage_decreased");
//         // batteryCheckInfo.push_back("Percentage_increased");
//         // batteryCheckInfo.push_back("Adapter_status");
//         batteryCheckInfo.push_back("Right_speaker_status");
                                   
                                    
//         for(const std::string& bluetoothcheck:batteryCheckInfo){
//             if(result.find(bluetoothcheck)!=std::string::npos){
//                 return true;
//                 break;
//             }
//         }
//         return false;

//     }
// };
#include <iostream>
#include <string>
#include <filesystem>


using namespace std;

class SpeakerTest: public V2Service {

    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;
    GetFromERPService getfromERP;
 
    public: void Speaker(string stockinfoID, string partId, string item_id) {
        map<string, string> parametrmap;
        map<string, string> fieldmap;
        float left_rms=0.0;
        float right_rms=0.0;
        float left_match=0.0;
        float right_match=0.0;
        string File_updated_name;
        string cameracommand = "audio_similarity.exe";

        cout << "\nSpeaker Check Running..." << endl;
        string speakerresult = util.executeTerminal(cameracommand);
        cout<<speakerresult<<endl;
        if(speakerresult.find("FAIL") != std::string::npos) {
            parametrmap["Speaker_distortion_status"] = "FAIL";
        } else {
            parametrmap["Speaker_distortion_status"] = "PASS";
        }
        std::regex leftRMSRegex(R"(Left speaker dBFS:\s+(-?[0-9.]+))");
        std::regex rightRMSRegex(R"(Right speaker dBFS:\s+(-?[0-9.]+))");
        std::regex leftmatch(R"(left_audio_match:\s*(-?[0-9.]+))");
        std::regex rightmatch(R"(right_audio_match:\s*(-?[0-9.]+))");
        std::smatch match;

        if (std::regex_search(speakerresult, match, leftRMSRegex)) {
            left_rms = stof(match[1]); 
        } 

        if(std::regex_search(speakerresult, match, leftmatch)){
            left_match = stof(match[1]);
        }

        if (std::regex_search(speakerresult, match, rightRMSRegex)) {
            right_rms = stof(match[1]); 
        }

        if(std::regex_search(speakerresult, match, rightmatch)){
            right_match = stof(match[1]);
        }
         
        // cout<<"left_match: "<<left_match<<endl;
        // cout<<"right_match: "<<right_match<<endl;
        // if(left_rms>=0.07){
        //     parametrmap["Left_speaker_low_volume_status"] = "PASS";
        // }else{
        //     parametrmap["Left_speaker_low_volume_status"] = "FAIL";
        // }

        // if(right_rms>=0.07){
        //     parametrmap["Right_speaker_low_volume_status"] = "PASS";
        // }else{
        //     parametrmap["Right_speaker_low_volume_status"] = "FAIL";
        // }
        // printf("Left RMS:%f\n", left_rms);
        // printf("Right RMS: %f\n", right_rms);
        if(left_rms>=-35 && right_rms>=-35 && left_match>=80 && right_match>=80){
            parametrmap["speaker_volume_status"] = "PASS";
        }else{
            parametrmap["speaker_volume_status"] = "FAIL";
        }
        parametrmap["Left_speaker_dBFS"] = to_string(left_rms);
        parametrmap["Right_speaker_dBFS"] = to_string(right_rms);
        parametrmap["Left_speaker_audio_match"] = to_string(left_match)+"%";
        parametrmap["Right_speaker_audio_match"] = to_string(right_match)+"%";

        std::regex rms_regex(R"(([\w_\.]+\.wav)\s+RMS Value:\s+([\d.]+))");
        std::vector<std::string> files;
        for (auto it = std::sregex_iterator(speakerresult.begin(), speakerresult.end(), rms_regex); it != std::sregex_iterator(); ++it) {
            std::string file = (*it)[1];
            std::string value = (*it)[2];
            files.push_back(file);
            parametrmap["RMS_" + file] = value;
        }

        // Parse Clipping Ratio, Peak-to-RMS, Spectral Flatness
        std::regex metrics_regex(R"(([\w_\.]+\.wav)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+))");
        for (auto it = std::sregex_iterator(speakerresult.begin(), speakerresult.end(), metrics_regex); it != std::sregex_iterator(); ++it) {
            std::string file = (*it)[1];
            std::string clipping = (*it)[2];
            std::string peak_to_rms = (*it)[3];
            std::string flatness = (*it)[4];
            parametrmap["ClippingRatio_" + file] = clipping;
            parametrmap["PeakRMS_" + file] = peak_to_rms;
            parametrmap["SpectralFlatness_" + file] = flatness;
        }
        
        // Parse 2nd/3rd Harmonic and Distortion Verdict
        std::regex harmonic_regex(R"([\s]*([\-\d.]+)\s+([\-\d.]+)\s+(FAIL[^\\n]*|PASS))");
        auto harm_begin = std::sregex_iterator(speakerresult.begin(), speakerresult.end(), harmonic_regex);
        int idx = 0;
        for (auto it = harm_begin; it != std::sregex_iterator(); ++it, ++idx) {
            if (idx >= files.size()) break;
            std::string file = files[idx];
            std::string second_harmonic = (*it)[1];
            std::string third_harmonic = (*it)[2];
            std::string verdict = (*it)[3];
            parametrmap["2nd_Harmonic_dB_" + file] = second_harmonic;
            parametrmap["3rd_Harmonic_dB_" + file] = third_harmonic;
            parametrmap["DistortionVerdict_" + file] = verdict;
        }

      
       if(speakerresult.length()>140){
           speakerresult=speakerresult.substr(0,140);
       }
       
       string leftFile_updated_name = getfromERP.updateFile2ERp("Temp_Data/recorded_audio_left.wav");
       if(leftFile_updated_name!=""){ 
        parametrmap["left_speaker_File_Updated_name"] = leftFile_updated_name;
       }
       string rightFile_updated_name = getfromERP.updateFile2ERp("Temp_Data/recorded_audio_right.wav");
       if(rightFile_updated_name!=""){ 
       parametrmap["right_speaker_File_Updated_name"] = rightFile_updated_name;
       }
       

        // Set metadata fields
        fieldmap["stock_info_id"] = stockinfoID;
        fieldmap["part_id"] = partId;
        fieldmap["item_id"] = item_id;
       

        // Store results
        partresultservice.updateBulkPartResult(parametrmap, fieldmap);
        iqcservice.ChecksResultSync("Speaker Check");

        cout << "\nSpeaker Check Completed..." << endl;
    }



    public: bool runSpeakerTest(string result){
        std::vector<std::string>cameraCheckInfo;
        cameraCheckInfo.push_back("Speaker_distortion_status");
        cameraCheckInfo.push_back("speaker_volume_status");
        for(const std::string& bluetoothcheck:cameraCheckInfo){
            if(result.find(bluetoothcheck)!=std::string::npos){
                return true;
                break;
            }
        }
        return false;

    }
};