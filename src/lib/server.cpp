#include "server.h"

int serverListen(SERVER* server)
{
    if(server == NULL) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "server is NULL");
        return -1;
    }

    if(server->server_sd <= 0) {
        server->server_sd = socket(AF_INET, SOCK_STREAM, 0);
        if(server->server_sd < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "socket create failed.");
            return EXIT_FAILURE;
        }

        server->flag = fcntl(server->server_sd, F_GETFL, 0);

        server->server_addr.sin_family      = AF_INET;
        server->server_addr.sin_addr.s_addr = INADDR_ANY;
        server->server_addr.sin_port        = htons(server->listen_port);

        socklen_t addr_len = sizeof(struct sockaddr);
        if(bind(server->server_sd, (struct sockaddr*)&server->server_addr, addr_len) < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "Bind failed.");
            serverClose(server, 4);
            return EXIT_FAILURE;
        }

        if(listen(server->server_sd, CLIENT_MAX_CNT) < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "Listen failed.");
            serverClose(server, 4);
            return EXIT_FAILURE;
        }
    }
    else {
        UTILLOG(LOGLV_NOR, __FUNCTION__, "Server socket discriptor is already[%d]", server->server_sd);
    }

    UTILLOG(LOGLV_NOR, __FUNCTION__, "Server is listening on port %d...\n", server->listen_port);

    return server->server_sd;
}

int serverAccept(SERVER* server)
{
    if(server == NULL) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "server is NULL");
        return -1;
    }

    socklen_t addr_len = sizeof(struct sockaddr);
    server->client_sd = accept(server->server_sd, (struct sockaddr*)&server->client_addr, &addr_len);
    if(server->client_sd < 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "Accept failed.");
        serverClose(server, 3);
        return EXIT_FAILURE;
    }

    UTILLOG(LOGLV_NOR, __FUNCTION__, "[C:%d]Client Accept\n", server->client_sd);

    if(server->is_plexing == true) {
        int pos = 0;
        for(pos = 0; pos < CLIENT_MAX_CNT; pos++) {
            if(server->clients_sd[pos] > 0) {
                continue;
            }

            server->clients_sd[pos] = server->client_sd;
            break;
        }
    }

    return server->client_sd;
}

int serverClose(SERVER* server, int close_mask)
{
    if(server == NULL) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "server is NULL");
        return -1;
    }

    if(close_mask < 0 || close_mask > 7) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "close_mask is not valid\n");
        return -1;
    }

    if(close_mask & CLOSE_MASK_SERVER) {
        if(server->server_sd > 0) {
            close(server->server_sd);
            server->server_sd = -1;
        }
    }

    if(close_mask & CLOSE_MASK_CLIENT) {
        if(server->client_sd > 0) {
            close(server->client_sd);

            if(server->is_plexing == true) {
                int pos = 0;
                for(pos = 0; pos < CLIENT_MAX_CNT; pos++) {
                    if(server->clients_sd[pos] != server->client_sd) {
                        continue;
                    }

                    server->clients_sd[pos] = -1;
                    break;
                }
            }

            server->client_sd = -1;
        }
    }

    if(close_mask & CLOSE_MASK_CLIENTS) {
        int pos = 0;
        for(pos = 0; pos < CLIENT_MAX_CNT; pos++) {
            if(server->clients_sd[pos] <= 0) { 
                continue;
            }

            close(server->clients_sd[pos]);
            server->clients_sd[pos] = -1;
        }
    }

    return 0;
}
