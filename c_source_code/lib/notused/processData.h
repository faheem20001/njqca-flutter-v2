#include <iostream>
#include <map>
#include <nlohmann/json.hpp>

/*#include <../model/RawDataModel.h>
#include <../service/RawDataService.h>
#include <../model/StockInfoModel.h>
#include <../service/StockInfoService.h>
#include <../model/ModelModel.h>
#include <../service/ModelService.h>*/

class ProcessData {
	Database database;
	std::map<std::string, string> sysInfo;
	using json = nlohmann::json;
	RawDataModel rawDataModel;
	RawDataService rawDataService;
	StockInfoModel stockInfoModel;
	StockInfoService stockInfoService;
	ModelService modelService;

	public:
	int processData(string jsonData) {
		int returnValue = 0;

		json jsonObject = json::parse(jsonData);
		
		for (const auto& item : jsonObject.items()) {

			//Get first level product details
			if (item.key() == "vendor") {sysInfo["vendor"] = item.value();}
			if (item.key() == "product") {sysInfo["modelNo"] = item.value();}
			if (item.key() == "serial") {sysInfo["serialNo"] = item.value();}
			if (item.key() == "version") {sysInfo["version"] = item.value();}

			//if (item.key() == "children") {sysInfo["children"] = item.value();}
		/*
			json j_complete = json::parse(jsonData);
			//Get system Information
			for (const auto& item : j_complete.items()) {
				if (item.key() == "vendor") {sysInfo["vendor"] = item.value();}
				if (item.key() == "product") {sysInfo["modelNo"] = item.value();}
				if (item.key() == "serial") {sysInfo["serialNo"] = item.value();}
			}
			sysInfo["manufacturerId"] = "";
			sysInfo["modelId"] = "";
			insertStockInfo();
			//if (sysInfo["manufacturerId"] != "" && sysInfo["modelNo"] != "") {return 0;}

			for (const auto& item : j_complete.items()) {
				for (const auto& val : item.value().items()) {
					std::cout << "  --" << val.key()  << "\n"; //<< ": " << val.value()
					for (const auto& val2 : val.value().items()) {
						if (val2.key() == "id") {
							std::cout << "  --" << val2.value()  << "\n";
						}
					}
				}
			}
			std::cout << "\nor\n\n" << sysInfo["modelNo"] << sysInfo["vendor"] << sysInfo["serialNo"] << "\n";*/
		}
		//std::cout << "\n\n" + sysInfo["children"];
		//std::cout << "\n\nModel No: " << sysInfo["version"] << "\nVendor: " << sysInfo["vendor"] <<"\nSerial No: " << sysInfo["serialNo"] << "\n";
		
		

		
		//Save the raw JSON Data
		//rawDataModel.setManufacturerId();
		rawDataModel.setManufacturerName(sysInfo["vendor"]);
		//rawDataModel.setModelId();
		rawDataModel.setModelName(sysInfo["version"]);
		rawDataModel.setSerialNo(sysInfo["serialNo"]);
		rawDataModel.setJsonData(jsonData);
		returnValue = rawDataService.updateRawData(rawDataModel);
		//returnValue = updateRawData(jsonData);

		//get Manufacturer and Model Information
		std::map<std::string, string> whereMap;
		whereMap["model_name"] = sysInfo["version"];
		whereMap["manufacturer_name"] = sysInfo["vendor"];
		modelService.whereMap = whereMap;
		map<string, string> modelData = modelService.getModelData();
		
		stockInfoModel.setManufacturerId(stoi(modelData["manufacturer_id"]));
		stockInfoModel.setModelId(stoi(modelData["model_id"]));
		stockInfoModel.setSerialNo(sysInfo["serialNo"]);
		returnValue = stockInfoService.updateStockInfo(stockInfoModel);
		
		//cout << "ModelId:" << modelData["model_id"];


		//insertStockInfo();
		//std::cout << "Manufacturer Id: " << sysInfo["manufacturerId"] << "\nModel Id: " << sysInfo["modelId"] <<"\n";

		return returnValue;
	}

	/*private:
	int updateRawData(string jsonData) {
		int returnValue = 0;
		std::map<std::string, string> fieldData;
		fieldData["json_data"] = jsonData;
		returnValue = database.update("nj_raw_data", fieldData);
		return returnValue;
	}*/

	/*void insertStockInfo(){
		int returnValue = 0;
		std::map<std::string, string> whereMap;
		std::map<std::string, string> fieldData;
		whereMap["manufacturer_name"] = sysInfo["vendor"];
		database.whereMap = whereMap;
		string manufacturerId = database.getField("select manufacturer_id from nj_manufacturer");
		
		whereMap.clear();
		whereMap["model_name"] = sysInfo["version"];
		whereMap["manufacturer_id"] = manufacturerId;
		database.whereMap = whereMap;
		string modelId = database.getField("select model_id from nj_model");
		
		sysInfo["manufacturerId"] = manufacturerId;
		sysInfo["modelId"] = modelId;

		
		fieldData["serial_no"] = sysInfo["serialNo"];
		fieldData["manufacturer_id"] = sysInfo["manufacturerId"];
		fieldData["model_id"] = sysInfo["modelId"];
		returnValue = database.update("nj_stock_info", fieldData);
		//return returnValue;
	}*/
};
