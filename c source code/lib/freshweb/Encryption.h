#include <iostream>
#include <string>

using namespace std;

class Encryption {
  private: string logMessage;

  public: std::string encrypt(std::string msg, std::string key) {
    std::string tmp(key);
    while (key.size() < msg.size())
      key += tmp;
    
    // And now for the encryption part
    for (std::string::size_type i = 0; i < msg.size(); ++i)
        msg[i] ^= key[i];
    return msg;
  }

  public: std::string decrypt(std::string msg, std::string key) {
    return encrypt(msg, key);
  }

  string bubble_digit(char digit) {
    // Convert single digit to "bubbled" form
    if (isdigit(digit)) {
        int value = (digit - '0') * 2; // Multiply the numeric value by 2
        return value < 10 ? "0" + to_string(value) : to_string(value); // Add leading zero if needed
    }
    return string(1, digit); // Return as-is if not a digit
}

string encrypt_with_shift(const string text, int shift) {
    string encrypted = "";
    for (char c : text) {
        if (isdigit(c)) {
            // Bubble the digit
            encrypted += bubble_digit(c);
        } else if (isalpha(c)) {
            char shift_base = isupper(c) ? 'A' : 'a';
            encrypted += (c - shift_base + shift) % 26 + shift_base;
        } else {
            encrypted += c; // Keep non-alphabetic characters as is
        }
    }
    return encrypted;
}

string decrypt_with_shift(const string encrypted_text, int shift) {
    string decrypted = "";
    for (size_t i = 0; i < encrypted_text.size(); ++i) {
        if (isdigit(encrypted_text[i])) {
            // Reconstruct the original digit
            if (i + 1 < encrypted_text.size() && isdigit(encrypted_text[i + 1])) {
                string bubbled_digit = encrypted_text.substr(i, 2);
                int original_digit = stoi(bubbled_digit) / 2;
                decrypted += to_string(original_digit);
                i++; // Skip the next character since we processed a pair
            }
        } else if (isalpha(encrypted_text[i])) {
            char shift_base = isupper(encrypted_text[i]) ? 'A' : 'a';
            decrypted += (encrypted_text[i] - shift_base - shift + 26) % 26 + shift_base;
        } else {
            decrypted += encrypted_text[i]; // Keep non-alphabetic characters as is
        }
    }
    return decrypted;
}
};