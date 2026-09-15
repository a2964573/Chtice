#include "dbquery.h"


int _queryEscapeString(MYSQL* mysql, const std::string& input, std::string& output) {
    if(input.empty()) {
        return -1;
    }

    std::vector<char> buffer(input.length() * 2 + 1);
    mysql_real_escape_string(mysql, buffer.data(), input.c_str(), input.length());

    output = std::string(buffer.data());
    return output.length();
}

// streamer_info //////////////////////////////////////////////////////////////
int querySelectStreamerInfoByStreamingChannelId(MYSQL* mysql, const std::string& streaming_channel_id, STREAMER_INFO& info)
{
    std::string query;
    int qlen;

    std::string esc_streaming_channel_id;
    if(_queryEscapeString(mysql, streaming_channel_id, esc_streaming_channel_id) < 0) {
        return -1;
    }

    query = "SELECT " DBQUERY_STREAMER_INFO_SELECT_COLUMN "FROM streamer_info stm "
                "WHERE streaming_channel_id = '" + esc_streaming_channel_id + "' ";
    qlen  = query.length();

    if(dbmysqlQueryExecute(mysql, query, qlen) != 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "query execute error[%d:%s]", qlen, query.c_str());
        return -1;
    }

    MYSQL_RES* result = dbmysqlQueryResult(mysql);
    if(result == NULL) {
        return -1;
    }

    int count = 0;
    MYSQL_ROW row = dbmysqlResultFetch(result);
    if(row != NULL) {
        if(row[0 ] != NULL) info.streaming_channel_id  = row[0 ];
        if(row[1 ] != NULL) info.streaming_channel_url = row[1 ];
        if(row[2 ] != NULL) info.streamer_name         = row[2 ];
        if(row[3 ] != NULL) info.streamer_profile      = row[3 ];
        if(row[4 ] != NULL) info.streaming_title       = row[4 ];
        if(row[5 ] != NULL) info.streaming_category    = row[5 ];
        if(row[6 ] != NULL) info.streaming_status      = row[6 ][0];
        if(row[7 ] != NULL) info.open_time             = row[7 ];
        if(row[8 ] != NULL) info.close_time            = row[8 ];
        if(row[9 ] != NULL) info.regist_time           = row[9 ];
        if(row[10] != NULL) info.w_time                = row[10];

        count++;
    }
    dbmysqlResultFinish(result);

    return count;
}

int querySelectStreamerInfoByStreamerName(MYSQL* mysql, const std::string& streamer_name, STREAMER_INFO& info)
{
    std::string query;
    int qlen;

    std::string esc_streamer_name;
    if(_queryEscapeString(mysql, streamer_name, esc_streamer_name) < 0) {
        return -1;
    }

    query = "SELECT " DBQUERY_STREAMER_INFO_SELECT_COLUMN
                ", (CASE "
                    "WHEN stm.streamer_name = '" + esc_streamer_name + "' THEN 1 "
                    "WHEN stm.streamer_name LIKE '" + esc_streamer_name + "%' THEN 2 "
                    "ELSE 3 "
                "END) AS match_priority "
                "FROM streamer_info stm "
                "WHERE stm.streamer_name LIKE '%" + esc_streamer_name + "%' "
                "ORDER BY match_priority ASC, LENGTH(stm.streamer_name) ASC "
                "LIMIT 1 ";

    qlen  = query.length();

    if(dbmysqlQueryExecute(mysql, query, qlen) != 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "query execute error[%d:%s]", qlen, query.c_str());
        return -1;
    }

    MYSQL_RES* result = dbmysqlQueryResult(mysql);
    if(result == NULL) {
        return -1;
    }

    int count = 0;
    MYSQL_ROW row = dbmysqlResultFetch(result);
    if(row != NULL) {
        if(row[0 ] != NULL) info.streaming_channel_id  = row[0 ];
        if(row[1 ] != NULL) info.streaming_channel_url = row[1 ];
        if(row[2 ] != NULL) info.streamer_name         = row[2 ];
        if(row[3 ] != NULL) info.streamer_profile      = row[3 ];
        if(row[4 ] != NULL) info.streaming_title       = row[4 ];
        if(row[5 ] != NULL) info.streaming_category    = row[5 ];
        if(row[6 ] != NULL) info.streaming_status      = row[6 ][0];
        if(row[7 ] != NULL) info.open_time             = row[7 ];
        if(row[8 ] != NULL) info.close_time            = row[8 ];
        if(row[9 ] != NULL) info.regist_time           = row[9 ];
        if(row[10] != NULL) info.w_time                = row[10];

        count++;
    }
    dbmysqlResultFinish(result);

    return count;
}

int querySelectStreamerInfoAll(MYSQL* mysql, std::vector<STREAMER_INFO>& list)
{
    std::string query;
    int qlen;

    query = "SELECT " DBQUERY_STREAMER_INFO_SELECT_COLUMN "FROM streamer_info stm ";
    qlen  = query.length();

    if(dbmysqlQueryExecute(mysql, query, qlen) != 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "query execute error[%d:%s]", qlen, query.c_str());
        return -1;
    }

    MYSQL_RES* result = dbmysqlQueryResult(mysql);
    if(result == NULL) {
        return -1;
    }

    MYSQL_ROW row;
    STREAMER_INFO info;
    while(true) {
        row = dbmysqlResultFetch(result);
        if(row == NULL) {
            break;
        }

        info = STREAMER_INFO();

        if(row[0 ] != NULL) info.streaming_channel_id  = row[0 ];
        if(row[1 ] != NULL) info.streaming_channel_url = row[1 ];
        if(row[2 ] != NULL) info.streamer_name         = row[2 ];
        if(row[3 ] != NULL) info.streamer_profile      = row[3 ];
        if(row[4 ] != NULL) info.streaming_title       = row[4 ];
        if(row[5 ] != NULL) info.streaming_category    = row[5 ];
        if(row[6 ] != NULL) info.streaming_status      = row[6 ][0];
        if(row[7 ] != NULL) info.open_time             = row[7 ];
        if(row[8 ] != NULL) info.close_time            = row[8 ];
        if(row[9 ] != NULL) info.regist_time           = row[9 ];
        if(row[10] != NULL) info.w_time                = row[10];

        list.push_back(info);
    }
    dbmysqlResultFinish(result);

    return list.size();
}

int queryInsertStreamerInfo(MYSQL* mysql, STREAMER_INFO& info)
{
    std::string query;
    int qlen;

    std::string esc_streamer_name;
    if(_queryEscapeString(mysql, info.streamer_name, esc_streamer_name) < 0) {
        return -1;
    }

    query = std::string("INSERT INTO streamer_info(") + DBQUERY_STREAMER_INFO_INSERT_COLUMN + ") VALUES ("
            + "'" + info.streaming_channel_id  + "', "
            + "'" + info.streaming_channel_url + "', "
            + "'" + esc_streamer_name          + "', "
            + "'" + info.streamer_profile      + "', "
            + "'', "
            + "'', "
            + "'C', "
            + "'', "
            + "'', "
            + "CURRENT_TIMESTAMP(), "
            + "CURRENT_TIMESTAMP()) ";
    qlen = query.length();

    return dbmysqlQueryExecute(mysql, query, qlen);
}

int queryUpdateStreamerInfoLiveStatus(MYSQL* mysql, STREAMER_INFO& info)
{
    std::string query;
    int qlen;

    std::string esc_streaming_title;
    if(_queryEscapeString(mysql, info.streaming_title, esc_streaming_title) < 0) {
        return -1;
    }

    query = "UPDATE streamer_info SET "
                "streaming_title = '" + esc_streaming_title + "', "
                "streaming_category = '" + info.streaming_category + "', "
                "streaming_status = '" + info.streaming_status + "', "
                "open_time = '" + info.open_time + "', "
                "close_time = '" + info.close_time + "' "
                "WHERE streaming_channel_id = '" + info.streaming_channel_id + "' ";
    qlen = query.length();

    return dbmysqlQueryExecute(mysql, query, qlen);
}
///////////////////////////////////////////////////////////////////////////////

// notice_info ////////////////////////////////////////////////////////////////
int querySelectNoticeInfoAll(MYSQL* mysql, std::vector<NOTICE_INFO>& list)
{
    std::string query;
    int qlen;

    query = "SELECT " DBQUERY_NOTICE_INFO_SELECT_COLUMN "FROM notice_info ntc ";
    qlen  = query.length();

    if(dbmysqlQueryExecute(mysql, query, qlen) != 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "query execute error[%d:%s]", qlen, query.c_str());
        return -1;
    }

    MYSQL_RES* result = dbmysqlQueryResult(mysql);
    if(result == NULL) {
        return -1;
    }

    MYSQL_ROW row;
    NOTICE_INFO info;
    while(true) {
        row = dbmysqlResultFetch(result);
        if(row == NULL) {
            break;
        }

        info = NOTICE_INFO();

        if(row[0] != NULL) info.streaming_channel_id  = row[0];
        if(row[1] != NULL) info.notice_channel_id     = std::stoull(row[1]);
        if(row[2] != NULL) info.mention_id            = std::stoull(row[2]);
        if(row[3] != NULL) info.notice_state          = std::stoi(row[3]);
        if(row[4] != NULL) info.notice_time           = row[4];
        if(row[5] != NULL) info.regist_id             = std::stoull(row[5]);
        if(row[6] != NULL) info.regist_time           = row[6];
        if(row[7] != NULL) info.w_time                = row[7];

        list.push_back(info);
    }
    dbmysqlResultFinish(result);

    return list.size();
}

int querySelectNoticeInfoAllByMentionId(MYSQL* mysql, const long long& mention_id, std::vector<NOTICE_INFO>& list)
{
    std::string query;
    int qlen;

    query = "SELECT " DBQUERY_NOTICE_INFO_SELECT_COLUMN "FROM notice_info ntc "
                "WHERE ntc.mention_id =" + std::to_string(mention_id) + " ";
    qlen  = query.length();

    if(dbmysqlQueryExecute(mysql, query, qlen) != 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "query execute error[%d:%s]", qlen, query.c_str());
        return -1;
    }

    MYSQL_RES* result = dbmysqlQueryResult(mysql);
    if(result == NULL) {
        return -1;
    }

    MYSQL_ROW row;
    NOTICE_INFO info;
    while(true) {
        row = dbmysqlResultFetch(result);
        if(row == NULL) {
            break;
        }

        info = NOTICE_INFO();

        if(row[0] != NULL) info.streaming_channel_id  = row[0];
        if(row[1] != NULL) info.notice_channel_id     = std::stoull(row[1]);
        if(row[2] != NULL) info.mention_id            = std::stoull(row[2]);
        if(row[3] != NULL) info.notice_state          = std::stoi(row[3]);
        if(row[4] != NULL) info.notice_time           = row[4];
        if(row[5] != NULL) info.regist_id             = std::stoull(row[5]);
        if(row[6] != NULL) info.regist_time           = row[6];
        if(row[7] != NULL) info.w_time                = row[7];

        list.push_back(info);
    }
    dbmysqlResultFinish(result);

    return list.size();
}

int querySelectNoticeInfoForSend(MYSQL* mysql, std::vector<NOTICE_INFO>& list)
{
    std::string query;
    int qlen;

    query = std::string("SELECT ") + DBQUERY_NOTICE_INFO_SELECT_COLUMN + "FROM notice_info ntc "
                "JOIN streamer_info stm ON stm.streaming_channel_id = ntc.streaming_channel_id "
                "WHERE stm.streaming_status = '" + STREAMING_STATUS_OPEN + "' "
                    "AND ntc.notice_state = " + std::to_string(NOTICE_STATE_WAIT) + " "; 
    qlen = query.length();

    if(dbmysqlQueryExecute(mysql, query, qlen) != 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "query execute error[%d:%s]", qlen, query.c_str());
        return -1;
    }

    MYSQL_RES* result = dbmysqlQueryResult(mysql);
    if(result == NULL) {
        return -1;
    }

    MYSQL_ROW row;
    NOTICE_INFO info;
    while(true) {
        row = dbmysqlResultFetch(result);
        if(row == NULL) {
            break;
        }

        info = NOTICE_INFO();

        if(row[0] != NULL) info.streaming_channel_id  = row[0];
        if(row[1] != NULL) info.notice_channel_id     = std::stoull(row[1]);
        if(row[2] != NULL) info.mention_id            = std::stoull(row[2]);
        if(row[3] != NULL) info.notice_state          = std::stoi(row[3]);
        if(row[4] != NULL) info.notice_time           = row[4];
        if(row[5] != NULL) info.regist_id             = std::stoull(row[5]);
        if(row[6] != NULL) info.regist_time           = row[6];
        if(row[7] != NULL) info.w_time                = row[7];

        list.push_back(info);
    }
    dbmysqlResultFinish(result);

    return list.size();
}

int queryInsertNoticeInfo(MYSQL* mysql, NOTICE_INFO& info)
{
    std::string query;
    int qlen;

    query = std::string("INSERT INTO notice_info(") + DBQUERY_NOTICE_INFO_INSERT_COLUMN + ") VALUES ("
            + "'" + info.streaming_channel_id + "', "
            + std::to_string(info.notice_channel_id) + ", "
            + std::to_string(info.mention_id) + ", "
            + std::to_string(info.notice_state) + ", "
            + "'', "
            + std::to_string(info.regist_id) + ", "
            + "CURRENT_TIMESTAMP(), "
            + "CURRENT_TIMESTAMP()) ";
    qlen = query.length();

    return dbmysqlQueryExecute(mysql, query, qlen);
}

int queryUpdateNoticeInfoNoticeState(MYSQL* mysql, int notice_state, const std::string& streaming_channel_id)
{
    std::string query;
    int qlen;

    query = "UPDATE notice_info SET "
                "notice_state = '" + std::to_string(notice_state) + "', "
                "notice_time = CURRENT_TIMESTAMP(), "
                "w_time = CURRENT_TIMESTAMP() "
                "WHERE streaming_channel_id = '" + streaming_channel_id + "' "
                    "AND notice_state <> " + std::to_string(notice_state) + " ";
    qlen = query.length();

    return dbmysqlQueryExecute(mysql, query, qlen);
}

int queryDeleteNoticeInfoUnitByMentionId(MYSQL* mysql, const std::string& streaming_channel_id, unsigned long long& mention_id)
{
    std::string query;
    int qlen;

    query = "DELETE FROM notice_info "
                "WHERE mention_id = '" + std::to_string(mention_id) + "' "
                    "AND streaming_channel_id = '" + streaming_channel_id + "' ";
    qlen = query.length();

    return dbmysqlQueryExecute(mysql, query, qlen);
}
///////////////////////////////////////////////////////////////////////////////

// notice_info ////////////////////////////////////////////////////////////////
int querySelectApiAuthByName(MYSQL* mysql, const std::string& name, API_AUTH& auth)
{
    std::string query;
    int qlen;

    query = "SELECT " DBQUERY_API_AUTH_SELECT_COLUMN "FROM api_auth ath "
                "WHERE name = '" + name + "' ";
    qlen  = query.length();

    if(dbmysqlQueryExecute(mysql, query, qlen) != 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "query execute error[%d:%s]", qlen, query.c_str());
        return -1;
    }

    MYSQL_RES* result = dbmysqlQueryResult(mysql);
    if(result == NULL) {
        return -1;
    }

    int count = 0;
    MYSQL_ROW row = dbmysqlResultFetch(result);
    if(row != NULL) {
        if(row[0] != NULL) auth.name        = row[0];
        if(row[1] != NULL) auth.value       = row[1];
        if(row[2] != NULL) auth.expire_time = row[2];
        if(row[3] != NULL) auth.update_time = row[3];

        count++;
    }
    dbmysqlResultFinish(result);

    return count;
}

int querySelectApiAuthAll(MYSQL* mysql, std::vector<API_AUTH>& list)
{
    std::string query;
    int qlen;

    query = "SELECT " DBQUERY_API_AUTH_SELECT_COLUMN "FROM api_auth ath ";
    qlen  = query.length();

    if(dbmysqlQueryExecute(mysql, query, qlen) != 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "query execute error[%d:%s]", qlen, query.c_str());
        return -1;
    }

    MYSQL_RES* result = dbmysqlQueryResult(mysql);
    if(result == NULL) {
        return -1;
    }

    MYSQL_ROW row;
    API_AUTH auth;
    while(true) {
        row = dbmysqlResultFetch(result);
        if(row == NULL) {
            break;
        }

        auth = API_AUTH();

        if(row[0] != NULL) auth.name        = row[0];
        if(row[1] != NULL) auth.value       = row[1];
        if(row[2] != NULL) auth.expire_time = row[2];
        if(row[3] != NULL) auth.update_time = row[3];

        list.push_back(auth);
    }
    dbmysqlResultFinish(result);

    return list.size();
}

int queryInsertApiAuth(MYSQL* mysql, API_AUTH& auth, int add_expire)
{
    std::string query;
    int qlen;

    query = std::string("INSERT INTO api_auth(") + DBQUERY_API_AUTH_INSERT_COLUMN + ") VALUES ("
            + "'" + auth.name + "', "
            + "'" + auth.value + "', "
            + "DATE_ADD(CURRENT_TIMESTAMP(), INTERVAL " + std::to_string(add_expire) + " HOUR), "
            + "CURRENT_TIMESTAMP()) ";
    qlen = query.length();

    return dbmysqlQueryExecute(mysql, query, qlen);
}

int queryUpdateApiAuth(MYSQL* mysql, API_AUTH& auth, int add_expire)
{
    std::string query;
    int qlen;

    std::string query_expire = "";
    if(add_expire > 0) {
        query_expire = ", expire_time = DATE_ADD(CURRENT_TIMESTAMP(), INTERVAL " + std::to_string(add_expire) + " HOUR) ";
    }

    query = "UPDATE api_auth SET "
                "value = '" + auth.value + "' " + query_expire +
                "WHERE name = '" + auth.name + "' ";
    qlen = query.length();

    return dbmysqlQueryExecute(mysql, query, qlen);
}

int queryDeleteApiAuthByName(MYSQL* mysql, const std::string& name)
{
    std::string query;
    int qlen;

    query = "DELETE FROM api_auth WHERE name = '" + name + "' ";
    qlen = query.length();

    return dbmysqlQueryExecute(mysql, query, qlen);
}
///////////////////////////////////////////////////////////////////////////////

