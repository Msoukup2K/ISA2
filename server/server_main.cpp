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

    std::string interface{"0.0.0.0"};

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(params.port);

    if(inet_pton(AF_INET, interface.c_str(), &addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid interface" << std::endl;
        exit(1);
    }

    socklen_t addr_len = sizeof(addr);
    int welcome_socket; 
    welcome_socket = socket(AF_INET, SOCK_STREAM, 0);
    int run = 1;
    while(run == 1)
    {
        std::cout << "S: listening" << std::endl;

        int c_socket = accept(welcome_socket, (struct sockaddr*)&addr, &addr_len);
        if(run)
        {
            if(c_socket < 0)
            {
                std::cerr << "Accepting request has failed" << std::endl;

            }

        } 
    }

    return 0;
}
