#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
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
#include <filesystem>
#include <csignal>

#include "../utils/utils.hpp"
#include "../utils/netasciiparser.hpp"
#include "../utils/infowriter.hpp"

#define BUFSIZE 512

namespace fs = std::filesystem;

/*static volatile sig_atomic_t stopRequest;
static void handleSIGINT(int signum)
{
	std::cout << "Closing server" << std::endl;

	exit(signum);

}*/

class Server {
public:
    Server(int port_s = 69, std::string root = "")
        : serv_port(port_s),stopRequest(false),root_dirpath(root), sockfd(-1), c_adress()
    {
		if( root_dirpath == "")
		{
			exit(1);
		}

		//signal(SIGINT,handleSIGINT);

		for( int i = 0; i<BUFSIZE ;i++)
		{
			buffer[i] = '\0';

		}

		socklen_t len;
		struct sockaddr_in servaddr;
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_addr.s_addr = INADDR_ANY;
		servaddr.sin_port = htons(port_s);
		servaddr.sin_family = AF_INET;

		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd < 0)
		{
			std::cout << "Error opening socket" << std::endl;
		}

		int reuse = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
		{
			// Handle the error
		}

		if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))
		{
			std::cout << "Error creating a socket, could not bind" << std::endl;
			close(sockfd);
			exit(1);
		}	
	}

	void Start() 
	{

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
        uint16_t opcode = (buffer[0] << 8) | buffer[1];
		
        // Handle TFTP opcodes
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
		case 3:
			writeDATA(c_adress, serv_port, 0);
			break;
		case 4:
			writeACK(c_adress,0);
			break;
		case 5:
			writeERROR(c_adress,serv_port,4,"Illegal TFTP operation");
			break;
		case 6:
			writeOACK(c_adress);
			break;
        default:
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
			std::cout << "Error opening file: " << filepath << std::endl;
			//TODO ERRORS
			return;
		}

		uint16_t clientBlocksize = 512;
		ssize_t received;
		socklen_t client_len = sizeof(c_adress);
		int blocksize = 0;
		int tsize = 0;
		struct timeval timeout;
		//kontrola co to je za option musi se projit vsechna option

		std::vector<OptionInfo> optionVector;
		int i = 1;
    	for (const auto &opt : request->opts) {
			
			if( !strcmp( opt.option, "blksize"))
			{
				blocksize = atoi(opt.value);
				optionVector.emplace_back("blksize", opt.value, i);

				std::cout << blocksize << std::endl;

			}
			else if( !strcmp( opt.option, "tsize"))
			{
				tsize = atoi(opt.value);
				if(tsize > 65535)
				{
					continue;
				}
				optionVector.emplace_back("tsize", opt.value, i);

				std::cout << tsize << std::endl;
			}
			else if( !strcmp( opt.option, "timeout"))
			{
				timeout.tv_sec = atoi(opt.value);
				optionVector.emplace_back("timeout", opt.value, i);

				std::cout << tsize << std::endl;
			}
			
			i++;
    	}
		writeRRQ(c_adress, request->filename, request->mode, optionVector);


		if( !optionVector.empty() )
		{
			sendOAck(optionVector,0, 6);
			received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&c_adress, &client_len);

			if(received < 0 )
			{
				std::cout << "Failed to receive initial data block" << std::endl;
				return;
			}
			
			uint16_t opcode = (buffer[0] << 8) | buffer[1];

			if( opcode != 4 )
			{
				//Poslat error paket
				return;
			}
			if(received >= 4 )
			{
				TFTPACK *dataBlock = reinterpret_cast<TFTPACK *>(buffer);

				if(dataBlock->blockNumber != 0)	
				{
					//poslat Error ze prisel spatny block
				}

				writeACK(c_adress, dataBlock->blockNumber);
			}
		}

		const size_t blockSize = blocksize > 0 ? blocksize : 512;
		size_t block = 1;
		while(true)
		{
			std::vector<char> file_buffer(blockSize);
    		file.read(file_buffer.data(), blockSize);

			//parser pro mode
			//convertToNetascii( file_buffer );

			size_t actualDataSize = static_cast<size_t>(file.gcount());
			size_t sizeToSend;

			if( actualDataSize > blockSize )
			{
				 sizeToSend = blockSize;
			}
			else
			{
				 sizeToSend = actualDataSize;
			}

			TFTPDataBlock data_block;
			data_block.opcode = htons(3);
			data_block.blockNumber = htons(block);
            memcpy(data_block.data, file_buffer.data(), sizeToSend);

			fd_set readfds;
			FD_ZERO(&readfds);
			FD_SET(sockfd, &readfds);

			if(sendto(sockfd, &data_block, sizeToSend + 4, 0, (struct sockaddr *)&c_adress, sizeof(c_adress)) < 0)
			{ 
				std::cout << "Failed to send data block: " << errno << std::endl;
				break;
			}

			++block;

			int result = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

			if( result == -1)
			{
				return;
			}
			else if( result == 0 )
			{
				std::cout << "Timeout occured" << std::endl;
				if(sendto(sockfd, &data_block, sizeToSend + 4, 0, (struct sockaddr *)&c_adress, sizeof(c_adress)) < 0)
				{ 
					std::cout << "Failed to send data block: " << errno << std::endl;
					break;
				}
			}

			if( file.eof() )
			{
				
				break;

			}

			received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&c_adress, &client_len);

			if (received < 0)
            {
                std::cout << "Failed to receive acknowledgment. Error code: " << errno << std::endl;
                // You may handle the error and send an appropriate response to the client
                break;
            }
			uint16_t opcode = (buffer[0] << 8) | buffer[1];

			if( opcode != 4)
			{
				//Send error paket
				return;
			}
			if (received >= 4)
            {

                TFTPDataBlock *ackPacket = reinterpret_cast<TFTPDataBlock *>(buffer);

                //Check block
				if( ackPacket->blockNumber = block)
				{
					//SEND ERROR
					std::cout << "zly error blok" << std::endl;
					return;
				}
				writeACK(c_adress, ackPacket->blockNumber);

            }
            else
            {
                // Invalid acknowledgment packet size
                std::cout << "Invalid acknowledgment packet size" << std::endl;
                // You may handle the error and send an appropriate response to the client
                break;
            }
			
		}
		file.close();
		
       
    }

    void handleWRQ()
    {
      		
		TFTPRequest *request = reinterpret_cast<TFTPRequest *>(buffer);


		std::string filepath = root_dirpath + "/" + request->filename;
		fs::path dest_wd{filepath};


		std::ofstream file(dest_wd, std::ios::binary | std::ios::app);

		if(!file.is_open())
		{
			std::cout << "Error opening file for writing" << filepath << std::endl;

			return;
		}

		int blocksize = 0;
		int tsize = 0;
		struct timeval timeout;
		//kontrola co to je za option musi se projit vsechna option

		std::vector<OptionInfo> optionVector;
		int i = 1;
    	for (const auto &opt : request->opts) {
			
			if( !strcmp( opt.option, "blksize"))
			{
				blocksize = atoi(opt.value);
				optionVector.emplace_back("blksize", opt.value, i);

				std::cout << blocksize << std::endl;

			}
			else if( !strcmp( opt.option, "tsize"))
			{
				tsize = atoi(opt.value);
				if(tsize > 65535)
				{
					continue;
				}
				optionVector.emplace_back("tsize", opt.value, i);

				std::cout << tsize << std::endl;
			}
			else if( !strcmp( opt.option, "timeout"))
			{
				timeout.tv_sec = atoi(opt.value);
				optionVector.emplace_back("timeout", opt.value, i);

				std::cout << tsize << std::endl;
			}
			
			i++;
    	}
		writeWRQ(c_adress, request->filename, request->mode, optionVector);
		bool dontWait = true;
		if( !optionVector.empty() )
		{
			dontWait = false;
			sendOAck(optionVector,0, 6);
		}
		else
		{
			dontWait = false;
			sendAck(0, 4);
		}

		size_t block = 1;
		const size_t blockSize = blocksize > 0 ? blocksize : 512;
		socklen_t client_len = sizeof(c_adress);
		if(dontWait)
		{
			ssize_t received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&c_adress, &client_len);
			if( received < 0 )
			{
				std::cout << "Failed to receive" << std::endl;
				return;
			}

			uint16_t clientBlocksize = 512;

			uint16_t opcode = (buffer[0] << 8) | buffer[1];

			if(opcode != 4 )
			{
				//Poslat error paket
				return;
			}
		
			if (received >= 4)
			{
				TFTPDataBlock *datablock = reinterpret_cast<TFTPDataBlock *>(buffer);

				if( ntohs(datablock->opcode) == 3)
				{
					writeDATA(c_adress, serv_port, ntohs(datablock->blockNumber));
					
					file.write(datablock->data, received - 4);

					++block;
					
					sendAck(block,4);

					if( received < blockSize )
					{
						return;
					}

				}
				else
				{
					std::cout << "Unexpected TFTP packet type" << std::endl;
					return;
				}
			}
			else
			{
				//poslat error paket
			}
		}

		while (true)
		{
			ssize_t received = recvfrom(sockfd, buffer, blockSize+4, 0, (struct sockaddr *)&c_adress, &client_len);
			if( received < 0 )
			{
				std::cout << "Failed to receive data, Error: " << errno << std::endl;

				break;
			}

			if( received >= 4 )
			{
				TFTPDataBlock *dataBlock = reinterpret_cast<TFTPDataBlock *>(buffer);

				if (ntohs(dataBlock->opcode) == 3)
				{
					writeDATA(c_adress, serv_port, dataBlock->blockNumber);
					std::cout << dataBlock->data << std::endl;
					
					file.write(dataBlock->data, received - 4);

					++block;

					sendAck(block,4);

					if( received < blockSize )
					{
						return;
					}
				
				}
				else
				{
					std::cout << "Unexpected TFTP packet type" << std::endl;

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
			std::cout << "Failed to send ACK packet. Error code: " << errno << std::endl;

		}

	}

	static bool sortByPosition(const OptionInfo& a, const OptionInfo& b)
	{
		return a.position < b.position;
	}

	void sendOAck(std::vector<OptionInfo> options ,int blocknumber, int opc)
	{
		std::cout << "Sending OACK packet." << std::endl;

		TFTPOACK acknowledment;

		acknowledment.opcode = htons(opc);
		
		if( options.empty() )
		{
			return;
		}
		std::sort(options.begin(), options.end(), sortByPosition);
		int pos = 0;
		for (size_t i = 0; i < std::min(options.size(), size_t(50)); ++i) {

        	strncpy(acknowledment.opts[i].option, options[i].name.c_str(), sizeof(options));
        	strncpy(acknowledment.opts[i].value, options[i].value.c_str(), sizeof(options));
    	}
		for (size_t i = 0; i < std::min(options.size(), size_t(50)); ++i) {
        	std::cout << "Option: " << acknowledment.opts[i].option << ", Value: " << acknowledment.opts[i].value << std::endl;
    	}

		if( sendto(sockfd, &acknowledment, sizeof(acknowledment), 0, (struct sockaddr *)&c_adress, sizeof(c_adress)) < 0)
		{
			std::cout << "Failed to send ACK packet. Error code: " << errno << std::endl;

		}
	}

	int sigintReceived;
	int serv_port;
	int sockfd;
	bool stopRequest;
	sockaddr_in c_adress;
	std::string root_dirpath;
	std::vector<int> clients;
	char buffer[BUFSIZE];


};
