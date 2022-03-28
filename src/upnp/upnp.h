//returns:
//-1: no UPnP Device
// 0: found but doesn't work
// 1: ok
//

int testForUPNP();
int addMapping(char* IP, char* intPort, char* extPort, char* protocol);
int removeMapping(char* extPort, char* protocol);
std::string getLanAddress();
std::string getExtAddress();

