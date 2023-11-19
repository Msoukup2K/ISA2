#include <netinet/in.h>
#include <vector>
#include <iostream>
#include <string>
#include "utils.hpp"

void writeRRQ(struct sockaddr_in src_sockaddr, char filename[], char mode[],std::vector<OptionInfo> optionVector);
void writeWRQ(struct sockaddr_in src_sockaddr, char buffer[], char mode[],std::vector<OptionInfo> optionVector);
void writeACK(struct sockaddr_in src_sockaddr, int blockNumber);
void writeOACK(struct sockaddr_in src_sockaddr, std::vector<OptionInfo> optionVector = {});
void writeDATA(struct sockaddr_in src_sockaddr, int port, int blockNumber);
void writeERROR(struct sockaddr_in src_sockaddr, int port, int error, std::string messsage);