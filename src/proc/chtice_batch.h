#pragma once

#include "common.h"
#include "dbquery.h"
#include "util.h"
#include "https.h"
#include "dbmysql.h"

typedef struct _GlobalVar {
    std::string env_path;
    std::string ini_path;
    DBMYSQL dbmysql;
    std::vector<std::string> g_args;
} GlobalVar;

int mainInitailize(int argc, char* argv[]);
int mainTerminate();
int mainProcess();

// Management API Token
int batchManagementApiToken();

// Sync DB Straemer Info
int batchSyncStreamerInfoALL();

