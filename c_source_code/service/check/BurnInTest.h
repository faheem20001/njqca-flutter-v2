// #include <iostream>
// #include <string>
// #include <filesystem>


// using namespace std;

// class BurnInCheck: public V2Service {

//     IQCService iqcservice;
//     Util util;
//     PartResultService partresultservice;
//     GetFromERPService getfromERP;
//     public:void BurnInTest(string stockinfoID,string partId, string item_id,string created_by){
//         map<string,string> parametrmap;
//         map<string,string> fieldmap; 
//         string File_updated_name;
//         string cameracommand=".exe";
//         cout<<"\nCamera Check Running..."<<endl;
//         string cameraresult=util.executeTerminal(cameracommand);
//         std::string filePath = "Temp_Data/CameraTest.jpg";

//         std::regex variance_regex(R"(Variance:\s*([\d.]+))");
//         std::smatch match;
//         std::string variance;
//        if (std::regex_search(cameraresult, match, variance_regex)) {
//         variance = match[1];
//         // std::cout << "Extracted Variance: " << variance << std::endl;
//        }
//        if(variance!=""){
//            if (std::filesystem::exists(filePath)) {
//            File_updated_name= getfromERP.updateFile2ERp("Temp_Data/CameraTest.jpg");      
//            parametrmap["File_Updated_name"]=File_updated_name;
//         }
//        }
//         if(cameraresult.find("Blurry")!=std::string::npos){
//             parametrmap["Camera_Blurry_Status"]="FAIL";
//         }else{
//             parametrmap["Camera_Blurry_Status"]="PASS";
//         }
//         parametrmap["Camera_Variance"]=variance;
//         // cout<<cameraresult<<endl;
//         fieldmap["stock_info_id"]=stockinfoID;
//         fieldmap["part_id"]=partId;
//         // fieldmap["part_config_id"]=partconfigid;
//         fieldmap["item_id"]=item_id;
//         fieldmap["created_by"]=created_by;
//         partresultservice.updateBulkPartResult(parametrmap,fieldmap);
//         iqcservice.ChecksResultSync("Camera Check");
//         cout<<"\nCamera Check Completed..."<<endl;
//     }

//     public:    bool runCameraTest(string result){
//         std::vector<std::string>cameraCheckInfo;
//         cameraCheckInfo.push_back("Camera_Variance");
//         cameraCheckInfo.push_back("Camera_Blurry_Status");

//         for(const std::string& bluetoothcheck:cameraCheckInfo){
//             if(result.find(bluetoothcheck)!=std::string::npos){
//                 return true;
//                 break;
//             }
//         }
//         return false;

//     }
// };