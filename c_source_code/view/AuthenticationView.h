#include <map>
#include <conio.h> // For _getch()
#include <windows.h> // For Windows API functions
#include <freshweb/UtilTable.h>

class AuthenticationView {
  public: map<string,string> authentication() {
    string username;
    string password;
    map<string,string> inputMap;
    bool isEmpty = true;

    while (isEmpty) {
      std::cout << "Username: ";
      std::cin >> username;
      if (username != "") {isEmpty = false;} else {std::cout << "Please enter the username)\n";}
    }
    std::cin.clear(); // Clear error flags
    std::cin.sync();  // Clear input buffer
    char ch = getchar();
    std::cout << "Password: ";
    password = getPassword();
    
    inputMap["username"] = username;
    inputMap["password"] = password;
    return inputMap;
  }
private:
    std::string getPassword() {
        std::string password;
        char ch;

        while (true) {
            ch = _getch(); // Use _getch() to read characters without echoing
            if (ch == '\r') { // Enter key
                std::cout << std::endl;
                break;
            } 
            else if (ch == '\b') { // Backspace
                if (!password.empty()) {
                    std::cout << "\b \b"; // Move cursor back, print space, move cursor back again
                    password.pop_back();
                }
            } 
            else {
                std::cout << '*'; // Display asterisk
                password.push_back(ch); // Store actual character
            }
        }

        return password;
    }
};