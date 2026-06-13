#include <iostream>
#include <map>
#include <string>
//#include <../model/GetFromERPModel.h>
//#include <freshweb/restAPI.h>

using namespace std;
using namespace config;
using nlohmann::json;

class GetFromERPService: public V2Service {
  using json = nlohmann::json;
  private:
    Constant constant;
    DocType docType;
    //TableFieldMapping tableFieldMapping;
    //Database database;

  public: 
    //string response;
    //TableName tableName;

  public: string getURL(string tableAPIURL, string docTypeName) {
    logInfo(tableAPIURL + "::" + docTypeName );
    string url;
    if (tableAPIURL != "") {url = constant.API_CUSTOM_URL + tableAPIURL;} else {url = constant.API_DOC_URL;}
    url += util.encodeURL(docTypeName);
    logInfo(url);
    return url;
  }

  //Fetching the data from ERP
  public: string api2ERP(string url, std::map<std::string, string> requestKeyPairMap) {
    RestAPI restAPI;
    RestAPIModel restAPIModel;
    map<std::string, string> header;
    header["Authorization"] = config::TOKEN;
    //header["Content-Type"] = "application/json";
    header["Accept"] = "application/json";
    header["charset"] = "utf-8"; 

    //logInfo(url);
    restAPIModel.setApiURL(url);
    restAPIModel.setRequestJSON(false);
    restAPIModel.setHeader(header);
    restAPIModel.setRequestKeyPairMap(requestKeyPairMap);
    restAPIModel.setRequestMethod("GET");
    string response;
    response=restAPI.getRequest(restAPIModel, response);
    //logInfo(response);
    // cout<<response;
    return response;
  }


  //Fetching the data from ERP
  public: string authoriseERP(string username, string password) {
    string userName = "";
    RestAPI restAPI;
    RestAPIModel restAPIModel;
    map<std::string, string> header;
    map<std::string, string> requestMap;
    json jsonObject;
    jsonObject["usr"] = username;
    jsonObject["pwd"] = password;
    header["Content-Type"] = "application/json";
    header["Accept"] = "application/json";
    restAPIModel.setApiURL(constant.API_LOGIN_URL);
    restAPIModel.setRequestJSON(true);
    restAPIModel.setHeader(header);
    restAPIModel.setRequestMethod("POST");
    restAPIModel.setPostFields(to_string(jsonObject));
    restAPIModel.setCookieFile("cookie.txt");
    string response = "";
    bool returnValue = restAPI.postRequest(restAPIModel, response);
    // cout<<"response is"<<response;
    if (returnValue) {
      map<string, string> responseMap = processERPResponse(response);
      if (responseMap["message"] == "Logged In") {
        //Login successfull, get the user details
        response = "";
        restAPIModel.setApiURL(constant.API_LOGGED_URL);
        restAPIModel.setPostFields("");
        restAPI.postRequest(restAPIModel, response);
        if (response != "") { 
          try{
          json jsonObject = json::parse(response);
          userName = jsonObject["message"];
          }
          catch (const json::parse_error& e) {
            cout<<"JSON Parse Error While login: " << e.what() << response<<endl;
            exit(0);
          }
          catch (const exception& e) {
            cerr << "Standard Exception while login: " << e.what() << response<<endl;
            exit(0);
          }
          
        }
      }
    }
    else {
      userName = "-1";
    }
    return userName;
  }
      
  public: string SetToken(string username, string password){
    string userDepartment="";
    string Token="Token ";
    RestAPI restAPI;
    RestAPIModel restAPIModel;
    map<std::string, string> header;
    map<std::string, string> requestMap;
    json jsonObject;
    jsonObject["usr"] = username;
    jsonObject["pwd"] = password;
    header["Content-Type"] = "application/json";
    header["Accept"] = "application/json";
    restAPIModel.setApiURL(constant.API_TOKEN_URL);
    restAPIModel.setRequestJSON(true);
    restAPIModel.setHeader(header);
    restAPIModel.setRequestMethod("POST");
    restAPIModel.setPostFields(to_string(jsonObject));
    restAPIModel.setCookieFile("cookie.txt");
    string response = "";
    bool returnValue = restAPI.postRequest(restAPIModel, response);
    // cout<<"response is:"<<response<<endl;
    if (returnValue) {
    try {
      json responseMap = json::parse(response);
      // cout<<responseMap.dump(2)<<endl;
      // Safely access nested keys
      string api_key = responseMap.at("message").at("api_key");
      string api_secret = responseMap.at("message").at("api_secret");
      userDepartment = responseMap.at("message").at("department");
      
      // Concatenate token and set it
      Token = Token + api_key + ":" + api_secret;
      constant.setToken(Token);
    } catch (const json::parse_error& e) {
        cerr << "JSON Parse Error while getting Token: " << e.what() << response<<endl;
        exit(0);
    } catch (const json::exception& e) {     
        cerr << "JSON Exception While getting Token: " << e.what() <<response<< endl;
        exit(0);
    } catch (const exception& e) {
       cerr << "Standard Exception While getting Token: " << e.what() << response<<endl;
       exit(0);
    }
}

    return userDepartment;
  }

  public: string updateFile2ERp(const char* filePath){
  // string url="https://uat.newjaisa.biz/api/method/upload_file";
  string url=constant.apiBaseURL+"api/method/upload_file";

   string method="post";
     
    RestAPI restAPI;
    RestAPIModel restAPIModel1;
   string response = "";
    map<std::string, string> header;
    header["Authorization"] = config::TOKEN;
    // header["Content-Type"] = "application/json";
    header["Accept"] = "application/json";
    header["charset"] = "utf-8";
    restAPIModel1.setApiURL(url);
    restAPIModel1.setRequestJSON(true);
    restAPIModel1.setHeader(header);
    restAPIModel1.setRequestMethod(method);

    
      // restAPIModel1.setPostFields(postData);
      restAPI.uploadFile(filePath, restAPIModel1,response);
      string updated_name;
      if(response!=""){
        try {
        json jsonresponse = json::parse(response);
        updated_name = jsonresponse.at("message").at("name");
        } catch (const exception& e) {
        cerr << "Error while Uploading File to ERP: " << e.what() << response<<endl;
       }
      }
      return updated_name;
      // cout<<"response"<<response<<endl;
      // logInfo(response);
    
  }

  /* Author: Baskar P
  *  Date: 02nd March 2024
  *  Input: Response string from the ERP
  *  Output: Return the data in Map. For communication error then return empty value.
  *          The error code and message will be set in the service object
  */
 
  public: map<string, string> processERPResponse(string erpResponse) {
    map<string, string> responseMap;
    try{
    json jsonObject = json::parse(erpResponse);
    
    for (const auto& item : jsonObject.items()) {
      responseMap[item.key()] = item.value();
    }
    }
    catch (const json::parse_error& e) {
      cerr << "JSON Parse Error: " << e.what() << erpResponse<<endl;
      exit(0);
    }
    catch(const exception& e) {
      cerr << "JSON Exception: " << e.what() <<erpResponse<<endl;
      exit(0);
    }
    return responseMap;
  }
};
