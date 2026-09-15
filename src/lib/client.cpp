#include "client.h"

int clientConnect(CLIENT* client)
{
    if(client == NULL) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "client is NULL");
        return -1;
    }

    if(client->client_sd <= 0) {
        client->client_sd = socket(AF_INET, SOCK_STREAM, 0);
        if(client->client_sd < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "socket create failed: %s", strerror(errno));
            return EXIT_FAILURE;
        }

        client->flag = fcntl(client->client_sd, F_GETFL, 0);
        if(client->is_nonblocking == true) {
            client->flag |= O_NONBLOCK;
        }
        else {
            client->flag &= ~O_NONBLOCK;
        }
        fcntl(client->client_sd, F_SETFL, client->flag);

        client->server_addr.sin_family = AF_INET;
        client->server_addr.sin_port   = htons(client->server_port);

        int rtn = inet_pton(AF_INET, client->server_ip, &client->server_addr.sin_addr);
        if(rtn <= 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "inet_pton failed: Invalid IP address");
            clientClose(client);
            return rtn;
        }

        socklen_t addr_len = sizeof(struct sockaddr);
        rtn = connect(client->client_sd, (struct sockaddr*)&client->server_addr, addr_len);
        if(rtn < 0) {
            if((client->is_nonblocking == true) && (errno == EINPROGRESS)) {
                int so_error;
                socklen_t len = sizeof(so_error);
                if(getsockopt(client->client_sd, SOL_SOCKET, SO_ERROR, &so_error, &len) == 0 && so_error == 0) {
                    UTILLOG(LOGLV_NOR, __FUNCTION__, "Connect Success.[%d]", client->client_sd);
                }
                else {
                    UTILLOG(LOGLV_ERR, __FUNCTION__, "Connect Failed.[%d] (so_error: %d)", rtn, so_error);
                    clientClose(client);
                }
            }
            else {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "Connect Failed.[%d] (errno: %d, %s)", rtn, errno, strerror(errno));
                clientClose(client);
            }
        }
        else {
            UTILLOG(LOGLV_NOR, __FUNCTION__, "Connect Success.[%d]", client->client_sd);
        }
    }
    else {
        UTILLOG(LOGLV_NOR, __FUNCTION__, "Client socket discriptor is already[%d]", client->client_sd);
    }

    return client->client_sd;
}

int clientClose(CLIENT* client)
{
    if(client == NULL) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "client is NULL");
        return -1;
    }

    if(client->client_sd > 0) {
        close(client->client_sd);
        client->client_sd = -1;
    }

    return 0;
}
