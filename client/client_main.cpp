#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <err.h>
#include <string>
#include <csignal>
#include <iostream>
#include <filesystem>
#include <cstdint>
#include <cstring>
#include <unistd.h>

#include "../utils/utils.hpp"


namespace fs = std::filesystem;

int main( int argc, char *argv[] )
{

    /*struct sigaction interrupt_handle;

    interrupt_handle.sa_flags = on_close;

    sigemptyset(&interrupt_handle.sa_mask);*/

    struct c_parameters params;

    params.port = 69; //Default TFTP port;

    int opt;

    while((opt = getopt(argc, argv, "h:p:f:t:")) != -1)
    {
        switch (opt) {
            case 'h':
                params.host = optarg;
                break;

            case 'p':
                params.port = std::stoi(optarg);
                break;
            
            case 'f':
                params.filepath = optarg;
                break;

            case 't':
                params.dest = optarg;
                break;
            
            default:
                std::cerr << "Usage for Client: tftp-client -h hostname [-p port] [-f filename] -t dest_filepath";
                exit(1);
        }
    }

    if (params.host.empty() || params.dest.empty() )
    {
        std::cerr << "Hostname and destination filepath are required arguments." << std::endl;
        exit(1);

    }

    fs::path dest_wd{params.dest};

    int sckt; 

    char buffer[100];
    int n;
    struct sockaddr_in s_addr;
    bzero(&s_addr, sizeof(s_addr));
    s_addr.sin_addr.s_addr = inet_addr(params.host.c_str());
    s_addr.sin_port = htons(params.port);
    s_addr.sin_family = AF_INET;

    TFTPRequest request;

    if( params.filepath.empty() )
    {
        request.opcode = htons(2); //WRQ opcode

        strncpy(request.filename, params.dest.c_str(), sizeof(request.filename));
        strncpy(request.mode, "octet", sizeof(request.mode));
        strncpy(request.options, "blksize=1024", sizeof(request.options));

    }
    else
    {
        request.opcode = htons(1); //RRQ opcode

        strncpy(request.filename, "example.txt", sizeof(request.filename));
        strncpy(request.mode, "octet", sizeof(request.mode));
        strncpy(request.options, "blksize=1024", sizeof(request.options));
    }

    if ((sckt = socket(AF_INET, SOCK_DGRAM,0)) < 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        exit(1);
    }

    
    std::cout << "C: connecting" << std::endl;

    

    if(sendto(sckt, &request, sizeof(request), 0, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0)
    {
        perror("S fail");
        std::cerr << "Sending failed";
        close(sckt);
        exit(1);
    }

    socklen_t server_len = sizeof(s_addr);


    if( params.filepath.empty() )
    {

        ssize_t received = recvfrom(sckt, buffer, sizeof(buffer), 0, (struct sockaddr *)&s_addr, &server_len);

        if( received < 0)
        {
            perror("S fail");

            std::cerr << "Faioled to get a respo" << std::endl;

        }

        if( received >= 4)
        {
            TFTPDataBlock *responseBlock = reinterpret_cast<TFTPDataBlock *>(buffer);

            if (ntohs(responseBlock->opcode) == 4)
            {
                uint16_t rblock = ntohs(responseBlock->blockNumber);

                std::cout << "Received data Block #" << rblock << std::endl;
                std::cout << "Received data: " << responseBlock->data << std::endl;
            }
            else
            {
                std::cerr << "Unexpected TFTP packet" << std::endl;

            }
        }
        size_t block = 1;
        //WRQ
        while(true)
        {
            ssize_t bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer) - 4);
            if( bytesRead < 0 )
            {
                perror("Erorr reading from stdin");
                std::cerr << "Failed to read from stdin" << std::endl;

                break;
            } 

            TFTPDataBlock dataBlock;
            dataBlock.opcode = htons(3); //Data opcode
            dataBlock.blockNumber = htons(block);
            memcpy(dataBlock.data, buffer, bytesRead);

            if( sendto(sckt, &dataBlock, bytesRead + 4, 0, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0)
            {
                perror("Error sending data block");
                std::cerr << "Failed to send data" << std::endl;
                break;
            }

            ++block;

        }

    }
    else
    {
        //RRQ
            ssize_t received = recvfrom(sckt, buffer, sizeof(buffer), 0, (struct sockaddr *)&s_addr, &server_len);

            if( received < 0)
            {
                perror("S fail");

                std::cerr << "Faioled to get a respo" << std::endl;

            }

            if( received >= 4)
            {
                TFTPDataBlock *dataBlock = reinterpret_cast<TFTPDataBlock *>(buffer);

                if (ntohs(dataBlock->opcode) == 3)
                {
                    uint16_t block = ntohs(dataBlock->blockNumber);

                    std::cout << "Received data Block #" << block << std::endl;
                    std::cout << "Received data: " << dataBlock->data << std::endl;
                }
                else
                {
                    std::cerr << "Unexpected TFTP packet" << std::endl;

                }
            }
    }


    close(sckt);

    return 0;
}
