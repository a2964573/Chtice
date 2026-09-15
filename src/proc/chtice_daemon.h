#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <thread>

#include "common.h"
#include "dbquery.h"
#include "https.h"

typedef struct _GlobalVar {
    std::atomic_bool is_running;
    std::string env_path;
    std::string ini_path;
    DBMYSQL dbmysql;
} GlobalVar;

int mainInitailize();
int mainTerminate();
int mainProcess();
int pollLiveStatus(std::vector<STREAMER_INFO>& list);
int chticeGetLiveStatus(STREAMER_INFO& org_info, STREAMER_INFO& cur_info);

