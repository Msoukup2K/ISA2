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
    if(!fs::exists(dest_wd) && !fs::is_directory(dest_wd))
    {
        std::cerr << "Cannot find a dest directory" << std::endl;
        exit(1);
    }

    int sckt;

    struct sockaddr_in s_addr;
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(params.port);

    socklen_t addr_len{sizeof(s_addr)};

    if ((sckt = socket(AF_INET, SOCK_STREAM,0)) < 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        exit(1);
    }
    if (inet_pton(AF_INET, params.host.c_str(), &s_addr.sin_addr) <= 0 )
    {
        std::cerr << "Invalid host" << std::endl;
        exit(1);
    }
    if (connect(sckt, (struct sockaddr *)&s_addr, addr_len) < 0 )
    {
        std::cerr << "Cannost establish a connection" << std::endl;
        exit(1);
    };

    std::cout << "C: connecting" << std::endl;

    


    return 0;
}
