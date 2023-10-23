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
                return 1;
        }
    }

    if (params.host.empty() || params.dest.empty() )
    {
        std::cerr << "Hostname and destination filepath are required arguments." << std::endl;
        return 1;
    }

    fs::path dest_wd{params.dest};
    if(!fs::exists(dest_wd) && !fs::is_directory(dest_wd))
    {
        exit(2);
    }

    int socket;

    struct sockaddr_in s_addr;
    s_addr.sin_family = AF_INET;



    return 0;
}
