#pragma once

#include "common.h"
#include "dbmysql.h"

// streamer_info
#define STREAMING_STATUS_OPEN  'O'
#define STREAMING_STATUS_CLOSE 'C'

#define DBQUERY_STREAMER_INFO_SELECT_COLUMN "stm.streaming_channel_id, stm.streaming_channel_url, stm.streamer_name, " \
                                            "stm.streamer_profile, stm.streaming_title, stm.streaming_category, stm.streaming_status, " \
                                            "stm.open_time, stm.close_time, stm.regist_time, stm.w_time "

#define DBQUERY_STREAMER_INFO_INSERT_COLUMN "streaming_channel_id, streaming_channel_url, streamer_name, " \
                                            "streamer_profile, streaming_title, streaming_category, streaming_status, " \
                                            "open_time, close_time, regist_time, w_time "

typedef struct _STREAMER_INFO {
    std::string streaming_channel_id;
    std::string streaming_channel_url;
    std::string streamer_name;
    std::string streamer_profile;
    std::string streaming_title;
    std::string streaming_category;
    char        streaming_status;
    std::string open_time;
    std::string close_time;
    std::string regist_time;
    std::string w_time;
} STREAMER_INFO;

// notice_info
#define NOTICE_STATE_WAIT 0
#define NOTICE_STATE_SEND 1

#define DBQUERY_NOTICE_INFO_SELECT_COLUMN "ntc.streaming_channel_id, ntc.notice_channel_id, ntc.mention_id, " \
                                          "ntc.notice_state, ntc.notice_time, ntc.regist_id, ntc.regist_time, ntc.w_time "

#define DBQUERY_NOTICE_INFO_INSERT_COLUMN "streaming_channel_id, notice_channel_id, mention_id, " \
                                          "notice_state, notice_time, regist_id, regist_time, w_time "

typedef struct _NOTICE_INFO {
    std::string streaming_channel_id;
    uint64_t    notice_channel_id;
    uint64_t    mention_id;
    int         notice_state;
    std::string notice_time;
    uint64_t    regist_id;
    std::string regist_time;
    std::string w_time;
} NOTICE_INFO;

// api_auth
#define DBQUERY_API_AUTH_SELECT_COLUMN "ath.name, ath.value, ath.expire_time, ath.update_time "
#define DBQUERY_API_AUTH_INSERT_COLUMN "name, value, expire_time, update_time "

typedef struct _API_AUTH {
    std::string name;
    std::string value;
    std::string expire_time;
    std::string update_time;
} API_AUTH;

// streamer_info
int querySelectStreamerInfoByStreamingChannelId(MYSQL* mysql, const std::string& streaming_channel_id, STREAMER_INFO& info);
int querySelectStreamerInfoByStreamerName(MYSQL* mysql, const std::string& streamer_name, STREAMER_INFO& info);
int querySelectStreamerInfoAll(MYSQL* mysql,  std::vector<STREAMER_INFO>& list);
int queryInsertStreamerInfo(MYSQL* mysql, STREAMER_INFO& info);
int queryUpdateStreamerInfoLiveStatus(MYSQL* mysql, STREAMER_INFO& info);

// notice_info
int querySelectNoticeInfoAll(MYSQL* mysql, std::vector<NOTICE_INFO>& list);
int querySelectNoticeInfoForSend(MYSQL* mysql, std::vector<NOTICE_INFO>& list);
int querySelectNoticeInfoAllByMentionId(MYSQL* mysql, const long long& mention_id, std::vector<NOTICE_INFO>& list);
int queryInsertNoticeInfo(MYSQL* mysql, NOTICE_INFO& info);
int queryUpdateNoticeInfoNoticeState(MYSQL* mysql, int notice_state, const std::string& streaming_channel_id);
int queryDeleteNoticeInfoUnitByMentionId(MYSQL* mysql, const std::string& streaming_channel_id, unsigned long long& mention_id);

// api_auth
int querySelectApiAuthByName(MYSQL* mysql, const std::string& name, API_AUTH& auth);
int querySelectApiAuthAll(MYSQL* mysql, std::vector<API_AUTH>& list);
int queryInsertApiAuth(MYSQL* mysql, API_AUTH& auth, int add_expire);
int queryUpdateApiAuth(MYSQL* mysql, API_AUTH& auth, int add_expire);
int queryDeleteApiAuthByName(MYSQL* mysql, const std::string& name);

