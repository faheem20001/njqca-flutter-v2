#include <iostream>
#include <string>

using namespace std;

class StockInfoModel {
  private: int stockInfoId = 0;
  private: int rawDataId = 0;
  private: string purchaseOrderNo;
  private: string purchaseReceiptNo;
  private: string itemCode;
  private: string idx;
  private: int itemId = 0;
  private: int itemTypeId = 0;
  private: string uno;
  private: string serialNo;
  private: int manufacturerId = 0;
  private: string manufacturerName;
  private: int modelId = 0;
  private: string modelName;
  private: string modelMake;
  private: string powerOnStatus;
  private: string config;
  private: string isProcessed;
  private: string erpIqcName;
  private: string createdBy;

  public: int getStockInfoId() {return stockInfoId;}
  public: void setStockInfoId(int stockInfoIdLocal) {stockInfoId = stockInfoIdLocal;}

  public: int getRawDataId() {return rawDataId;}
  public: void setRawDataId(int rawDataIdLocal) {rawDataId = rawDataIdLocal;}

  public: string getPurchaseOrderNo() {return purchaseOrderNo;}
  public: void setPurchaseOrderNo(string purchaseOrderNoLocal) {purchaseOrderNo = purchaseOrderNoLocal;}

  public: string getPurchaseReceiptNo() {return purchaseReceiptNo;}
  public: void setPurchaseReceiptNo(string purchaseReceiptLocal) {purchaseReceiptNo = purchaseReceiptLocal;}

  public: string getItemCode() {return itemCode;}
  public: void setItemCode(string itemCodeLocal) {itemCode = itemCodeLocal;}

  public: string getIdx() {return idx;}
  public: void setIdx(string idxLocal) {idx = idxLocal;}

  public: int getItemId() {return itemId;}
  public: void setItemId(int itemIdLocal){itemId = itemIdLocal;}

   public: int getItemTypeId() {return itemTypeId;}
  public: void setItemTypeId(int itemTypeIdLocal){itemId = itemTypeIdLocal;}

   public: string getUno() {return uno;}
  public: void setUno(string unoLocal) {uno = unoLocal;}

  public: string getSerialNo() {return serialNo;}
  public: void setSerialNo(string serialNoLocal) {serialNo = serialNoLocal;}

  public: int getManufacturerId() {return manufacturerId;}
  public: void setManufacturerId(int manufacturerIdLocal){manufacturerId = manufacturerIdLocal;}

  public: string getManufacturerName() {return manufacturerName;}
  public: void setManufacturerName(string manufacturerNameLocal) {manufacturerName = manufacturerNameLocal;}

  public: int getModelId() {return modelId;}
  public: void setModelId(int modelIdLocal) {modelId = modelIdLocal;}

  public: string getModelName() {return modelName;}
  public: void setModelName(string modelNameLocal) {modelName = modelNameLocal;}

  public: string getModelMake() {return modelMake;}
  public: void setModelMake(string modelMakeLocal) {modelMake = modelMakeLocal;}

  public: string getPowerOnStatus() {return powerOnStatus;}
  public: void setPowerOnStatus(string powerOnStatusLocal) {powerOnStatus = powerOnStatusLocal;}

  public: string getConfig() {return config;}
  public: void setConfig(string configLocal) {config = configLocal;}

  public: string getIsProcessed() {return isProcessed;}
  public: void setIsProcessed(string isProcessedLocal) {isProcessed = isProcessedLocal;}

  public: string getErpIqcName() {return erpIqcName;}
  public: void setErpIqcName(string erpIqcNameLocal) {erpIqcName = erpIqcNameLocal;}

  public: string getCreatedBy() {return createdBy;}
  public: void setCreatedBy(string createdByLocal) {createdBy = createdByLocal;}

};