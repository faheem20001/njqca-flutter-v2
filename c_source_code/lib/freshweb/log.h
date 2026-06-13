#include <iostream>
#include <string>
#include <source_location>

using namespace std;
using namespace config;

class Log {
  private: string logMessage;

	public: void debug(string message, string file, string function, int line) {
    if (config::DEBUG) {buildMessage("Debug", message, file, function, line);}
  }

  public: void info(string message, string file, string function, int line) {
    if (config::INFO) {buildMessage("Info", message, file, function, line);}
  }

  public: void message(string message, string file, string function, int line) {
    //if (config::MESSAGE) {buildMessage("Message", message, file, function, line);}
    if (config::MESSAGE) {buildProdMessage(message);}
  }

  public: void error(string message, string file, string function, int line) {
    buildMessage("Error", message, file, function, line);
  }

  private: void buildMessage(string messageType, string message, string file, string function, int line){
    logMessage = "[" + messageType + "] " + file + " :: " + function + " ::" + to_string(line) + " :: " + message + "\n";
    std::cout << "\n" << logMessage;
  }

  private: void buildProdMessage(string message){
    std::cout << "\n" << message << "\n";
  }
};