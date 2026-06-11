#include <string>
#include <initializer_list>
#include <map>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

namespace config {
  std:: string TOKEN ;//= "Token 162fcec3d049a58:d63b8626d54dbd2"; //njhat@mail.com - erp.newjaisa.com
  //const string TOKEN = "Token 95b4cc8b9b8ce70:563d9e69a457afa"; //itsupport - erp.newjaisa.com
  // const string TOKEN = "Token b033104f846cf29:7786d3efb158e1d";//njqca@mail.com - /uat.biz
  // const string TOKEN = "Token 00add4397d54739:ca1f7f0daf16ac1";//njqca@mail.com - /erp.newjaisa.com
  const string SYNC_LIMIT = "2";
  const string ERP_PAGE_LENGTH = "200";
  const bool DEBUG = false;
  const bool INFO = false;
  const bool MESSAGE = true;

  /*class Config {
    public: const string TOKEN = "Token 0d7c2e9cfbc32d4:da2f6f8e5662fb9"; //Local
    public: const string SYNC_LIMIT = "2";
    public: const bool DEBUG = false;
    public: const bool INFO = true;
  };*/
}

class Constant {
  // public: string apiBaseURL = "http://10.190.4.176:8000/"; //Local
  //get ERP base URL
  public: string apiBaseURL = "https://erp.newjaisa.com/"; //production
  
  // public: string apiBaseURL = "https://amzuat.newjaisa.com/"; //UAT
  // public: string apiBaseURL="http://0.0.0.0:8000/";

  // 
  //API Methods
  public: const string API_LOGIN_URL = apiBaseURL + "api/method/login";
  public: const string API_LOGGED_URL = apiBaseURL + "api/method/frappe.auth.get_logged_user";
  public: const string API_CUSTOM_URL = apiBaseURL + "api/method/";
  public: const string API_TOKEN_URL=apiBaseURL+"api/method/nj_features.nj_features.api.login.login";
  public: const string API_DOC_URL = apiBaseURL + "api/resource/";
  public: const string API_TIME_URL=apiBaseURL+"api/method/nj_features.date_time_api.current_date_time";
  public: const string API_EVENT_DATA=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_njqca_event_triggers";
  public: const string API_selected_checks = apiBaseURL+"api/method/nj_lib.utils.common_utils.get_selected_checks";
  public: const string API_PARAMETER_URL=apiBaseURL+"api/method/nj_lib.utils.common_utils.is_parameter_exist";
  public: const string API_item_wizard=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_iw";
  public: const string API_Wizard=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_item_wizard_data";
  public: const string API_model_list=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_model_list";
  public: const string API_get_model_name=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_serial_no_model";
  public: const string API_features=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_feature_data";
  public: const string API_GET_PREVIOUS_TEST_DATA_URL =apiBaseURL+"api/method/nj_lib.utils.common_utils.get_previous_test_data";
    public: const string API_PORT_URL=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_port_number";
  public: const string API_resultsync_URL = apiBaseURL + "api/method/nj_lib.utils.common_utils.process_njqca_result";
  public: const string API_NO_OF_BDT_URL=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_battery_njqca_name_cycle_windows";
  // public: const string API_Wizard=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_item_wizard_data";
  public: const string API_update_BDT_cycle=apiBaseURL+"api/method/nj_lib.utils.common_utils.update_no_of_bdt";
  public: const string API_update_post_bdt_info=apiBaseURL+"api/method/nj_lib.utils.common_utils.update_post_battery_info";
  public: const string API_system_min_charge_percentage = apiBaseURL + "api/method/nj_lib.utils.common_utils.fetch_system_min_charge_percentage";
  public: const string API_component_item_group=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_inspection_type";
  public: const string API_role_inspection_types=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_inspection_types";
  public: const string API_adaptor_range=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_adaptor_range";
  public: const string API_part_list=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_part_list";
  public: const string score_mapping=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_score_map_Data";
  public: const string inspection_score_mapping=apiBaseURL+"api/method/nj_lib.utils.common_utils.get_score_map_Data";

  public: void setAPIBaseURL(string apiUrlLocal) {apiBaseURL = apiUrlLocal;}

  public: const string ENCRYPTION_KEY = "Kert4590";
  public: void setToken(const std::string& newToken) {
   config:: TOKEN = newToken;
  //  cout<<"\n Token used is:"<<config::TOKEN<<endl;
  }
};

class DocType {
  public: const string BRAND = "Brand";
  public: const string MODEL = "Models";
  public: const string RAW_DATA = "Raw%20Data";

  public: const string SERIAL_NO = "Serial%20No";
  public: const string IQC = "NJQCA";
  public: const string ITEM_GROUP = "Item%20Group?filters=[[\"parent_item_group\",\"=\",\"PRODUCTS\"]]";
  public: const string PURCHASE_RECEIPT = "Purchase%20Receipt";
  public: const string SETTING = "Setting";
  public: const string NJ_SETTINGS="NJ%20Settings";
};

class TableName {
  public: const string TABLE_PREFIX = "nj_";
  public: const string USER = TABLE_PREFIX + "user";
  public: const string RAW_DATA = TABLE_PREFIX + "raw_data";
  public: const string STOCK_INFO = TABLE_PREFIX + "stock_info";
  public: const string MANUFACTURER = TABLE_PREFIX + "manufacturer";
  public: const string MODEL = TABLE_PREFIX + "model";
  public: const string ITEM = TABLE_PREFIX + "item";
  public: const string ITEM_BOM = TABLE_PREFIX + "item_bom";
  public: const string PART_CONFIG = TABLE_PREFIX + "part_config";
  public: const string PART_RESULT = TABLE_PREFIX + "part_result";
  public: const string MASTER_FIELD = TABLE_PREFIX + "master_field";
  public: const string MASTER_DATA = TABLE_PREFIX + "master_data";
  public: const string SETTING = TABLE_PREFIX + "setting";
  public: const string SETTING_VALUE = TABLE_PREFIX + "setting_value";
};

class TableFieldMapping {
  public: std::map<std::string, std::map<std::string, std::string>> tableFieldMapping;
  public: std::map<std::string, std::map<std::string, std::string>> tableMapping;

  public: TableFieldMapping() {
    std::map<std::string, std::string> fieldMapping;
    std::map<std::string, std::string> tableMap;

    //brand Vs Manufacturer mapping
    fieldMapping["name"] = "name";
    fieldMapping["brand"] = "manufacturer_name";
    fieldMapping["modified"] = "modified_on";
    tableFieldMapping["Brand"] = fieldMapping;
    tableMap["nj_manufacturer"] = "";
    tableMapping["Brand"] = tableMap;

    fieldMapping.clear();
    tableMap.clear();
    fieldMapping["name"] = "name";
    fieldMapping["model_name"] = "model_name";
    fieldMapping["modified"] = "modified_on";
    tableFieldMapping["Models"] = fieldMapping;
    tableMap["nj_model"] = "";
    tableMapping["Models"] = tableMap;

    fieldMapping.clear();
    tableMap.clear();
    fieldMapping["name"] = "name";
    fieldMapping["item_group_name"] = "item_name";
    //fieldMapping["item_group_code"] = "item_code";
    //fieldMapping["item_njhat_code"] = "item_njhat_code";
    fieldMapping["modified"] = "modified_on";
    tableFieldMapping["Item Group"] = fieldMapping;
    tableMap["nj_item"] = "";
    tableMapping["Item Group"] = tableMap;

    fieldMapping.clear();
    tableMap.clear();
    fieldMapping["name"] = "name";
    fieldMapping["parent"] = "classification";
    fieldMapping["attribute_value"] = "varchar_01";
    fieldMapping["abbr"] = "varchar_02";
    fieldMapping["idx"] = "display_order";
    fieldMapping["modified"] = "modified_on";
    tableFieldMapping["get_attribute"] = fieldMapping;
    tableMap["nj_master_data"] = "njhat.api.njhat.";
    tableMapping["get_attribute"] = tableMap;
  }
};