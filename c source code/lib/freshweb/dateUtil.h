#include <iostream>
#include <string>

using namespace std;

class DateUtil {
	public: string getGMTDateTime() {
    // current date/time based on current system
		time_t now = time(0);
		tm *gmtm = gmtime(&now);
		char*dt = asctime(gmtm);
		string gmDate = std::to_string(1900 + gmtm->tm_year) + "-" + std::to_string(gmtm->tm_mon) + "-" + std::to_string(gmtm->tm_mday) + " " + 
			std::to_string(gmtm->tm_hour) + ":" + std::to_string(gmtm->tm_min) + ":" + std::to_string(gmtm->tm_sec);
		return gmDate;
  }
};

/*cout << "Year:" << 1900+ ltm->tm_year<<endl;
   cout << "Month: "<< ltm->tm_mon<< endl;
   cout << "Day: "<<  ltm->tm_mday << endl;
   cout << "Time: "<< ltm->tm_hour << ":";
   cout << ltm->tm_min << ":";
   cout << ltm->tm_sec << endl;*/