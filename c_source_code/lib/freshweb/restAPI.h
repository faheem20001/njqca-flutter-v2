#include <iostream>
#include <string>
#include <curl/curl.h>
#include <map>

using namespace std;
//using namespace config;

// Callback function to handle curl's response
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  ((string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

class RestAPI {
    private: CURL *curl = curl_easy_init();
    //private: Config config;

    public: RestAPI() {
      bool returnValue = true;
      if (!curl) {
          cerr << "Failed to initialize CURL" << endl;
          returnValue = false;
      }
      curl_global_init(CURL_GLOBAL_DEFAULT);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.38.0");
      curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
      //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    };

    /**
     * Post Method - Tested with JSON request format and returns JSON format data
    */
       bool postRequest(RestAPIModel restAPIModel, string& response) {
        bool returnValue = true;
        setHeader(restAPIModel.getHeader());
        // cout << "API URL: " << restAPIModel.getApiURL() << endl;
        curl_easy_setopt(curl, CURLOPT_URL, restAPIModel.getApiURL().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        string postFields = restAPIModel.getPostFields();
        // cout << "Post fields: " << postFields << endl;

        if (restAPIModel.getCookieFile() != "") {
            curl_easy_setopt(curl, CURLOPT_COOKIEJAR, restAPIModel.getCookieFile().c_str());
        }

        if (restAPIModel.getRequestMethod() == "PUT") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        }
        else {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        }

        setSSL();
        returnValue = curlExecute();
        // cout << "Response:===================== loooded in \n" << response << endl;
  
        return returnValue;
    }

    public: string getRequest(RestAPIModel restAPIModel, string& response) {
      bool returnValue = true;
      setHeader(restAPIModel.getHeader());
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, restAPIModel.getRequestMethod());
      string url = restAPIModel.getApiURL() + setGET(restAPIModel);

      // std::cout<< url << "::" << restAPIModel.getRequestMethod() << "\n";

      curl_easy_setopt(curl, CURLOPT_URL,url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
      setSSL();
      returnValue = curlExecute();
      //std::cout << "Check...." << response;
      return response;
    }

    private:
    void setSSL() {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); // Enable peer verification
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L); // Enable host verification
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem"); // Path to cacert.pem in Windows
        // curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookies.txt"); // Store cookies in a file
        // curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "cookies.txt"); // Use the same cookies for subsequent requests

    }

    private: void setHeader(map<std::string, string> header) {
      struct curl_slist *slist1;
      slist1 = NULL;
      for (auto const& [key, value] : header) {
        slist1 = curl_slist_append(slist1, (key + ":" + value).c_str());
      }
      header["Authorization"] = config::TOKEN;
      header["Content-Type"] = "application/json"; 

      if (slist1 != NULL) {curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);}
    }

    private: //Not working need to be checked
    void setPostFields(string postFields) {
      //if (restAPIModel.getPostFields() != "") {
          //string postFields = restAPIModel.getPostFields();
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields);
      //}
    }

    private: void setPOST() {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    }

    private: string setGET(RestAPIModel restAPIModel) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
      //set key pair
      string keyPairValue = "";
      if (restAPIModel.getRequestKeyPairMap().size() != 0) {
        for (auto const& [key, value] : restAPIModel.getRequestKeyPairMap()) {
          if (keyPairValue != "") {keyPairValue += "&";}
          keyPairValue += key + "=" + value;
        }
        keyPairValue = "?" + keyPairValue;
      }
      return keyPairValue;
    }

    private: bool curlExecute() {
      bool returnValue = true;
      //std::cout << "Curl: " << curl;
      CURLcode res = curl_easy_perform(curl);
      //curl_easy_cleanup(curl);
      //std::cout << "perform" << CURLE_OK << "\n";
      if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        returnValue = false;
      }
      return returnValue;
    }

    public: void uploadFile(const char* file_path,RestAPIModel restAPIModel, string& response){
        curl_mime* form = curl_mime_init(curl);
        curl_mimepart* field = nullptr;
        field = curl_mime_addpart(form);
        curl_mime_name(field, "file");
        curl_mime_filedata(field, file_path);

        field = curl_mime_addpart(form);
        curl_mime_name(field, "is_private");
        curl_mime_data(field, "0", CURL_ZERO_TERMINATED);

        setHeader(restAPIModel.getHeader());
        curl_easy_setopt(curl, CURLOPT_URL, restAPIModel.getApiURL().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        // curl_easy_setopt(curl, CURLOPT_URL, url);
       
        // Attach the form to the request
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
        setSSL();
        curlExecute();
        curl_mime_free(form);
    }

};