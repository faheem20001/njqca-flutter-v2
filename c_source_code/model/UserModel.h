#include <iostream>
#include <string>

using namespace std;

class UserModel {
  private: int userId = 0;
  private: string userName;
  private: string password;
  private: string status;

  public: int getUserId() {return userId;}
  public: void setUserId(int userIdLocal) {userId = userIdLocal;}

  public: string getUserName() {return userName;}
  public: void setUserName(string userNameLocal) {userName = userNameLocal;}

  public: string getPassword() {return password;}
  public: void setPassword(string passwordLocal) {password = passwordLocal;}

  public: string getStatus() {return status;}
  public: void setStatus(string statusLocal) {status = statusLocal;}
};