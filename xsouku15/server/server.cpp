/*
*      Autor: Martin Soukup
*      Login: xsouku15
*
*/
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

#define BUFSIZE 2048

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
			std::cout << "Error opening socket" << std::endl;
			close(sockfd);
			exit(1);
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

		ssize_t received;
		socklen_t client_len = sizeof(c_adress);
		uint16_t blocksize = 512;
		int tsize = 0;
		struct timeval timeout;
		char optionVal[25];
		std::memset(&timeout, 0, sizeof(timeout));
		//kontrola co to je za option musi se projit vsechna option

		std::vector<OptionInfo> optionVector;
		int i = 1;

    	for (const auto &opt : request->opts) {
			
			if( !strcmp( opt.option, "blksize"))
			{
				if(blocksize > 65464 || blocksize < 8 )
				{
					continue;
				}
				blocksize = atoi(opt.value);
				optionVector.emplace_back("blksize", opt.value, i);

			}
			else if( !strcmp( opt.option, "tsize"))
			{
				tsize = atoi(opt.value);
				if(tsize > 65535)
				{
					sendError("Disk full or allocation exceeded", 3);
					continue;
				}
				optionVector.emplace_back("tsize", opt.value, i);

			}
			else if( !strcmp( opt.option, "timeout"))
			{
				if(atoi(opt.value) <= 0)
				{
					continue;
				}
				timeout.tv_sec = atoi(opt.value);
				
				optionVector.emplace_back("timeout", opt.value, i);
			}
			
			i++;
    	}

		writeRRQ(c_adress, request->filename, request->mode, optionVector);

		std::string filepath = root_dirpath + "/" + request->filename;
		fs::path p{filepath};
		std::ifstream file(filepath.c_str(), std::ios::binary);
		
		if( !file.is_open() )
		{
			std::cout << "Error opening file: " << filepath << std::endl;
			sendError("Access violation",2);
			return;
		}


		if( !optionVector.empty() )
		{
			sendOAck(optionVector,fs::file_size(p), 6);
			received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&c_adress, &client_len);

			if(received < 0 )
			{
				std::cout << "Failed to receive data block" << std::endl;
				return;
			}
			
			uint16_t opcode = (buffer[0] << 8) | buffer[1];

			if( opcode != 4 )
			{
				sendError("Illegal TFTP operation", 4);
				return;
			}
			if(received >= 4 )
			{
				TFTPACK *dataBlock = reinterpret_cast<TFTPACK *>(buffer);

				writeACK(c_adress, dataBlock->blockNumber);
			}
			else
			{
				sendError("Disk full or allocation exceeded", 3);
				return;
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
			std::cout << sizeToSend << std::endl;
			if(sendto(sockfd, &data_block, sizeToSend + 4, 0, (struct sockaddr *)&c_adress, sizeof(c_adress)) < 0)
			{ 
				std::cout << "Failed to send data block: " << std::endl;
				break;
			}

			++block;
			if( timeout.tv_sec != 0 || timeout.tv_usec != 0 )
			{

				fd_set readfds;
				FD_ZERO(&readfds);
				FD_SET(sockfd, &readfds);

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

			}
			if( file.eof() )
			{

				break;

			}

			received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&c_adress, &client_len);
			if (received < 0)
            {
                std::cout << "Failed to receive acknowledgment" << std::endl;
                break;
            }
			uint16_t opcode = (buffer[0] << 8) | buffer[1];
			if( opcode == 5)
			{
				return;
			}
			else if( opcode != 4)
			{
				sendError("Illegal TFTP operation",4);
				break;
			}
			if (received >= 4)
            {

                TFTPDataBlock *ackPacket = reinterpret_cast<TFTPDataBlock *>(buffer);

				writeACK(c_adress, ackPacket->blockNumber);

            }
            else
            {
                
                std::cout << "Invalid acknowledgment packet size" << std::endl;
				sendError("Disk full or allocation exceeded", 3);
                break;
            }

			
		}
		file.close();
		
       
    }

    void handleWRQ()
    {
      		
		TFTPRequest *request = reinterpret_cast<TFTPRequest *>(buffer);

		int blocksize = 512;
		int tsize;
		struct timeval timeout;
		//kontrola co to je za option musi se projit vsechna option

		std::vector<OptionInfo> optionVector;
		int i = 1;

    	for (const auto &opt : request->opts) 
		{

			if( !strcmp( opt.option, "blksize"))
			{
				if(blocksize > 65464 || blocksize < 8 )
				{
					continue;
				}
				blocksize = atoi(opt.value);

				optionVector.emplace_back("blksize", opt.value, i);

			}
			else if( !strcmp( opt.option, "tsize"))
			{
				tsize = atoi(opt.value);
				if(tsize > 65535)
				{
					sendError("Disk full or allocation exceeded", 3);
					continue;
				}
				optionVector.emplace_back("tsize", opt.value, i);

			}
			else if( !strcmp( opt.option, "timeout"))
			{
				if(atoi(opt.value) <= 0)
				{
					continue;
				}
				timeout.tv_sec = atoi(opt.value);
				
				optionVector.emplace_back("timeout", opt.value, i);
			}
			
			i++;
    	}
		writeWRQ(c_adress, request->filename, request->mode, optionVector);
		
		std::string filepath = root_dirpath + "/" + request->filename;
		fs::path dest_wd{filepath};

		std::ifstream fileCheck(dest_wd);
		if(fileCheck.good())
		{
			std::cout << "File already exists" << std::endl;
			sendError("File already exists", 6);
			return;
		}

		std::ofstream file(dest_wd, std::ios::binary | std::ios::app);

		if( !file.is_open() )
		{
			std::cout << "Error opening file: " << filepath << std::endl;
			sendError("Access violation",2);
			return;
		}

		bool dontWait = true;
		if( !optionVector.empty() )
		{
			dontWait = false;
			sendOAck(optionVector, tsize,6);
		}
		else
		{
			dontWait = true;
			sendAck(0, 4);
		}

		size_t block = 1;
		const size_t blockSize = blocksize > 0 ? blocksize : 512;
		socklen_t client_len = sizeof(c_adress);

		if(!dontWait)
		{
			ssize_t received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&c_adress, &client_len);
			if( received < 0 )
			{
				std::cout << "Failed to receive" << std::endl;
				return;
			}

			uint16_t opcode = (buffer[0] << 8) | buffer[1];

			if( opcode == 5)
			{
                TFTPERROR *responseBlock = reinterpret_cast<TFTPERROR *>(buffer);

				writeERROR(c_adress, serv_port, responseBlock->errorcode, responseBlock->message);
				return;
			}
			else if( opcode != 3 )
			{
				sendError("Illegal TFTP operation", 4);
				return;
			}
			
			if (received >= 4)
			{
				TFTPDataBlock *datablock = reinterpret_cast<TFTPDataBlock *>(buffer);

				writeDATA(c_adress, serv_port, datablock->blockNumber);
				
				file.write(datablock->data, received - 4);
				sendAck(block,4);
				++block;

				if( received < blockSize )
				{

					return;
				}

			}
			else
			{
				sendError("Disk full or allocation exceeded", 3);
				return;
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

			uint16_t opcode = (buffer[0] << 8) | buffer[1];
			
			if(opcode != 3 )
			{
				sendError("Illegal TFTP operation",4);
				return;
			}

			if( received >= 4 )
			{
				TFTPDataBlock *dataBlock = reinterpret_cast<TFTPDataBlock *>(buffer);

					
				writeDATA(c_adress, serv_port, dataBlock->blockNumber);
				
				file.write(dataBlock->data, received - 4);

				sendAck(block,4);

				++block;

				if( received < blockSize )
				{
					return;
				}
				
				
			}
			else
			{
				sendError("Disk full or allocation exceeded", 3);
				return;
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
			std::cout << "Failed to send ACK packet" << std::endl;

		}

	}

	static bool sortByPosition(const OptionInfo& a, const OptionInfo& b)
	{
		return a.position < b.position;
	}

	void sendOAck(std::vector<OptionInfo> options, size_t file_size ,int opc)
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
			if(options[i].name == "tsize")
			{

				strncpy(acknowledment.opts[i].option, options[i].name.c_str(), sizeof(options));
        		strncpy(acknowledment.opts[i].value, std::to_string(file_size).c_str(), sizeof(options));
			}
			else
			{
				strncpy(acknowledment.opts[i].option, options[i].name.c_str(), sizeof(options));
        		strncpy(acknowledment.opts[i].value, options[i].value.c_str(), sizeof(options));
			}
        	
    	}
		for (size_t i = 0; i < std::min(options.size(), size_t(50)); ++i) {
        	std::cout << "Option: " << acknowledment.opts[i].option << ", Value: " << acknowledment.opts[i].value << std::endl;
    	}

		if( sendto(sockfd, &acknowledment, sizeof(acknowledment), 0, (struct sockaddr *)&c_adress, sizeof(c_adress)) < 0)
		{
			std::cout << "Failed to send OACK packet" << std::endl;

		}
	}



	void sendError(std::string message ,int errorCode)
	{
		std::cout << "Sending Error packet" << std::endl;

		TFTPERROR errorPacket;

		errorPacket.opcode = htons(5);
		errorPacket.message = message + '\0';
		errorPacket.errorcode = htons(errorCode);

		if( sendto(sockfd, &errorPacket, sizeof(errorPacket), 0, (struct sockaddr *)&c_adress, sizeof(c_adress)) < 0)
		{
			std::cout << "Failed to send Error packet." << std::endl;
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
