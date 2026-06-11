#include <map>
#include <../service/ModelService.h>
#include <../service/ManufacturerService.h>
#include <../service/ItemService.h>
#include <../service/PartConfigService.h>
#include <../model/StockInfoModel.h>
#include <../dao/StockInfoDAO.h>


class StockInfoService: public V2Service  {
  StockInfoModel stockInfoModel;
  StockInfoDAO stockInfoDAO;
  PartConfigService partConfigService;
  ModelModel modelModel;
  ModelService modelService;
  ManufacturerModel manufacturerModel;
  ManufacturerService manufacturerService;
  public: static bool DO_CBC;
  public: static bool DO_Adapter;
  public: static bool DO_RAM;
  string Battery_Serial_no;

  public: int updateStockInfo(std::map<string, string> systemInfo) {
    //get Manufacturer and Model Information
    ModelService modelService;
    ItemService itemService;
		std::map<std::string, string> whereMap;
    int stockInfoId = 0;
    /*try {*/
      whereMap["model_name"] = systemInfo["version"];
      whereMap["manufacturer_name"] = systemInfo["vendor"];
      modelService.whereMap = whereMap;
      std::map<string, string> modelData = modelService.getModelData();
    
      if (modelData.empty()) {
        //Insert Manufacturer / model in the database
        modelData = insertManufacturerModel(systemInfo["vendor"], systemInfo["version"]);
      }
      
      //Get item id whether it is Laptop / desktop
      whereMap.clear();
      whereMap["#"] = "item.item_njhat_code like '%::Notebook::%'";
      itemService.whereMap = whereMap;
      std::map<string, string> itemData = itemService.getItemData();
      // for (auto item:itemData){
      //   cout<<item.first<<"=="<<item.second<<endl;
      // }
      
      if (itemData["item_id"] == "") {itemData["item_id"] = "1";}
      stockInfoModel.setItemId(stoi(itemData["item_id"]));
      stockInfoModel.setManufacturerId(stoi(modelData["manufacturer_id"]));
      stockInfoModel.setModelId(stoi(modelData["model_id"]));
      stockInfoModel.setSerialNo(systemInfo["serialNo"]);
      stockInfoModel.setUno(systemInfo["uno"]);
      // stockInfoModel.setRawDataId(stoi(systemInfo["rawDataId"]));
      stockInfoModel.setPurchaseOrderNo(systemInfo["purchaseOrderNo"]);
      stockInfoModel.setPurchaseReceiptNo(systemInfo["purchaseReceiptNo"]);
      stockInfoModel.setModelMake(systemInfo["make"]);
      stockInfoModel.setItemCode(systemInfo["itemCode"]);
      stockInfoModel.setIdx(systemInfo["idx"]);
      stockInfoModel.setCreatedBy(systemInfo["createdBy"]);

       if(DO_CBC){
        itemData["item_id"]="11";
        // cout<<"\nEnter the Battery Serial Number:";
        // cin>>Battery_Serial_no;
        // string batterys = util.executeTerminal("upower -e|grep battery");
        // std::istringstream deviceStream(batterys);
        // string battery;
        // while (std::getline(deviceStream, battery)) {
        // json batteryInfo = util.executeTerminalJSON(("upower -i " + battery).c_str());
        // Battery_Serial_no=batteryInfo["serial"];
        // }
        stockInfoModel.setItemId(stoi(itemData["item_id"]));
        // stockInfoModel.setSerialNo(Battery_Serial_no);
      }
      if(DO_Adapter){
        itemData["item_id"]="24";
        stockInfoModel.setItemId(stoi(itemData["item_id"]));
      }
      if(DO_RAM){
        itemData["item_id"]="4";
        stockInfoModel.setItemId(stoi(itemData["item_id"]));
      }
      //logInfo(to_string(stockInfoModel.getRawDataId()));
      //util.printMap(systemInfo);
      //std::cout << stockInfoModel.getModelMake();
      stockInfoId = updateStockInfo(stockInfoModel);
    /*}
    catch (...) {
      logError("Unknown error has occurred");
    }*/
    return stockInfoId; 
  }

  public: map<int, map<string, string>> getStockInfoById(std::string stockInfoId) {
    map<int, map<string, string>> stockData;
    setDefault();
    whereMap["stock_info_id"] = stockInfoId;
    stockData = getStockInfoList();
    return stockData;
  }

  //return first record from the database
  public: map<string, string> getStockInfoData() {
    std::map<int, map<string, string>> stockInfoList = getStockInfoList();
    return stockInfoList[0];
  }

  public: map<int, map<string, string>> getStockInfoList(bool childTable) {
    using json = nlohmann::json;
    std::map<int, std::map<std::string, std::string>> stockInfoList;
    setDefault();
    logDebug("stock Infoid 7:" + whereMap["stock_info_id"]);
    stockInfoList = stockInfoDAO.getStockInfoList();
    if (childTable) {
      for (int i = 0; i < stockInfoList.size(); ++i) {
        std::map<int, std::map<std::string, std::string>> partConfigList;
        partConfigList = partConfigService.getPartConfigList(stockInfoList[i]["stock_info_id"]);
        json jsonObject = json::array();
        jsonObject = partConfigList;
        //stockInfoList[i]["iqc_item_component"] = jsonObject.dump(0);
      }
    }
    return stockInfoList;
  }

  public: map<int, map<string, string>> getStockInfoList() {
    setDefault();
    return stockInfoDAO.getStockInfoList();
  }

  public: int updateStockInfo(StockInfoModel stockInfoModel) {
    std::map<std::string, string> conditionMap;
    return updateStockInfo(stockInfoModel, conditionMap);
  }

  public: int updateStockInfobyID(StockInfoModel stockInfoModel,std::map<std::string, string> whereMap) {
    stockInfoDAO.whereMap = whereMap;
    std::map<std::string, string> conditionMap;
    return updateStockInfo(stockInfoModel, conditionMap);
  }

  public: int updateStockInfo(StockInfoModel stockInfoModel, std::map<std::string, string> conditionMap) {
    return stockInfoDAO.updateStockInfo(stockInfoModel, conditionMap);
  }
   
  private: map<string, string> insertManufacturerModel(string manufacturerName, string model) {
    std::map<string, string> manufacturerData ;
    std::map<string, string> whereMapLocal;
    whereMapLocal["manufacturer_name"] = manufacturerName;
    manufacturerService.whereMap = whereMapLocal;
    manufacturerData = manufacturerService.getManufacturerData();
    if (manufacturerData.count("manufacturer_id") == 0) {
      manufacturerModel.setManufacturerName(manufacturerName);
      int manufacturerId = manufacturerService.updateManufacturer(manufacturerModel);
      manufacturerData["manufacturer_id"] = to_string(manufacturerId);
    }
    logInfo("manufacturerid:" + manufacturerData["manufacturer_id"] + ":: Name:" + manufacturerModel.getManufacturerName());
    modelModel.setManufacturerId(stoi(manufacturerData["manufacturer_id"]));
    modelModel.setModelName(model);
    int modelId = modelService.updateModel(modelModel);
    return modelService.getModelById(to_string(modelId));
  }

  private: void setDefault(){
    if (whereMap.size() != 0) {stockInfoDAO.whereMap = whereMap;}
    if (orderMap.size() != 0) {stockInfoDAO.orderMap = orderMap;}
    if (start != "") {stockInfoDAO.start = start;}
    if (length != "") {stockInfoDAO.length = length;}
  }
};

bool StockInfoService::DO_CBC=false;
bool StockInfoService::DO_Adapter=false;
bool StockInfoService::DO_RAM=false;