
using namespace std;
#define logDebug(msg) log.debug(msg,  __FILE__, __FUNCTION__, __LINE__);
#define logInfo(msg) log.info(msg,  __FILE__, __FUNCTION__, __LINE__);
#define logMsg(msg) log.message(msg,  __FILE__, __FUNCTION__, __LINE__);
#define logError(msg) log.error(msg,  __FILE__, __FUNCTION__, __LINE__);

class V2Service {
  protected:
    Log log;
    Util util;

  public:
    std::map<std::string, string> whereMap;
    std::map<std::string, string> orderMap;
	  string start;
	  string length;
  
  private:
    std::string errorCode;
    std::string errorMessage;

  public: string getErrorCode() {return errorCode;}
  public: void setErrorCode(string errorCodeLocal) {errorCode = errorCodeLocal;}

  public: string getErrorMessage() {return errorMessage;}
  public: void setErrorMessage(string errorMessageLocal) {errorCode = errorMessageLocal;}
};