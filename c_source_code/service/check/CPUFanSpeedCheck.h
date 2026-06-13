// #include <iostream>
// #include <string>
// #include <map>
// #include <thread>

// using namespace std;

// class CPUFanSpeedCheck: public V2Service {

//     IQCService iqcservice;
//     Util util;
//     PartResultService partresultservice;
//     std::thread loopThread;
//     bool startloop= true;
//    string fan1Speed_cammand="sensors | grep \"fan1\" |awk '{print $2}'";
//    string fan2Speed_cammand="sensors | grep \"fan2\" |awk '{print $2}'";

//     public: void FanSpeedTest(string stockinfoID,string partId,string partconfigid, string item_id,string created_by){
//       int return_value;
//       cout<<"\nFAN test running...\n";
//       // loopThread=std::thread(CPUFanSpeedCheck::CPUTemUpLoop,this);
//       loopThread = std::thread([this]() { this->CPUTemUpLoop(); });
//       map<string, string>parametermap=getCPUFanSpeed();
//       if(stoi(parametermap["Fan1Speed"])>0 || stoi(parametermap["Fan2Speed"])>0){
//         parametermap["CPU_FAN Status"]="Working";
//       }
//       else{
//         parametermap["CPU_FAN Status"]="Not Working";
//       }
//       map<string, string>fieldmap;
//     // map<string, string>para
        
//     fieldmap["stock_info_id"]=stockinfoID;
//     fieldmap["part_id"]=partId;
//     fieldmap["part_config_id"]=partconfigid;
//     fieldmap["item_id"]=item_id;
//     fieldmap["created_by"]=created_by;
//     return_value= partresultservice.updateBulkPartResult(parametermap,fieldmap);
//     iqcservice.ChecksResultSync("Fan Speed Check");
//     cout<<"\nFAN Check Completed..."<<endl;
//     }

//    public:void CPUTemUpLoop(){
//     int fan1Speed=1;
//     int  fan2Speed=1;
//     int res;
//     auto start = std::chrono::steady_clock::now();
//     map<string, string>fan_speed;
//     cout<<"\nRunning loop..."<<endl;
//     while(startloop){
//       res=fan1Speed*fan2Speed/fan2Speed;

       
//         auto end = std::chrono::steady_clock::now();
//         auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
//         if (duration.count()>30){
//          startloop=false;
//         }
//     }
//     }

//     public:map<string, string> getCPUFanSpeed(){
//       map<string, string>fan_speed;
//       int fan1Speed=0;
//       int fan2Speed=0;
//       string speed1;
//       string speed2;
//       cout<<"\nChecking Fan speed..."<<endl;
//       while(startloop){
//       speed1=util.executeTerminal(fan1Speed_cammand);
//       speed2=util.executeTerminal(fan2Speed_cammand);

//       if(!speed1.empty() && fan1Speed< stoi(speed1)){
//         fan1Speed=stoi(speed1);
//       }

//       if(!speed2.empty() && fan2Speed< stoi(speed2)){
//       fan2Speed=stoi(speed2);
//       }
//       // cout<<fan1Speed<<endl;
//       // cout<<fan2Speed<<endl;
//       }
      
//       fan_speed["Fan2Speed"]=to_string(fan2Speed);
//       fan_speed["Fan1Speed"]=to_string(fan1Speed);
//     return fan_speed;
//     }
//   };
