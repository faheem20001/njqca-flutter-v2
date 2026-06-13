#include <iostream>
#include <string>

using namespace std;

class File {

  struct loginInfo {
    string userName;
    string password;
  };

  public: bool writeBinary(std::string fileName, loginInfo &fileData) {
    bool isWrite = true;
    ofstream writeFile(fileName, ios::out | ios::binary);
    if(! writeFile) {
      cout << "Cannot open file!" << fileName << endl;
      isWrite = false;
    }
    ofstream outfile;
    outfile.write((char *) &fileData, sizeof(fileData));
    outfile.close();
    return isWrite;
  }

  /*public: struct fileData readBinary(std::string fileName) {
    return fileData;
  }*/
};