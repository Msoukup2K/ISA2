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

    /*if (params.host.empty() || params.dest.empty() )
    {
        std::cerr << "Hostname and destination filepath are required arguments." << std::endl;
        exit(1);

    }*/

    fs::path dest_wd{params.dest};
    if(!fs::exists(dest_wd) && !fs::is_directory(dest_wd))
    {
        std::cerr << "Cannot find a dest directory" << std::endl;
        exit(1);
    }

    int sckt; 

    char buffer[100];
    char *message = "Hello Server\0";
    int n;
    struct sockaddr_in s_addr;
    bzero(&s_addr, sizeof(s_addr));
    s_addr.sin_addr.s_addr = inet_addr(params.host.c_str());
    s_addr.sin_port = htons(params.port);
    s_addr.sin_family = AF_INET;

    if ((sckt = socket(AF_INET, SOCK_DGRAM,0)) < 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        exit(1);
    }

    
    std::cout << "C: connecting" << std::endl;

    

        if(sendto(sckt, message, strlen(message), 0, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0)
        {
            perror("S fail");
            std::cerr << "Sending failed";
        }

        socklen_t server_len = sizeof(s_addr);

        if(recvfrom(sckt, buffer, sizeof(buffer), 0, (struct sockaddr *)&s_addr, &server_len) < 0)
        {
            perror("S fail");

            std::cerr << "Faioled to get a respo" << std::endl;

        }

        std::string response(buffer); 

        std::cout << response << std::endl;

    

   /*struct sockaddr_in *sin = (struct sockaddr_in *)&s_addr;

    recvfrom(sckt, buffer, sizeof(buffer), 0, (struct sockaddr*)NULL, NULL);
    puts(buffer);*/
    close(sckt);

    return 0;
}
