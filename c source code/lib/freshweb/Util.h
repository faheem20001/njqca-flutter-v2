#include <iostream>
#include <string>
#include <regex>
#include <Windows.h>

using namespace std;

class Util
{
public:
  string json2String(json jsonValue){
    std::string returnString = "";
    if (jsonValue.is_number()) {returnString = jsonValue.dump(0);}
    if (jsonValue.is_string()) {returnString = jsonValue;}
    // if (jsonValue.is_object() || jsonValue.is_array()) {returnString = jsonValue.dump(0);}
    return returnString;
  }

public:
  std::map<int, std::string> splitString(string stringData, string delimiter) {
    map<int, std::string> splitStringMap;
    size_t pos = 0;
    std::string token;
    int count = 0;
    while ((pos = stringData.find(delimiter)) != std::string::npos) {
      token = stringData.substr(0, pos);
      splitStringMap[count] = token;
      // std::cout << "count::" << count << token << splitStringMap[splitStringMap.size()];
      stringData.erase(0, pos + delimiter.length());
      count++;
    }
    splitStringMap[count] = stringData;
    return splitStringMap;
  }

public:json executeTerminal(string command, bool isJsonObject) {
  string sysResultJSON = executeTerminal(command);
  json jsonObject = json::parse(sysResultJSON);
  return jsonObject;
}

public: json executeTerminalJSON(string command, string splitchar="") {
  char buffer[128];
    string result = "";
    json jsonData;

    // Open pipe to file
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
      return "popen failed!";
    }

    // read till end of process:
    while (!feof(pipe)) {
      // use buffer to read and add to result
      if (fgets(buffer, 128, pipe) != NULL) {
        //std::cout << buffer << "\n";
        std::map<int, std::string> splitArray ;
        if(splitchar==""){
          splitArray = splitString(buffer, "=");
        }else{
          splitArray = splitString(buffer, splitchar);
        }
        string jsonKey = trim(splitArray[0]);
        string jsonValue = removeString(splitArray[1], "\n");
        jsonValue = trim(jsonValue);
        jsonData[jsonKey]  = jsonValue;
        
        result += buffer;
      }
    }
    pclose(pipe);
    //std::cout << jsonData.dump(2);
    return jsonData;
}

public: json executeTerminalJSON1(string command) {
  char buffer[128];
    string result = "";
    json jsonData;

    // Open pipe to file
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
      return "popen failed!";
    }

    // read till end of process:
    while (!feof(pipe)) {
      // use buffer to read and add to result
      if (fgets(buffer, 128, pipe) != NULL) {
        //std::cout << buffer << "\n";
        std::map<int, std::string> splitArray = splitString(buffer, ":");
        string jsonKey = trim(splitArray[0]);
        string jsonValue = removeString(splitArray[1], "\n");
        jsonValue = trim(jsonValue);
        jsonData[jsonKey]  = jsonValue;
        
        result += buffer;
      }
    }
    pclose(pipe);
    //std::cout << jsonData.dump(2);
    return jsonData;
}
public:string executeTerminal(string command) {
    char buffer[128];
    string result = "";

    // Open pipe to file
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
      return "popen failed!";
    }

    // read till end of process:
    while (!feof(pipe)) {
      // use buffer to read and add to result
      if (fgets(buffer, 128, pipe) != NULL) {
        //std::cout << buffer << "\n";
        result += buffer;
      }
    }
    pclose(pipe);
    return result;
  }

private:
  string decimal2Hex(int dec)
  {
    if (dec < 1)
      return "0";
    int hex = dec;
    string hexStr;
    while (dec > 0)
    {
      hex = dec % 16;
      if (hex < 10)
      {
        hexStr = hexStr.insert(0, string(1, (char)(hex + 48)));
      }
      else
      {
        hexStr = hexStr.insert(0, string(1, (char)(hex + 55)));
      }
      dec /= 16;
    }
    return hexStr;
  }

public:
  string encodeURL(string data) {
    string result;
    for (char c : data)
    {
      if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'))
      {
        result += c;
      }
      else
      {
        result += '%';
        string s = DecimalToHexadecimal(c);
        if (s.length() < 2)
          s = s.insert(0, "0");
        result += s;
      }
    }
    return result;
  }

private:
  string DecimalToHexadecimal(int dec) {
    if (dec < 1) return "0";
    int hex = dec;
    string hexStr;
    while (dec > 0) {
      hex = dec % 16;
      if (hex < 10)
        hexStr = hexStr.insert(0, string(1, (char)(hex + 48)));
      else
        hexStr = hexStr.insert(0, string(1, (char)(hex + 55)));

      dec /= 16;
    }
    return hexStr;
  }

public:
  std::string number2Ordinal(size_t number)
  {
    std::string suffix = "th";
    if (number % 100 < 11 || number % 100 > 13)
    {
      switch (number % 10)
      {
      case 1:
        suffix = "st";
        break;
      case 2:
        suffix = "nd";
        break;
      case 3:
        suffix = "rd";
        break;
      }
    }
    return std::to_string(number) + suffix;
  }

public:
  bool findString(string stringValue, string string2Search){
    bool returnValue = false;
    if (stringValue.find(string2Search) != std::string::npos) {returnValue = true;}
    return returnValue;
  }

public: 
  bool findMap(map<string, string>findMap, string searchString) {
    bool returnValue = false;
    if (findMap.find(searchString) != findMap.end())
      returnValue = true;
    return returnValue;
  }

public:
  string removeString(string stringValue, string string2Remove) {
    std::string::size_type i = stringValue.find(string2Remove);
    if (i != std::string::npos) {
      stringValue.erase(i, string2Remove.length());
    }
    return stringValue;
  }

public:
  string leftTrim(const string &s) {
    return regex_replace(s, regex("^\\s+"), string(""));
  }

public:
  string rightTrim(const string &s) {
    return regex_replace(s, regex("\\s+$"), string(""));
  }

public:
  string trim(const string &s) {
    string right = rightTrim(s);
    return leftTrim(right);
  }

public:
  map<std::string, std::any> json2Map(json jsonData) {
    map<std::string, std::any> jsonMap;
    for (const auto &item : jsonData.items())
    {
      json jsonChild = item.value();
      jsonMap[jsonChild["id"]] = item.value();
    }
    return jsonMap;
  }

public:
  void printMap(std::map<string, string> printMap) {
    for (std::map<string, string>::iterator it = printMap.begin(); it != printMap.end(); ++it) {
      std::cout << it->first << ":" << it->second << "\n";
    }
    std::cout << "\n";
  }

public:
  void printMap(std::map<string, std::any> printMap) {
    for (std::map<string, std::any>::iterator it = printMap.begin(); it != printMap.end(); ++it) {
      json abc = std::any_cast<json>(it->second);
      std::cout << it->first << ":" << abc.dump(2) << "\n";
    }
    std::cout << "\n";
  }
  public: json ExcetuteTerminalJSonArray(std::istringstream& input) {
    json jsonArray = json::array();
    map<string,string> resultData;
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) {
          if(!resultData.empty()){
            json jsonObject = resultData; 
            resultData.clear();
            jsonArray.insert(jsonArray.end(), jsonObject);
        }
         
        }

        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos+1);
            key=trim(key);
            value=trim(value);
            resultData[key] = value;
        }
    }
      if(!resultData.empty()){
            json jsonObject = resultData; 
            jsonArray.insert(jsonArray.end(), jsonObject);
        }

    return jsonArray;
    }
    public: json ExcetuteTerminalJSonArray1(std::istringstream& input) {
    json jsonArray = json::array();
    map<string,string> resultData;
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) {
          if(!resultData.empty()){
            json jsonObject = resultData; 
            resultData.clear();
            jsonArray.insert(jsonArray.end(), jsonObject);
        }
         
        }

        size_t delimiterPos = line.find(':');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos+1);
            key=trim(key);
            value=trim(value);
            resultData[key] = value;
        }
    }
      if(!resultData.empty()){
          json jsonObject = resultData; 
          jsonArray.insert(jsonArray.end(), jsonObject);
        }

    return jsonArray;
    }

    public: json ExcetuteTerminalJSonArray_file(std::ifstream& input_file) {
    json jsonArray = json::array();
    map<string,string> resultData;
    std::string line;

    if (!input_file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return jsonArray;
    }
    while (std::getline(input_file, line)) {
        if (line.empty()) {
          if(!resultData.empty()){
            json jsonObject = resultData; 
            resultData.clear();
            jsonArray.insert(jsonArray.end(), jsonObject);
        }
         
        }

        size_t delimiterPos = line.find(':');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos+1);
            key=trim(key);
            value=trim(value);
            resultData[key] = value;
        }
    }
      if(!resultData.empty()){
          json jsonObject = resultData; 
          jsonArray.insert(jsonArray.end(), jsonObject);
        }

    return jsonArray;
    }
    public:bool generatePopupYes(const std::string& msg) {
      std::string popup =
        "powershell -NoProfile -Command \""
        "Add-Type -AssemblyName System.Windows.Forms; "
        "$form = New-Object System.Windows.Forms.Form; "
        "$form.Text = 'NJQCA Notification'; "
        "$form.Size = New-Object System.Drawing.Size(400, 200); "
        "$form.StartPosition = 'CenterScreen'; "
        "$form.TopMost = $true; "
        "$label = New-Object System.Windows.Forms.Label; "
        "$label.Text = '" + msg + "'; "
        "$label.AutoSize = $true; "
        "$label.MaximumSize = New-Object System.Drawing.Size(380, 0); "
        "$label.Font = New-Object System.Drawing.Font('Arial', 12); "
        "$label.Location = New-Object System.Drawing.Point(10, 20); "
        "$yesButton = New-Object System.Windows.Forms.Button; "
        "$yesButton.Text = 'Yes'; "
        "$yesButton.Size = New-Object System.Drawing.Size(100, 30); "
        "$yesButton.Location = New-Object System.Drawing.Point(80, 120); "
        "$yesButton.Add_Click({ $global:Result = 1; $form.Close() }); "
        "$noButton = New-Object System.Windows.Forms.Button; "
        "$noButton.Text = 'No'; "
        "$noButton.Size = New-Object System.Drawing.Size(100, 30); "
        "$noButton.Location = New-Object System.Drawing.Point(220, 120); "
        "$noButton.Add_Click({ $global:Result = 0; $form.Close() }); "
        "$form.Controls.Add($label); "
        "$form.Controls.Add($yesButton); "
        "$form.Controls.Add($noButton); "
        "$form.Add_FormClosing({ if (-not $global:Result) { $global:Result = 0 } }); "
        "$form.ShowDialog(); "
        "Write-Output $Result\"";


    string result=executeTerminal(popup);
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    result=trim(result);
    // cout<<"\nresult="<<result;
    if(result.find("1")!=std::string::npos){
      return true;
    }else{
      return false;
    }

    }

    public:bool generatePopup(const std::string& check,const std::string& msg) {
    std::string popup =
    "powershell -NoProfile -Command \""
    "Add-Type -AssemblyName System.Windows.Forms; "
    "$form = New-Object System.Windows.Forms.Form; "
    "$form.Text = 'NJQCA Notification'; "
    "$form.Size = New-Object System.Drawing.Size(500, 250); "
    "$form.StartPosition = 'CenterScreen'; "
    "$form.TopMost = $true; "

    "$label1 = New-Object System.Windows.Forms.Label; "
    "$label1.Text = 'Warning:  " + check + "';"
    "$label1.ForeColor = 'Red'; "
    "$label1.AutoSize = $true; "
    "$label1.MaximumSize = New-Object System.Drawing.Size(460, 0); "
    "$label1.Font = New-Object System.Drawing.Font('Arial', 12, [System.Drawing.FontStyle]::Bold); "
    "$label1.Location = New-Object System.Drawing.Point(10, 20); "

    "$label2 = New-Object System.Windows.Forms.Label; "
    "$label2.Text = '" + msg + "'; "
    "$label2.AutoSize = $true; "
    "$label2.MaximumSize = New-Object System.Drawing.Size(460, 0); "
    "$label2.Font = New-Object System.Drawing.Font('Arial', 12); "
    "$label2.Location = New-Object System.Drawing.Point(10, 60); "

    "$yesButton = New-Object System.Windows.Forms.Button; "
    "$yesButton.Text = 'Not Working'; "
    "$yesButton.ForeColor = 'Red'; "
    "$yesButton.Size = New-Object System.Drawing.Size(120, 35); "
    "$yesButton.Location = New-Object System.Drawing.Point(260, 150); "
    "$yesButton.Add_Click({ $global:Result = 1; $form.Close() }); "

    "$noButton = New-Object System.Windows.Forms.Button; "
    "$noButton.Text = 'Retry'; "
    "$noButton.Size = New-Object System.Drawing.Size(120, 35); "
    "$noButton.Location = New-Object System.Drawing.Point(100, 150); "
    "$noButton.Add_Click({ $global:Result = 0; $form.Close() }); "

    "$form.Controls.Add($label1); "
    "$form.Controls.Add($label2); "
    "$form.Controls.Add($yesButton); "
    "$form.Controls.Add($noButton); "

    "$form.Add_FormClosing({ if (-not $global:Result) { $global:Result = 0 } }); "
    "$form.ShowDialog(); "
    "Write-Output $Result\"";
    string result=executeTerminal(popup);
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    result=trim(result);
    // cout<<"\nresult="<<result;
    if(result.find("1")!=std::string::npos){
      return true;
    }else{
      return false;
    }

    }
        public:bool generatePopupredok(const std::string& check,const std::string& msg) {
    std::string popup =
    "powershell -NoProfile -Command \""
    "Add-Type -AssemblyName System.Windows.Forms; "
    "$form = New-Object System.Windows.Forms.Form; "
    "$form.Text = 'NJQCA Notification'; "
    "$form.Size = New-Object System.Drawing.Size(500, 250); "
    "$form.StartPosition = 'CenterScreen'; "
    "$form.TopMost = $true; "

    "$label1 = New-Object System.Windows.Forms.Label; "
    "$label1.Text = 'Warning:  " + check + "';"
    "$label1.ForeColor = 'Red'; "
    "$label1.AutoSize = $true; "
    "$label1.MaximumSize = New-Object System.Drawing.Size(460, 0); "
    "$label1.Font = New-Object System.Drawing.Font('Arial', 12, [System.Drawing.FontStyle]::Bold); "
    "$label1.Location = New-Object System.Drawing.Point(10, 20); "

    "$label2 = New-Object System.Windows.Forms.Label; "
    "$label2.Text = '" + msg + "'; "
    "$label2.AutoSize = $true; "
    "$label2.MaximumSize = New-Object System.Drawing.Size(460, 0); "
    "$label2.Font = New-Object System.Drawing.Font('Arial', 12); "
    "$label2.Location = New-Object System.Drawing.Point(10, 60); "

    "$yesButton = New-Object System.Windows.Forms.Button; "
    "$yesButton.Text = 'OK'; "
    "$yesButton.ForeColor = 'Red'; "
    "$yesButton.Size = New-Object System.Drawing.Size(120, 35); "
    "$yesButton.Location = New-Object System.Drawing.Point(260, 150); "
    "$yesButton.Add_Click({ $global:Result = 1; $form.Close() }); "

    // "$noButton = New-Object System.Windows.Forms.Button; "
    // "$noButton.Text = 'Retry'; "
    // "$noButton.Size = New-Object System.Drawing.Size(120, 35); "
    // "$noButton.Location = New-Object System.Drawing.Point(100, 150); "
    // "$noButton.Add_Click({ $global:Result = 0; $form.Close() }); "

    "$form.Controls.Add($label1); "
    "$form.Controls.Add($label2); "
    "$form.Controls.Add($yesButton); "
    // "$form.Controls.Add($noButton); "

    "$form.Add_FormClosing({ if (-not $global:Result) { $global:Result = 0 } }); "
    "$form.ShowDialog(); "
    "Write-Output $Result\"";
    string result=executeTerminal(popup);
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    result=trim(result);
    // cout<<"\nresult="<<result;
    if(result.find("1")!=std::string::npos){
      return true;
    }else{
      return false;
    }

    }
  public:
bool generatePopupOK(const std::string& msg) {
    std::string popup =
        "powershell -NoProfile -Command \""
        "Add-Type -AssemblyName System.Windows.Forms; "
        "$form = New-Object System.Windows.Forms.Form; "
        "$form.Text = 'NJQCA Notification'; "
        "$form.Size = New-Object System.Drawing.Size(400, 200); "
        "$form.StartPosition = 'CenterScreen'; "
        "$form.TopMost = $true; "
        "$label = New-Object System.Windows.Forms.Label; "
        "$label.Text = '" + msg + "'; "
        "$label.AutoSize = $true; "
        "$label.MaximumSize = New-Object System.Drawing.Size(380, 0); "
        "$label.Font = New-Object System.Drawing.Font('Arial', 12); "
        "$label.Location = New-Object System.Drawing.Point(10, 20); "
        "$okButton = New-Object System.Windows.Forms.Button; "
        "$okButton.Text = 'OK'; "
        "$okButton.Size = New-Object System.Drawing.Size(100, 30); "
        "$okButton.Location = New-Object System.Drawing.Point(150, 120); "
        "$okButton.Add_Click({ $global:Result = 1; $form.Close() }); "
        "$form.Controls.Add($label); "
        "$form.Controls.Add($okButton); "
        "$form.Add_FormClosing({ if (-not $global:Result) { $global:Result = 0 } }); "
        "$form.ShowDialog(); "
        "Write-Output $Result\"";

    std::string result = executeTerminal(popup);
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    result = trim(result);
    return result.find("1") != std::string::npos;
}
public: bool generatePopupAlt(const std::string& check, const std::string& msg) {
    std::string popup =
        "powershell -NoProfile -Command \""
        "Add-Type -AssemblyName System.Windows.Forms; "
        "Add-Type -AssemblyName System.Drawing; "
        "$form = New-Object System.Windows.Forms.Form; "
        "$form.Text = 'NJQCA Notification'; "
        "$form.Size = New-Object System.Drawing.Size(500, 250); "
        "$form.StartPosition = 'CenterScreen'; "
        "$form.TopMost = $true; "
        "$form.KeyPreview = $true; "

        "$label1 = New-Object System.Windows.Forms.Label; "
        "$label1.Text = 'Warning: " + check + "'; "
        "$label1.ForeColor = 'Red'; "
        "$label1.AutoSize = $true; "
        "$label1.MaximumSize = New-Object System.Drawing.Size(460, 0); "
        "$label1.Font = New-Object System.Drawing.Font('Arial', 12, [System.Drawing.FontStyle]::Bold); "
        "$label1.Location = New-Object System.Drawing.Point(10, 20); "

        "$label2 = New-Object System.Windows.Forms.Label; "
        "$label2.Text = '" + msg + "'; "
        "$label2.AutoSize = $true; "
        "$label2.MaximumSize = New-Object System.Drawing.Size(460, 0); "
        "$label2.Font = New-Object System.Drawing.Font('Arial', 12); "
        "$label2.Location = New-Object System.Drawing.Point(10, 60); "

        "$form.Controls.Add($label1); "
        "$form.Controls.Add($label2); "

        "$global:Result = 0; "
        "$form.Add_KeyDown({ "
        "    if ($_.Alt -and $_.KeyCode -eq 'R') { $global:Result = 0; $form.Close() } "
        "    elseif ($_.Alt -and $_.KeyCode -eq 'N') { $global:Result = 1; $form.Close() } "
        "}); "

        "$form.Add_FormClosing({ if (-not $global:Result) { $global:Result = 0 } }); "
        "$form.ShowDialog() | Out-Null; "
        "Write-Output $Result\"";

    std::string result = executeTerminal(popup);
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    result = trim(result);
    return result.find("1") != std::string::npos;
}


int  generatePopup2value(const std::string& msg) {
  int value=0;
    std::string valuecommand =
    "powershell -NoProfile -Command \""
    "Add-Type -AssemblyName System.Windows.Forms; "
    "Add-Type -AssemblyName System.Drawing; "
    "$form = New-Object System.Windows.Forms.Form; "
    "$form.Text = 'NJQCA Notification'; "
    "$form.Size = New-Object System.Drawing.Size(400, 200); "
    "$form.StartPosition = 'CenterScreen'; "
    "$form.Topmost = $true; "
    "$label = New-Object System.Windows.Forms.Label; "
    "$label.Text = '" + msg + "'; "
    "$label.AutoSize = $true; "
    "$label.Font = New-Object System.Drawing.Font('Arial', 10); "
    "$label.Location = New-Object System.Drawing.Point(10, 20); "
    "$form.Controls.Add($label); "
    "$textBox = New-Object System.Windows.Forms.TextBox; "
    "$textBox.Size = New-Object System.Drawing.Size(300, 25); "
    "$textBox.Location = New-Object System.Drawing.Point(10, 50); "
    "$form.Controls.Add($textBox); "
    "$okButton = New-Object System.Windows.Forms.Button; "
    "$okButton.Text = 'OK'; "
    "$okButton.Location = New-Object System.Drawing.Point(200, 80); "
    "$okButton.Add_Click({ $form.Close() }); "
    "$form.Controls.Add($okButton); "
    "$form.ShowDialog() | Out-Null; "
    "$userInput = $textBox.Text; "
    "if ($userInput -match '^[0-9]+$') { "
    "    Write-Host $userInput; "
    "} else { "
    "    Write-Host 'Invalid input. Please enter a number.'; "
    "}\"";


    std::string result = executeTerminal(valuecommand);
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    result = trim(result);
    try{
      value=stoi(result);
    }catch(exception & e){
      cout<<"Error while getting the value in popup"<<e.what()<<endl;
    }
    return value;
}

bool generateNOT_workingPopup(string msg){
  std::string psScript = 
        "powershell -NoProfile -Command \""
        "Add-Type -AssemblyName System.Windows.Forms; "
        "Add-Type -AssemblyName System.Drawing; "
        "$form = New-Object System.Windows.Forms.Form; "
        "$form.Text = 'NJQCA Notifications'; "
        "$form.Size = New-Object System.Drawing.Size(400, 200); "
        "$form.StartPosition = 'CenterScreen'; "
        "$form.Topmost = $true; "
        "$label = New-Object System.Windows.Forms.Label; "
        "$label.Text = '" + msg + "'; "
        "$label.AutoSize = $true; "
        "$label.MaximumSize = New-Object System.Drawing.Size(380, 0); "
        "$label.Font = New-Object System.Drawing.Font('Arial', 12); "
        "$label.Location = New-Object System.Drawing.Point(10, 20); "
        "$form.Controls.Add($label); "
        "$button = New-Object System.Windows.Forms.Button; "
        "$button.Text = 'Not Working'; "
        "$button.Location = New-Object System.Drawing.Point(150, 120); "
        "$button.Size = New-Object System.Drawing.Size(100, 30); "
        "$form.Controls.Add($button); "
        "$button.Add_Click({ Write-Host 'Not Working'; $form.Close(); }); "
        "$form.Add_KeyDown({ "
        "    if ($_.Alt -and $_.KeyCode -eq 'N') { "
        "        Write-Host 'Not Working'; "
        "        $form.Close(); "
        "    } "
        "}); "
        "$form.KeyPreview = $true; "
        "$form.ShowDialog();\"";
      string result = executeTerminal(psScript);
      // result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
      // result = trim(result);
      return result.find("Not Working") != std::string::npos;

}

public: void Generatepopup(const std::string& msg) {
    std::string Script =
        "powershell -NoProfile -Command \""
        "Add-Type -AssemblyName System.Windows.Forms; "
        "Add-Type -AssemblyName System.Drawing; "
        "$form = New-Object System.Windows.Forms.Form; "
        "$form.Text = 'NJQCA Notifications'; "
        "$form.Size = New-Object System.Drawing.Size(400, 200); "
        "$form.StartPosition = 'CenterScreen'; "
        "$form.Topmost = $true; "
        "$label = New-Object System.Windows.Forms.Label; "
        "$label.Text = '" + msg + "'; "
        "$label.AutoSize = $true; "
        "$label.MaximumSize = New-Object System.Drawing.Size(380, 0); "
        "$label.Font = New-Object System.Drawing.Font('Arial', 12); "
        "$label.Location = New-Object System.Drawing.Point(10, 20); "
        "$form.Controls.Add($label); "
        "$form.add_Shown({ Start-Sleep -Seconds 1; $form.Close() }); "
        "$form.ShowDialog();\"";
    std::string result = executeTerminal(Script);
}

public:string selected_option(vector<string>options){
  std::ostringstream itemsStream;
    itemsStream << "$listBox.Items.AddRange(@(";
    for (size_t i = 0; i < options.size(); ++i) {
        itemsStream << "'" << options[i] << "'";
        if (i < options.size() - 1)
            itemsStream << ", ";
    }
    itemsStream << ")); ";

    std::string Script =
        "powershell -NoProfile -Command \""
        "Add-Type -AssemblyName System.Windows.Forms; "
        "Add-Type -AssemblyName System.Drawing; "

        "$form = New-Object System.Windows.Forms.Form; "
        "$form.Text = 'Select an Option'; "
        "$form.Size = New-Object System.Drawing.Size(320, 300); "
        "$form.StartPosition = 'CenterScreen'; "

        "$listBox = New-Object System.Windows.Forms.ListBox; "
        "$listBox.Size = New-Object System.Drawing.Size(280, 150); "
        "$listBox.Location = New-Object System.Drawing.Point(10, 10); "
        "$listBox.Font = New-Object System.Drawing.Font('Segoe UI', 12); "

        + itemsStream.str() +

        "$form.Controls.Add($listBox); "

        "$okButton = New-Object System.Windows.Forms.Button; "
        "$okButton.Text = 'OK'; "
        "$okButton.Size = New-Object System.Drawing.Size(75, 30); "
        "$okButton.Location = New-Object System.Drawing.Point(10, 210); "
        "$okButton.Font = New-Object System.Drawing.Font('Segoe UI', 10); "
        "$okButton.Add_Click({ "
        "if ($listBox.SelectedItems.Count -gt 0) { "
        "    $selected = @(); "
        "    foreach ($item in $listBox.SelectedItems) { $selected += $item.ToString() } "
        "    $form.Tag = ($selected -join ','); "
        "    $form.Close(); "
        "} else { "
        "    [System.Windows.Forms.MessageBox]::Show('Please select an option.'); "
        "} "
        "}); "
        "$form.Controls.Add($okButton); "

        "$cancelButton = New-Object System.Windows.Forms.Button; "
        "$cancelButton.Text = 'Cancel'; "
        "$cancelButton.Size = New-Object System.Drawing.Size(75, 30); "
        "$cancelButton.Location = New-Object System.Drawing.Point(($form.ClientSize.Width - 85), 210); "
        "$cancelButton.Anchor = 'Bottom, Right'; "
        "$cancelButton.Font = New-Object System.Drawing.Font('Segoe UI', 10); "
        "$cancelButton.Add_Click({ $form.Close() }); "
        "$form.Controls.Add($cancelButton); "

        "$form.ShowDialog() | Out-Null; "
        "if ($form.Tag) { Write-Output \\\"$($form.Tag)\\\" } "
        "else { Write-Output \\\" \\\" } "
        "\"";
      string result=executeTerminal(Script);
      result=trim(result);
      return result;
}

public:
string multi_selection_option(vector<string> options) {
    std::ostringstream itemsStream;
    itemsStream << "$listBox.Items.AddRange(@(";
    for (size_t i = 0; i < options.size(); ++i) {
        itemsStream << "'" << options[i] << "'";
        if (i < options.size() - 1)
            itemsStream << ", ";
    }
    itemsStream << ")); ";

std::string Script =
    "powershell -NoProfile -Command \""
    "Add-Type -AssemblyName System.Windows.Forms; "
    "Add-Type -AssemblyName System.Drawing; "

    "$form = New-Object System.Windows.Forms.Form; "
    "$form.Text = 'NJQCA Notifications'; "
    "$form.Size = New-Object System.Drawing.Size(400, 250); "
    "$form.StartPosition = 'CenterScreen'; "

    "$header = New-Object System.Windows.Forms.Label; "
    "$header.Text = 'Please Select Not Working OR Not Available Displays?'; "
    "$header.Font = New-Object System.Drawing.Font('Segoe UI', 10, [System.Drawing.FontStyle]::Bold); "
    "$header.ForeColor = [System.Drawing.Color]::Yellow; "
    "$header.BackColor = [System.Drawing.Color]::FromArgb(45,45,48); "
    "$header.AutoSize = $true; "
    "$header.Location = New-Object System.Drawing.Point(10, 5); "
    "$form.Controls.Add($header); "

    "$listBox = New-Object System.Windows.Forms.ListBox; "
    "$listBox.Size = New-Object System.Drawing.Size(360, 120); "
    "$listBox.Location = New-Object System.Drawing.Point(10, 40); "
    "$listBox.Font = New-Object System.Drawing.Font('Segoe UI', 12); "
    "$listBox.SelectionMode = 'MultiSimple'; "

    + itemsStream.str() +

    "$form.Controls.Add($listBox); "

    "$okButton = New-Object System.Windows.Forms.Button; "
    "$okButton.Text = 'OK'; "
    "$okButton.Size = New-Object System.Drawing.Size(75, 30); "
    "$okButton.Location = New-Object System.Drawing.Point(10, 170); "
    "$okButton.Font = New-Object System.Drawing.Font('Segoe UI', 10); "
    "$okButton.Add_Click({ "
    "if ($listBox.SelectedItems.Count -gt 0) { "
    "   $form.Tag = ($listBox.SelectedItems -join ', '); $form.Close() "
    "} else { [System.Windows.Forms.MessageBox]::Show('Please select at least one option.') } "
    "}); "
    "$form.Controls.Add($okButton); "

    "$cancelButton = New-Object System.Windows.Forms.Button; "
    "$cancelButton.Text = 'Cancel-Retry'; "
    "$cancelButton.Size = New-Object System.Drawing.Size(120, 30); "
    "$cancelButton.Location = New-Object System.Drawing.Point(($form.ClientSize.Width - 135), 170); "
    "$cancelButton.Anchor = 'Bottom, Right'; "
    "$cancelButton.Font = New-Object System.Drawing.Font('Segoe UI', 10); "
    "$cancelButton.Add_Click({ $form.Close() }); "
    "$form.Controls.Add($cancelButton); "

    "$form.ShowDialog() | Out-Null; "
    "if ($form.Tag) { Write-Output \\\"$($form.Tag)\\\" } "
    "else { Write-Output \\\" \\\" } "
    "\"";




    string result = executeTerminal(Script);
    result = trim(result);
    return result;
}


  public:string extractInteger(std::string str) {
    std::string numberstr;
    for(char ch :str){
      if(isdigit(ch)){
        numberstr+=ch;
      }else{
        continue;
      }
    }
    return numberstr;
  }

};