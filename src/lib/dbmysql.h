#pragma once

#include <string>
#include <mysql/mysql.h>

#include "util.h"

typedef struct _DBMYSQL {
    std::string env_path;
    std::string env_file;
    std::string ini_path;
    std::string ini_file;
    std::string db_host;
    std::string db_user;
    std::string db_pass;
    std::string db_name;
    int         db_port;
    MYSQL* mysql;
} DBMYSQL;

int dbmysqlSetEnvFile(DBMYSQL& dbmysql, const std::string& path, const std::string& file);
int dbmysqlSetIniFile(DBMYSQL& dbmysql, const std::string& path, const std::string& file);
int dbmysqlConnect(DBMYSQL& dbmysql);
int dbmysqlClose(MYSQL* mysql);
int dbmysqlQueryExecute(MYSQL* mysql, const std::string& query, int qlen);
MYSQL_RES* dbmysqlQueryResult(MYSQL* mysql);
MYSQL_ROW dbmysqlResultFetch(MYSQL_RES* result);
int dbmysqlResultFinish(MYSQL_RES* result);
int dbmysqlSelectGetRowsCount(MYSQL_RES* result);

