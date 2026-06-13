#include <iostream>
#include <string>

using namespace std;

class GetFromERPModel {
  private: int limit = 0;

  public: int getLimit() {return limit;}
  public: void setLimit(int limitLocal) {limit = limitLocal;}

};