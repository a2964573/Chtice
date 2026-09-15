#pragma once

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "util.h"

typedef struct _CLIENT {
    int client_sd;
    int flag;
    int is_nonblocking;
    int server_port;
    char server_ip[64];
    struct sockaddr_in server_addr;
} CLIENT;

int clientConnect(CLIENT* client);
int clientClose(CLIENT* client);

