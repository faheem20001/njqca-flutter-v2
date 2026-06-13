#include <iostream>
#include <string>
#include <map>

using namespace std;

class StockInfoDAO: public Database {
  TableName tableName;

  public: std::map<int, map<string, string>> getStockInfoList() {
    setDefault();
    getDataList(tableName.STOCK_INFO + " as stockInfo ");
    return returnData;
  }
  
  public: int updateStockInfo(StockInfoModel stockInfoModel, std::map<std::string, string> conditionMap) {
		int returnValue = 0;
   
    std::map<std::string, string> fieldDataMap;
    if (stockInfoModel.getStockInfoId() != 0) {fieldDataMap["stock_info_id"] = to_string(stockInfoModel.getStockInfoId());}
    if (stockInfoModel.getRawDataId() != 0) {fieldDataMap["raw_data_id"] = to_string(stockInfoModel.getRawDataId());}
    if (stockInfoModel.getPurchaseOrderNo() != "") {fieldDataMap["purchase_order_no"] = stockInfoModel.getPurchaseOrderNo();}
    if (stockInfoModel.getPurchaseReceiptNo() != "") {fieldDataMap["purchase_receipt_no"] = stockInfoModel.getPurchaseReceiptNo();}
    if (stockInfoModel.getItemCode() != "") {fieldDataMap["item_code"] = stockInfoModel.getItemCode();}
    if (stockInfoModel.getIdx() != "") {fieldDataMap["idx"] = stockInfoModel.getIdx();}
    if (stockInfoModel.getItemId() != 0) {fieldDataMap["item_id"] = to_string(stockInfoModel.getItemId());}
    if (stockInfoModel.getItemTypeId() != 0) {fieldDataMap["item_type_id"] = stockInfoModel.getItemTypeId();}
    if (stockInfoModel.getUno() != "") {fieldDataMap["uno"] = stockInfoModel.getUno();}
    if (stockInfoModel.getSerialNo() != "") {fieldDataMap["serial_no"] = stockInfoModel.getSerialNo();}
    if (stockInfoModel.getErpIqcName() != "") {fieldDataMap["erp_iqc_name"] = stockInfoModel.getErpIqcName();}
    if (stockInfoModel.getManufacturerId() != 0) {fieldDataMap["manufacturer_id"] = to_string(stockInfoModel.getManufacturerId());}
    if (stockInfoModel.getModelId() != 0) {fieldDataMap["model_id"] = to_string(stockInfoModel.getModelId());}
    if (stockInfoModel.getModelMake() != "") {fieldDataMap["model_make"] = stockInfoModel.getModelMake();}
    if (stockInfoModel.getPowerOnStatus() != "") {fieldDataMap["[powder_on_status"] = stockInfoModel.getPowerOnStatus();}
    if (stockInfoModel.getConfig() != "") {fieldDataMap["config"] = stockInfoModel.getConfig();}
    if (stockInfoModel.getIsProcessed() != "") {fieldDataMap["is_processed"] = stockInfoModel.getIsProcessed();}
    if (stockInfoModel.getCreatedBy() != "") {fieldDataMap["created_by"] = stockInfoModel.getCreatedBy();}
    
    //std::cout << stockInfoModel.getRawDataId() << "--"<< fieldDataMap["raw_data_id"]; 

		returnValue = update(tableName.STOCK_INFO, fieldDataMap);
		return returnValue;
	}

  private: void setDefault() {
    if (selectFieldNameList.size() == 0 ) {
      selectFieldNameList = {
        "stockInfo.stock_info_id", "stockInfo.uno as njhat_serialno, stockInfo.item_id, serial_no as bios_serialno, stockInfo.purchase_order_no as purchase_order, stockInfo.purchase_receipt_no as purchase_receipt, stockInfo.model_make as make, stockInfo.item_code, stockInfo.created_by as created, manufacturer.name as brand, model.model_name as model, item.item_name as item_group "
      };
    }
    if (selectJoinList.size() == 0) {
      selectJoinList = {
        "left join " + tableName.MANUFACTURER + " as manufacturer on manufacturer.manufacturer_id = stockInfo.manufacturer_id " + 
        "left join " + tableName.MODEL + " as model on model.model_id = stockInfo.model_id "
        "left join " + tableName.ITEM + " as item on item.item_id = stockInfo.item_id "
      };
    }
  }
};