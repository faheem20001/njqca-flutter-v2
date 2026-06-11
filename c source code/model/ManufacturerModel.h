#include <iostream>
#include <string>

using namespace std;

class ManufacturerModel {
  private: int manufacturerId = 0;
  private: string manufacturerName;
  private: string status;

  public: int getManufacturerId() {return manufacturerId;}
  public: void setManufacturerId(int manufacturerIdLocal){manufacturerId = manufacturerIdLocal;}

  public: string getManufacturerName() {return manufacturerName;}
  public: void setManufacturerName(string manufacturerNameLocal) {manufacturerName = manufacturerNameLocal;}

  public: string getStatus() {return status;}
  public: void setStatus(string statusLocal) {status = statusLocal;}
};