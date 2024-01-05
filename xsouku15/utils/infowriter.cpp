/*
*      Autor: Martin Soukup
*      Login: xsouku15
*
*/
#include "infowriter.hpp"


void writeRRQ(sockaddr_in src_sockaddr, char filename[], char mode[], std::vector<OptionInfo> optionVector = {})
{
    std::string optionString;
    for( size_t i = 0; i < optionVector.size(); ++i)
    {
        optionString += optionVector[i].name + "=" + optionVector[i].value + " ";
    }
    std::cerr << "RRQ " << inet_ntoa(src_sockaddr.sin_addr) << ":" << (src_sockaddr.sin_port) << " \"" << filename << "\" " << mode << " " << optionString << std::endl;
}

void writeWRQ(sockaddr_in src_sockaddr, char filename[], char mode[], std::vector<OptionInfo> optionVector)
{
    std::string optionString;
    for( size_t i = 0; i < optionVector.size(); ++i)
    {
        optionString += optionVector[i].name + "=" + optionVector[i].value + " ";
    }
    std::cerr << "WRQ " << inet_ntoa(src_sockaddr.sin_addr) << ":" << ntohs(src_sockaddr.sin_port) << " \"" << filename << "\" " << mode << " " << optionString << std::endl;

}

void writeACK(sockaddr_in src_sockaddr, int blockNumber)
{
    std::cerr << "ACK " << inet_ntoa(src_sockaddr.sin_addr) << ":" << ntohs(src_sockaddr.sin_port) << " " << ntohs(blockNumber) << std::endl;
}

void writeOACK(sockaddr_in src_sockaddr, std::vector<OptionInfo> optionVector)
{
    std::string optionString;
    for( size_t i = 0; i < optionVector.size(); ++i)
    {
        optionString += optionVector[i].name + "=" + optionVector[i].value + " ";
    }
    std::cerr << "OACK " << inet_ntoa(src_sockaddr.sin_addr) << ":" << ntohs(src_sockaddr.sin_port) << " " << optionString << std::endl;
}

void writeDATA(sockaddr_in src_sockaddr, int port, int blockNumber)
{
    std::cerr << "DATA " << inet_ntoa(src_sockaddr.sin_addr) << ":" << ntohs(src_sockaddr.sin_port) << ":" << port << " " << ntohs(blockNumber) << std::endl;
}

void writeERROR(sockaddr_in src_sockaddr, int port, int error, std::string message)
{
    std::cerr << "ERROR " << inet_ntoa(src_sockaddr.sin_addr) << ":" << ntohs(src_sockaddr.sin_port) << ":" << port << " " << ntohs(error) << " " << message << std::endl;
}

