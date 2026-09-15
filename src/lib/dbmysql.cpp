#include "dbmysql.h"


int dbmysqlSetEnvFile(DBMYSQL& dbmysql, const std::string& path, const std::string& file)
{
    dbmysql.env_path = path;
    dbmysql.env_file = file;
    return 0;
}

int dbmysqlSetIniFile(DBMYSQL& dbmysql, const std::string& path, const std::string& file)
{
    dbmysql.ini_path = path;
    dbmysql.ini_file = file;
    return 0;
}

int dbmysqlConnect(DBMYSQL& dbmysql)
{
    // MySQL 연결 초기화
    dbmysql.mysql = mysql_init(NULL);
    if(dbmysql.mysql == NULL) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "mysql_init() failed");
        return EXIT_FAILURE;
    }

    std::string host = utilGetIniVar(dbmysql.ini_path, dbmysql.ini_file, "MYSQL", "HOST");
    dbmysql.db_host = host.empty() ? "127.0.0.1" : host;

    std::string port = utilGetIniVar(dbmysql.ini_path, dbmysql.ini_file, "MYSQL", "PORT");
    dbmysql.db_port = port.empty() ? 3306 : stoi(port);

    dbmysql.db_user = utilGetIniVar(dbmysql.ini_path, dbmysql.ini_file, "MYSQL", "USER");
    if(dbmysql.db_user.empty()) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "db_user empty.[%s]", dbmysql.db_user.c_str());
        mysql_close(dbmysql.mysql);
        return EXIT_FAILURE;
    }

    dbmysql.db_pass = utilGetEnvVar(dbmysql.env_path, dbmysql.env_file, "DB_PASSWORD");
    if(dbmysql.db_pass.empty()) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "db_pass empty.[%s]", dbmysql.db_pass.c_str());
        mysql_close(dbmysql.mysql);
        return EXIT_FAILURE;
    }

    dbmysql.db_name = utilGetIniVar(dbmysql.ini_path, dbmysql.ini_file, "MYSQL", "NAME");
    if(dbmysql.db_name.empty()) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "db_name empty.[%s]", dbmysql.db_name.c_str());
        mysql_close(dbmysql.mysql);
        return EXIT_FAILURE;
    }

    // MySQL 서버 연결
    if(mysql_real_connect(dbmysql.mysql, dbmysql.db_host.c_str(), dbmysql.db_user.c_str(),
                          dbmysql.db_pass.c_str(), dbmysql.db_name.c_str(), 0, NULL, 0) == NULL
    ) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "mysql_real_connect() failed.: %s", mysql_error(dbmysql.mysql));
        mysql_close(dbmysql.mysql);
        return EXIT_FAILURE;
    }

    return 0;
}

int dbmysqlClose(MYSQL* mysql)
{
    mysql_close(mysql);
    return 0;
}

int dbmysqlQueryExecute(MYSQL* mysql, const std::string& query, int qlen)
{
    UTILLOG(LOGLV_DBG, __FUNCTION__, "dbmysql query[%d:%s]", qlen, query.c_str());

    if(mysql_query(mysql, query.c_str())) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "dbmysql query failed[%s]", mysql_error(mysql));
        return EXIT_FAILURE;
    }

    return 0;
}

MYSQL_RES* dbmysqlQueryResult(MYSQL* mysql)
{
    return mysql_store_result(mysql);
}

MYSQL_ROW dbmysqlResultFetch(MYSQL_RES* result)
{
    return mysql_fetch_row(result);
}

int dbmysqlResultFinish(MYSQL_RES* result)
{
    mysql_free_result(result);
    return 0;
}

int dbmysqlSelectGetResultCount(MYSQL_RES* result)
{
    return (int)mysql_num_rows(result);
}

