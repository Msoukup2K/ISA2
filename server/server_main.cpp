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
#include "server.cpp"

namespace fs = std::filesystem;

#define MAXLINE 1000


int main( int argc, char *argv[] )
{

    struct s_parameters params;
    

    params.port = 69; //Default TFTP port;

    int opt;

    while((opt = getopt(argc, argv, "p:")) != -1)
    {
        switch (opt) {
        
            case 'p':
                params.port = std::stoi(optarg);
                break;
        
            default:
                std::cout << "Usage for Server: tftp-server [-p port] root_dirpath." << std::endl;
                return 1;
        }
    }

    if (optind < argc)
    {
        params.root = argv[optind];
    }
    else
    {
        std::cout << "Root directory path is required argument." << std::endl;
        return 1;
    }

    if (!fs::exists(params.root) && !fs::is_directory(params.root))
    {
        std::cout << "Root directory does not exist." << std::endl;
        return 1; 
    }

    fs::path root_path{params.root};

    Server server(params.port, params.root);
    server.Start();
    
    
}
