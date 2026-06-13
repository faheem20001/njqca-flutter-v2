#include <iostream>
#include <string>

using namespace std;

class ModelModel {
  private: int modelId = 0;
  private: int manufacturerId = 0;
  private: string manufacturerName;
  private: string modelName;
  private: string status;

  public: int getModelId() {return modelId;}
  public: void setModelId(int modelIdLocal) {modelId = modelIdLocal;}

  public: string getModelName() {return modelName;}
  public: void setModelName(string modelNameLocal) {modelName = modelNameLocal;}

  public: int getManufacturerId() {return manufacturerId;}
  public: void setManufacturerId(int manufacturerIdLocal){manufacturerId = manufacturerIdLocal;}

  public: string getManufacturerName() {return manufacturerName;}
  public: void setManufacturerName(string manufacturerNameLocal) {manufacturerName = manufacturerNameLocal;}

  public: string getStatus() {return status;}
  public: void setStatus(string statusLocal) {status = statusLocal;}
};