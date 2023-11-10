#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <arpa/inet.h>
#include <thread>
#include <cctype>
#include <sstream>
#include <stack>

#define BUFSIZE 1024


class Server {
public:
    Server(int port_s, const char* serverAdress, std::string root)
        : serv_port(port_s), stopRequest(false),root_dirpath(root), sockfd(-1)
    {

        sockaddr_in adress;
		std::memset(&adress, 0, sizeof(sockaddr_in));
        adress.sin_family = AF_INET;
        adress.sin_addr.s_addr = inet_addr(serverAdress);
        adress.sin_port = htons(serv_port);

       
            sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if ( sockfd < 0 )
            {
                std::cerr << "ERROR creating UDP socket";
                exit( EXIT_FAILURE );
            }

            if (bind(sockfd, (sockaddr*)&adress, sizeof(adress)) < 0)
            {
                std::cerr << "ERROR could not bind socket to adress";
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        }


	void Start() {

		while( !stopRequest )
		{
			sockaddr_in clientAddress;
			socklen_t clientAddressLen = sizeof(clientAddress);
			handleTFTP(sockfd, clientAddress);
		} 

	}

private:

	void handleTFTP(int clientSocket, const sockaddr_in& clientAddr) 
{
    char buffer[BUFSIZE];
    int numbytes = 0;

    socklen_t clientAddrLen = sizeof(clientAddr);

    while(true)
    {
        numbytes = recvfrom(clientSocket, buffer, BUFSIZE, 0, (sockaddr*) &clientAddr, &clientAddrLen);

        if( numbytes == -1)
        {
            std::cerr << "Error in recvfrom(), Exiting";
            continue;
        }

        if( numbytes == 0 )
        {
            std::cout << "Client disconnected " << std::endl;
            continue;
        }

        uint16_t opcode = ntohs(*reinterpret_cast<uint16_t*>(buffer)); // TFTP opcode is 2 bytes
        uint16_t status = 0; // Assuming status codes are 2 bytes
        uint16_t result = 0;

        if (opcode == 1) // TFTP Read Request (RRQ)
        {
            std::string filename(buffer + 2); // Extract the filename from the buffer
            std::cout << "Received Read Request for file: " << filename << std::endl;

            // TFTP Read request

            // For example, you can send the file in blocks of 512 bytes as follows:
            // sendto(clientSocket, data, data_size, 0, (struct sockaddr *)&clientAddr, clientAddrLen);

        }
        else if (opcode == 2) // TFTP Write Request (WRQ)
        {
            std::string filename(buffer + 2); 
            std::cout << "Received Write Request for file: " << filename << std::endl;

            // TFTP write request

            // For example, you can receive the file in blocks of 512 bytes as follows:
            // recvfrom(clientSocket, buffer, BUFSIZE, 0, (sockaddr*) &clientAddr, &clientAddrLen);

        }
        else
        {
            // Unsupported opcode, send an error response
            std::string error = "Unsupported TFTP opcode";
            status = 5; // Error code 5 for unknown transfer ID
            uint16_t response[BUFSIZE] = {0, status};
            std::memcpy(response + 2, error.c_str(), error.size());
            sendto(clientSocket, (char*)response, error.size() + 2, 0, (struct sockaddr *)&clientAddr, clientAddrLen);
            continue;
        }
    }

    close(clientSocket);

}


	int serv_port;
	int sockfd;
	sockaddr_in adress;
	const std::string root_dirpath;
	std::vector<int> clients;
	bool stopRequest;

};
