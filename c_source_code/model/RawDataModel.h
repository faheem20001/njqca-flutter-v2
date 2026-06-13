#include <iostream>
#include <string>

using namespace std;

class RawDataModel {
  private: int rawDataId = 0;
  private: int manufacturerId = 0;
  private: string manufacturerName;
  private: int modelId = 0;
  private: string modelName;
  private: string serialNo;
  private: string jsonData;
  private: string isProcessed = "No";

  public: int getRawDataId() {return rawDataId;}
  public: void setRawDataId(int rawDataIdLocal) {rawDataId = rawDataIdLocal;}

  public: int getManufacturerId() {return manufacturerId;}
  public: void setManufacturerId(int manufacturerIdLocal){manufacturerId = manufacturerIdLocal;}

  public: string getManufacturerName() {return manufacturerName;}
  public: void setManufacturerName(string manufacturerNameLocal) {manufacturerName = manufacturerNameLocal;}

  public: int getModelId() {return modelId;}
  public: void setModelId(int modelIdLocal) {modelId = modelIdLocal;}

  public: string getModelName() {return modelName;}
  public: void setModelName(string modelNameLocal) {modelName = modelNameLocal;}

  public: string getSerialNo() {return serialNo;}
  public: void setSerialNo(string serialNoLocal) {serialNo = serialNoLocal;}

  public: string getJsonData() {return jsonData;}
  public: void setJsonData(string jsonDataLocal) {jsonData = jsonDataLocal;}

  public: string getIsProcessed() {return isProcessed;}
  public: void setIsProcessed(string isProcessedLocal) {isProcessed = isProcessedLocal;}
};