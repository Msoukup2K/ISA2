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
#include <fstream>

#include "../utils/utils.hpp"
#include "../utils/netasciiparser.hpp"
#define BUFSIZE 1024


namespace fs = std::filesystem;


void sendAck(int sockfd, sockaddr_in adress ,uint16_t block,int opc)
{
    std::cout << "Sending ACK packet." << std::endl;

    TFTPACK acknowledment;

    acknowledment.opcode = htons(opc);
    acknowledment.blockNumber = htons(block);

    if( sendto(sockfd, &acknowledment, sizeof(acknowledment), 0, (struct sockaddr *)&adress, sizeof(adress)) < 0)
    {
        perror("Error sending ACK packet");
        std::cerr << "Failed to send ACK packet. Error code: " << errno << std::endl;

    }

}

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

    char buffer[BUFSIZE];
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
        strncpy(request.opts->option, "blksize\0", sizeof(request.opts->option));
        strncpy(request.opts->value, "1024\0", sizeof(request.opts->value));

    }
    else
    {
        request.opcode = htons(1); //RRQ opcode

        strncpy(request.filename, params.filepath.c_str() + '\0', sizeof(request.filename));
        strncpy(request.mode, "octet\0", sizeof(request.mode));
        //strncpy(request.opts->option, "tsize\0", sizeof(request.opts->option));
        //strncpy(request.opts->value, "65536\0", sizeof(request.opts->value));
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

    int blksizeOption = 512;
    int timeoutOption = 0;
    int tsizeOption = 0;
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
        int opts = 1;
            // TODO TADY bude jeste kontrola jessli prijdou nejake options
            //RRQ
            if( opts == 0 )
            {
                ssize_t received = recvfrom(sckt, buffer, sizeof(buffer), 0, (struct sockaddr *)&s_addr, &server_len);
                if( received < 0)
                {
                    perror("S fail");

                    std::cerr << "Faioled to get a respo" << std::endl;

                }
                size_t block = 0;
                
                if( received >= 4)
                {
                    TFTPOACK *responseBlock = reinterpret_cast<TFTPOACK *>(buffer);

                    if (ntohs(responseBlock->opcode) == 6)
                    {

                        std::cout << "Received OACK" << std::endl;

                        for (const auto &opt : responseBlock->opts) 
                        {
                            
                            if( !strcmp( opt.option, "blksize"))
                            {
                                blksizeOption = atoi(opt.value);
                            }
                            else if( !strcmp( opt.option, "tsize"))
                            {
                                tsizeOption = atoi(opt.value);
                                
                            }
                            else if( !strcmp( opt.option, "timeout"))
                            {
                                timeoutOption = atoi(opt.value);
                            }

                        }

                        sendAck(sckt, s_addr, block, 4);

                    }
                    else
                    {
                        std::cout << "ERROR musí byt negotiated blocksize" << std::endl;
                        exit(1);
                    }
                    
                }
            }

            while(true)
            {
                ssize_t received = recvfrom(sckt, buffer, sizeof(buffer), 0, (struct sockaddr *)&s_addr, &server_len);
                std::cout << "Received bytes: " << received << std::endl;

                if( received >= 4)
                {
                    TFTPDataBlock *dataBlock = reinterpret_cast<TFTPDataBlock *>(buffer);
                    if (ntohs(dataBlock->opcode) == 3)
                    {
                        uint16_t block = ntohs(dataBlock->blockNumber);
                        //char *responseData = convertFromNetascii(dataBlock->data);

                        std::cout << "Received data Block #" << block << std::endl;
                        std::cout << "Received bytes: " << dataBlock->data << std::endl;
                        
                        std::ofstream destFile(dest_wd.c_str(), std::ios::binary | std::ios::app);

                        if (!destFile.is_open()) {
                            std::cerr << "Error opening destination file: " << dest_wd << std::endl;
                            // Handle the error as needed
                            break;
                        }

                        // Write the received data to the destination file
                        destFile.write(dataBlock->data, received - 4); // Exclude the 4 bytes of the TFTP header

                        // Close the destination file
                        destFile.close();
                        
                        sendAck(sckt, s_addr, ++block, 4);

                        if( received < blksizeOption )
                        {
                            break;
                        }

                    }
                    else
                    {
                        std::cerr << "Unexpected TFTP packet" << std::endl;

                    }
                }
            }
        
    }

    close(sckt);

    return 0;
}
