#include "infowriter.hpp"


void writeRRQ(sockaddr_in src_sockaddr, char filename[], char mode[], std::vector<OptionInfo> optionVector = {})
{
    std::string optionString;
    for( size_t i = 0; i < optionVector.size(); ++i)
    {
        optionString += optionVector[i].name + "=" + std::to_string(optionVector[i].position) + " ";
    }
    std::cerr << "RRQ " << inet_ntoa(src_sockaddr.sin_addr) << ":" << (src_sockaddr.sin_port) << " \"" << filename << "\" " << mode << optionString << std::endl;
}

void writeWRQ(sockaddr_in src_sockaddr, char filename[], char mode[], std::vector<OptionInfo> optionVector)
{
    std::string optionString;
    for( size_t i = 0; i < optionVector.size(); ++i)
    {
        optionString += optionVector[i].name + "=" + std::to_string(optionVector[i].position) + " ";
    }
    std::cerr << "WRQ " << inet_ntoa(src_sockaddr.sin_addr) << ":" << ntohs(src_sockaddr.sin_port) << " \"" << filename << "\" " << mode << optionString << std::endl;

}

void writeACK(sockaddr_in src_sockaddr, int blockNumber)
{
    std::cerr << "ACK " << inet_ntoa(src_sockaddr.sin_addr) << ":" << ntohs(src_sockaddr.sin_port) << " " << blockNumber << std::endl;
}

void writeOACK(sockaddr_in src_sockaddr, std::vector<OptionInfo> optionVector)
{
    std::string optionString;
    for( size_t i = 0; i < optionVector.size(); ++i)
    {
        optionString += optionVector[i].name + "=" + std::to_string(optionVector[i].position) + " ";
    }
    std::cerr << "OACK " << inet_ntoa(src_sockaddr.sin_addr) << ":" << ntohs(src_sockaddr.sin_port) <<  optionString << std::endl;
}

void writeDATA(sockaddr_in src_sockaddr, int port, int blockNumber)
{
    std::cerr << "DATA " << inet_ntoa(src_sockaddr.sin_addr) << ":" << ntohs(src_sockaddr.sin_port) << ":" << port << " " << blockNumber << std::endl;
}

void writeERROR(sockaddr_in src_sockaddr, int port, int error, std::string message)
{
    std::cerr << "ERROR " << inet_ntoa(src_sockaddr.sin_addr) << ":" << ntohs(src_sockaddr.sin_port) << ":" << port << " " << error << " " << message << std::endl;
}

