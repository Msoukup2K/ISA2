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
#include <fstream>

#include "../utils/utils.hpp"

#define BUFSIZE 1024

class Server {
public:
    Server(int port_s = 69, std::string root = "server_dir")
        : serv_port(port_s), stopRequest(false),root_dirpath(root), sockfd(-1), c_adress()
    {

		for( int i = 0; i<BUFSIZE ;i++)
		{
			buffer[i] = '\0';

		}

		socklen_t len;
		struct sockaddr_in servaddr;
		bzero(&servaddr, sizeof(servaddr));

		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd < 0)
		{
			std::cout << "Error opening socket" << std::endl;
		}
		servaddr.sin_addr.s_addr = INADDR_ANY;
		servaddr.sin_port = htons(port_s);
		servaddr.sin_family = AF_INET;

		int reuse = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
		{
			perror("setsockopt(SO_REUSEADDR) failed");
			// Handle the error
		}

		if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))
		{
			perror("Error handling socket");
			std::cout << "Error creating a socket, could not bind" << std::endl;
		}
	}

	void Start() {

		socklen_t len;

		while(1)
		{
			len = sizeof(c_adress);
			int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&c_adress, &len);

            
            handleTFTPRequest();

            memset(buffer, 0, sizeof(buffer));
			
		}

	}

private:
   
 void handleTFTPRequest()
    {
		struct sockaddr_in *sin = (struct sockaddr_in *)&c_adress;

        //Extract opcode from the TFTP request
        uint16_t opcode = (buffer[0] << 8) | buffer[1];
		
        // Handle different TFTP opcodes
        switch (opcode)
        {
        case 1:
            // Read request (RRQ)
            handleRRQ();
            break;
        case 2:
            // Write request (WRQ)
            handleWRQ();
            break;
        // Add more cases as needed for other TFTP opcodes
        default:
            // Unsupported opcode
            break;
        }
    }

    void handleRRQ()
    {
		TFTPRequest *request = reinterpret_cast<TFTPRequest *>(buffer);

		
		std::string filepath = root_dirpath + "/" + request->filename;
        std::ifstream file(filepath.c_str(), std::ios::binary);
		if ( !file.is_open() )
		{
			std::cerr << "Error opening file: " << filepath << std::endl;
			return;
		}

		sendAck(0,6);

		socklen_t client_len = sizeof(c_adress);
		ssize_t received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&c_adress, &client_len);

		if(received < 0 )
		{
			perror("Receiving failed");

			std::cerr << "Failed to receive initial data block" << std::endl;
			return;
		}

		uint16_t clientBlocksize = 512;

		if(received >= 4 )
		{
			TFTPDataBlock *dataBlock = reinterpret_cast<TFTPDataBlock *>(buffer);

			if(ntohs(dataBlock->opcode) == 4)
			{
				clientBlocksize = ntohs(dataBlock->blockNumber);

				std::cout << clientBlocksize << std::endl;
				std::cerr << "Got ACK od klienta" << std::endl;

			}
			else
			{
				std::cerr << "Unexpected tftp packet type" << std::endl;

				return;

			}
		}

		const size_t blockSize = clientBlocksize > 0 ? clientBlocksize : 512;

		size_t block = 1;
		while(true)
		{
			std::vector<char> file_buffer(blockSize);
    		file.read(file_buffer.data(), blockSize);

			size_t actualDataSize = static_cast<size_t>(file.gcount());


			TFTPDataBlock data_block;
			data_block.opcode = htons(3);
			data_block.blockNumber = htons(block);
            memcpy(data_block.data, file_buffer.data(), actualDataSize);

			if(sendto(sockfd, &data_block, actualDataSize + 4, 0, (struct sockaddr *)&c_adress, sizeof(c_adress)) < 0)
			{ 
				perror("Error handling data block");
				std::cerr << "Failed to send data block: " << errno << std::endl;
				break;
			}

			++block;

			std::cerr << "size" << file_buffer.size() << "vs blocksize " << blockSize << std::endl;

			if( file.eof() )
			{
				
				break;

			}


			received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&c_adress, &client_len);

			if (received < 0)
            {
                perror("Receiving acknowledgment failed");
                std::cerr << "Failed to receive acknowledgment. Error code: " << errno << std::endl;
                // You may handle the error and send an appropriate response to the client
                break;
            }

			 if (received >= 4)
            {
                TFTPDataBlock *ackPacket = reinterpret_cast<TFTPDataBlock *>(buffer);

                // Check if it's an acknowledgment packet
                if (ntohs(ackPacket->opcode) == 4)
                {
                }
                else
                {
                    // Unexpected packet type or block number
                    std::cerr << "Unexpected acknowledgment packet" << std::endl;
                    // You may handle the error and send an appropriate response to the client
                    break;
                }
            }
            else
            {
                // Invalid acknowledgment packet size
                std::cerr << "Invalid acknowledgment packet size" << std::endl;
                // You may handle the error and send an appropriate response to the client
                break;
            }

		}
		file.close();
		
       
    }

    void handleWRQ()
    {
		struct sockaddr_in *sin = (struct sockaddr_in *)&c_adress;

		fprintf(stdout, "Incoming request from %s:%d\n", inet_ntoa(sin->sin_addr), sin->sin_port);

        // Extract filename from the TFTP request
        std::string filename(&buffer[2]);

		std::string filepath = root_dirpath + "/" + filename;

		std::ofstream file(filepath, std::ios::binary);

		if(!file.is_open())
		{
			std::cerr << "Error opening file for writing" << filepath << std::endl;

			return;
		}


		sendAck(0,6);

		size_t block = 1;

		socklen_t client_len = sizeof(c_adress);
		ssize_t received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&c_adress, &client_len);
		if( received < 0 )
		{
			perror("Receiving failed");
			std::cerr << "Failed to receive" << std::endl;
			return;
		}

		uint16_t clientBlocksize = 512;

		if (received >= 4)
		{
			TFTPDataBlock *datablock = reinterpret_cast<TFTPDataBlock *>(buffer);

			if( ntohs(datablock->opcode) == 3)
			{
				clientBlocksize = ntohs(datablock->blockNumber);

				sendAck(block,4);
			}
			else
			{
				std::cerr << "Unexpected TFTP packet type" << std::endl;
				return;
			}
		}

		const size_t blockSize = clientBlocksize > 0 ? clientBlocksize : 512;

		while (true)
		{
			ssize_t received = recvfrom(sockfd, buffer, blockSize+4, 0, (struct sockaddr *)&c_adress, &client_len);
			if( received < 0 )
			{
				perror("Receiving failed");
				std::cerr << "Failed to receive data, Error: " << errno << std::endl;

				break;
			}

			if( received >= 4 )
			{
				TFTPDataBlock *dataBlock = reinterpret_cast<TFTPDataBlock *>(buffer);

				if (ntohs(dataBlock->opcode) == 3)
				{

					if( ntohs(dataBlock->blockNumber) == block)
					{
						file.write(dataBlock->data, received - 4);

						++block;

						sendAck(block,4);
					}
					else
					{
						//TODO
						//Send error packet
						break;
					}
				}
				else
				{
					perror("Undefined: ");
					std::cerr << "Unexpected TFTP packet type" << std::endl;

					break;
				}
			}


		}

		file.close();

    }

	void sendAck(uint16_t block,int opc)
	{
		std::cout << "Sending ACK packet." << std::endl;

		TFTPDataBlock acknowledment;

		acknowledment.opcode = htons(opc);
		acknowledment.blockNumber = htons(block);

		if( sendto(sockfd, &acknowledment, sizeof(acknowledment), 0, (struct sockaddr *)&c_adress, sizeof(c_adress)) < 0)
		{
			perror("Error sending ACK packet");
			std::cerr << "Failed to send ACK packet. Error code: " << errno << std::endl;

		}

	}
	

	int serv_port;
	int sockfd;
	sockaddr_in c_adress;
	const std::string root_dirpath;
	std::vector<int> clients;
	bool stopRequest;
	char buffer[BUFSIZE];


};
