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

		for( int i = 0; i<100;i++)
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

			
			struct sockaddr_in *sin = (struct sockaddr_in *)&c_adress;

            buffer[n] = '\0';
            puts(buffer);	
            fprintf(stdout, "Incoming request from %s:%d\n", inet_ntoa(sin->sin_addr), sin->sin_port);


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
			fprintf(stdout, "Incoming request from %s:%d with opcode %d\n", inet_ntoa(sin->sin_addr), sin->sin_port, opcode);
            break;
        case 2:
            // Write request (WRQ)
            //handleWRQ();
			fprintf(stdout, "Incoming request from %s:%d with opcode %d\n", inet_ntoa(sin->sin_addr), sin->sin_port, opcode);
            break;
        // Add more cases as needed for other TFTP opcodes
        default:
            // Unsupported opcode
            break;
        }
    }

    void handleRRQ()
    {
        // Extract filename from the TFTP request
        std::string filename(&buffer[2]);
		
		std::string filepath = root_dirpath + "/" + filename;
        std::ifstream file(filepath.c_str(), std::ios::binary);
		if ( !file.is_open() )
		{
			std::cerr << "Error opening file: " << filepath << std::endl;
			return;
		}

        std::vector<char> file_buffer(std::istreambuf_iterator<char>(file), {});

		file.close();
		
		size_t num_blocks = (file_buffer. size() + 511) / 512 ;
		std::cout << num_blocks << std::endl;

		for( size_t block = 1; block <= num_blocks; ++block)
		{
			size_t start_pos = (block - 1) * 512;
            size_t end_pos = std::min(start_pos + 512, file_buffer.size());

            TFTPDataBlock data_block;
			data_block.opcode = htons(3);
			data_block.blockNumber = htons(block);

            // Copy the file content into the data block
            std::copy(file_buffer.begin() + start_pos, file_buffer.begin() + end_pos, data_block.data);

            // Send the data block to the client
           	if(sendto(sockfd, &data_block, sizeof(data_block), 0, (struct sockaddr *)&c_adress, sizeof(c_adress)) < 0)
			{
				perror("Sending dataFile");
				std::cerr << "Failed to send data block. Error code: " << errno << std::endl;
			}

			//sendto(sockfd, data_block.data(), data_block.size(), 0, (struct sockaddr *)&c_adress, sizeof(c_adress));
		}

       
    }

    void handleWRQ()
    {
        // Extract filename from the TFTP request
        std::string filename(&buffer[2]);

        // TODO: Implement logic to receive and write the file in response to WRQ
        // You can use the filename and root_dirpath to construct the file path
    }

	int serv_port;
	int sockfd;
	sockaddr_in c_adress;
	const std::string root_dirpath;
	std::vector<int> clients;
	bool stopRequest;
	char buffer[100];


};
