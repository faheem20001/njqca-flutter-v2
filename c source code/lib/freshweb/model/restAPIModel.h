#include <iostream>
#include <string>

using namespace std;

class RestAPIModel {
  private: string apiURL;
  private: string headers;
  private: map<std::string, string> header;
  private: string requestMethod;
  private: bool requestJSON;
  private: string postFields;
  private: map<std::string, string> requestKeyPairMap;
  private: string cookieFile;

	public: string getApiURL() {return apiURL;}
  public: void setApiURL(string apiURLLocal) {apiURL = apiURLLocal;}

	public: string getHeaders() {return headers;}
  public: void setHeaders(string headersLocal) {headers = headersLocal;}

  public: std::map<std::string,string> getHeader() {return header;}
  public: void setHeader(std::map<std::string,string> headerLocal) {header = headerLocal;}

	public: string getRequestMethod() {return requestMethod;}
  public: void setRequestMethod(string requestMethodLocal) {requestMethod = requestMethodLocal;}

  public: bool getRequestJSON() {return requestJSON;}
  public: void setRequestJSON(bool requestJSONLocal) {requestJSON = requestJSONLocal;}

  public: string getPostFields() {return postFields;}
  public: void setPostFields(string postFieldsLocal) {postFields = postFieldsLocal;}

  public: std::map<std::string,string> getRequestKeyPairMap() {return requestKeyPairMap;}
  public: void setRequestKeyPairMap(std::map<std::string,string> requestKeyPairMapLocal) {requestKeyPairMap = requestKeyPairMapLocal;}

  public: string getCookieFile() {return cookieFile;}
  public: void setCookieFile(string cookieFileLocal) {cookieFile = cookieFileLocal;}
};