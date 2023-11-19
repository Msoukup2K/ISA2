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
#include <vector>


#ifndef UTILS_H
#define UTILS_H

struct TFTPDataBlock
{
    uint16_t opcode;
    uint16_t blockNumber;
    char data[512];
};

struct OptionInfo
{
    std::string name;
    std::string value;
    size_t position;

    OptionInfo(const std::string& n,const std::string& v, size_t p) : name(n),
    value(v), position(p) {}
};

struct options
{
    char option[50];
    char value[50];
};

struct TFTPRequest
{
    uint16_t opcode;
    char filename[100];
    char mode[10];
    options opts[50];
};

struct TFTPOACK
{
    uint16_t opcode;
    options opts[50];
};

struct TFTPACK
{
    uint16_t opcode;
    uint16_t blockNumber;
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