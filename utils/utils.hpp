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

struct TFTPDataBlock
{
    uint16_t opcode;
    uint16_t blockNumber;
    char data[512];
};

struct TFTPRequest
{
    uint16_t opcode;
    char filename[100];
    char mode[10];
    char options[50];
    char optionValue[50];
};

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