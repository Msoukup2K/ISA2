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
                std::cerr << "Usage for Server: tftp-server [-p port] root_dirpath." << std::endl;
                return 1;
        }
    }

    if (optind < argc)
    {
        params.root = argv[optind];
    }
    else
    {
        std::cerr << "Root directory path is required argument." << std::endl;
        return 1;
    }

    fs::path root_path{params.root};
    if (!fs::exists(root_path) && !fs::is_directory(root_path))
    {
        std::cerr << "Root directory does not exist." << std::endl;
        return 1; 
    }

    

    return 0;
}
