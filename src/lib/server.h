#pragma once

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "util.h"

#define CLIENT_MAX_CNT 1024

#define CLOSE_MASK_SERVER  1 // 001
#define CLOSE_MASK_CLIENT  2 // 010
#define CLOSE_MASK_CLIENTS 4 // 100

typedef struct _SERVER {
    int server_sd;
    int client_sd;
    int listen_port;
    int flag;
    int is_plexing;
    
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    
    int clients_sd[CLIENT_MAX_CNT];
} SERVER;

int serverListen(SERVER* server);
int serverAccept(SERVER* server);
int serverClose(SERVER* server, int close_mask);

