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


#ifndef UTILS_H
#define UTILS_H



struct c_parameters{
    std::string host;
    int port;
    std::string filepath;
    std::string dest;
};

struct s_parameters{
    int port;
    std::string root;
};


#endif