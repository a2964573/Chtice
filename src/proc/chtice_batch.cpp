#include "chtice_batch.h"


GlobalVar _global = {};

int main(int argc, char* argv[])
{
    UTILLOG(LOGLV_NOR, __FUNCTION__, "=== Chtice Batch Start ===");

    if(mainInitailize(argc, argv) < 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "main initailize failed.");
        return -1;
    }

    mainProcess();
    mainTerminate();
    return 0;
}

int mainInitailize(int argc, char* argv[])
{
    UTILLOG(LOGLV_NOR, __FUNCTION__, "main initailize.");

    if(argc < 2) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "a few arguments.");
        return -1;
    }

    int pos;
    for(pos = 1; pos < argc; pos++) {
        _global.g_args.push_back(argv[pos]);
    }

    const char* project_conf = std::getenv("PROJECT_CONF");
    _global.env_path = project_conf ? project_conf : ABSOLUTE_PATH_CONF;
    _global.ini_path = project_conf ? project_conf : ABSOLUTE_PATH_CONF;

    dbmysqlSetEnvFile(_global.dbmysql, _global.env_path, ".env");
    dbmysqlSetIniFile(_global.dbmysql, _global.ini_path, "interface.ini");

    if(dbmysqlConnect(_global.dbmysql) != 0 ) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "DB connect filed.");
        return -1;
    }

    return 0;
}

int mainTerminate()
{
    UTILLOG(LOGLV_NOR, __FUNCTION__, "main terminate.");

    dbmysqlClose(_global.dbmysql.mysql);
    return 0;
}

int mainProcess()
{
    std::string type = _global.g_args[0];

    if(type == "token") {
        if(batchManagementApiToken() < 0) {
            // error log
            return -1;
        }
    }
    else
    if(type == "sync") {
        if(batchSyncStreamerInfoALL() < 0) {
            // error log
            return -1;
        }
    }
    else {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "Unkown Type.[%s]", type.c_str());
        return -1;
    }

    return 0;
}

// Management API Token ///////////////////////////////////////////////////////
int _issueAccessToken(const std::string& auth_code, API_AUTH& acs_auth, API_AUTH& rfs_auth)
{
    HTTPS https;

    std::string host = utilGetIniVar(_global.ini_path, "interface.ini", "CHZZK", "OPENAPI_ADDRESS");
    httpsSetHeader(https, "Host"        , host              );
    httpsSetHeader(https, "User-Agent"  , "Chtice/1.0"      );
    httpsSetHeader(https, "Connection"  , "close"           );
    httpsSetHeader(https, "Content-Type", "application/json");

    httpsSetRequestType(https, REQUEST_TYPE_POST);
    httpsSetBodyType(https, BODY_TYPE_JSON);

    httpsSetHost(https, host);

    std::string path = "/auth/v1/token";
    httpsSetPath(https, path);

    // body
    std::string client_id     = utilGetEnvVar(_global.env_path, ".env", "CHZZK_CLIENT_ID"    );
    std::string client_secret = utilGetEnvVar(_global.env_path, ".env", "CHZZK_CLIENT_SECRET");

    httpsSetBody(https, "grantType"   , "authorization_code" );
    httpsSetBody(https, "clientId"    , client_id            );
    httpsSetBody(https, "clientSecret", client_secret        );
    httpsSetBody(https, "code"        , auth_code.c_str()    );
    httpsSetBody(https, "state"       , "chtice_state_1234"  );

    // address 는 세팅할 필요없음
    httpsSetAddress(https, "");

    std::string s_port = utilGetIniVar(_global.ini_path, "interface.ini", "CHZZK", "PORT");
    int port = s_port.empty() ? 443 : std::stoi(s_port);
    httpsSetPort(https, port);

    int rtn = httpsRequestCall(https);
    if(rtn < 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "httpsRequestCall function error[%s]", https.res.error_msg.c_str());
        return -1;
    }
    else
    if(rtn != 200) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "httpsRequestCall Bad Response[%d]", rtn);
        return -1;
    }

    try {
        json res_json;
        if(httpsGetParseJson(https.res.body, res_json) < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "httpsGetParseJson error");
            return -1;
        }

        if((!res_json.contains("code")) || (res_json["code"] != 200)) {
            int code = -1;
            if(res_json["code"].is_number()) {
                code = res_json["code"].get<int>();
            }

            std::string msg = "error.";
            if((res_json.contains("message")) && (res_json["message"].is_string())) {
                msg = res_json["message"].get<std::string>();
            }

            UTILLOG(LOGLV_ERR, __FUNCTION__, "API 응답 코드 에러[%d][%s]", code, msg.c_str());
            return -1;
        }

        if((!res_json.contains("content")) || (res_json["content"].is_null())) {
            UTILLOG(LOGLV_DBG, __FUNCTION__, "content 값이 null 입니다.");
            return 0;
        }

        auto content = res_json["content"];

        if((content.contains("accessToken")) && (content["accessToken"].is_string())) {
            acs_auth.name  = "access_token";
            acs_auth.value = content["accessToken"];
        }

        if((content.contains("refreshToken")) && (content["refreshToken"].is_string())) {
            rfs_auth.name  = "refresh_token";
            rfs_auth.value = content["refreshToken"];
        }
    }
    catch(const json::exception& e) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "치지직 API JSON 파싱 실패: %s", e.what());
        return -1;
    }

    return 0;
}

int _refreshAccessToken(const std::string& auth_code, API_AUTH& rfs_auth, API_AUTH& acs_auth)
{
    HTTPS https;

    std::string host = utilGetIniVar(_global.ini_path, "interface.ini", "CHZZK", "OPENAPI_ADDRESS");
    httpsSetHeader(https, "Host"        , host              );
    httpsSetHeader(https, "User-Agent"  , "Chtice/1.0"      );
    httpsSetHeader(https, "Connection"  , "close"           );
    httpsSetHeader(https, "Content-Type", "application/json");

    httpsSetRequestType(https, REQUEST_TYPE_POST);
    httpsSetBodyType(https, BODY_TYPE_JSON);

    httpsSetHost(https, host);

    std::string path = "/auth/v1/token";
    httpsSetPath(https, path);

    // body
    std::string client_id     = utilGetEnvVar(_global.env_path, ".env", "CHZZK_CLIENT_ID"    );
    std::string client_secret = utilGetEnvVar(_global.env_path, ".env", "CHZZK_CLIENT_SECRET");

    httpsSetBody(https, "grantType"   , "refresh_token"       );
    httpsSetBody(https, "refreshToken", rfs_auth.value.c_str());
    httpsSetBody(https, "clientId"    , client_id             );
    httpsSetBody(https, "clientSecret", client_secret         );

    // address 는 세팅할 필요없음
    httpsSetAddress(https, "");

    std::string s_port = utilGetIniVar(_global.ini_path, "interface.ini", "CHZZK", "PORT");
    int port = s_port.empty() ? 443 : std::stoi(s_port);
    httpsSetPort(https, port);

    if(httpsRequestCall(https) < 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "httpsRequestCall function error[%s]", https.res.error_msg.c_str());
        return -1;
    }

    try {
        json res_json;
        if(httpsGetParseJson(https.res.body, res_json) < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "httpsGetParseJson error");
            return -1;
        }

        if((!res_json.contains("code")) || (res_json["code"] != 200)) {
            int code = -1;
            if(res_json["code"].is_number()) {
                code = res_json["code"].get<int>();
            }

            std::string msg = "error.";
            if((res_json.contains("message")) && (res_json["message"].is_string())) {
                msg = res_json["message"].get<std::string>();
            }

            UTILLOG(LOGLV_ERR, __FUNCTION__, "API 응답 코드 에러[%d][%s]", code, msg.c_str());
            return -1;
        }

        if((!res_json.contains("content")) || (res_json["content"].is_null())) {
            UTILLOG(LOGLV_DBG, __FUNCTION__, "content 값이 null 입니다.");
            return 0;
        }

        auto content = res_json["content"];

        if((content.contains("accessToken")) && (content["accessToken"].is_string())) {
            acs_auth.name  = "access_token";
            acs_auth.value = content["accessToken"];
        }

        if((content.contains("refreshToken")) && (content["refreshToken"].is_string())) {
            rfs_auth.name  = "refresh_token";
            rfs_auth.value = content["refreshToken"];
        }
    }
    catch(const json::exception& e) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "치지직 API JSON 파싱 실패: %s", e.what());
        return -1;
    }

    return 0;
}

int batchManagementApiToken()
{
    // api code 조회
    API_AUTH cod_auth;
    if(querySelectApiAuthByName(_global.dbmysql.mysql, "auth_code", cod_auth) <= 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "querySelectApiAuthByName error or auth_code not found.");
        return -1;
    }

    // access_token 조회
    API_AUTH acs_auth;
    API_AUTH rfs_auth;
    int rtn = querySelectApiAuthByName(_global.dbmysql.mysql, "access_token", acs_auth);
    if(rtn < 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "querySelectApiAuthByName error.[access_token]");
        return -1;
    }
    else
    if(rtn > 0) {
        rtn = querySelectApiAuthByName(_global.dbmysql.mysql, "refresh_token", rfs_auth);
        if(rtn < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "querySelectApiAuthByName error.[refresh_token]");
            return -1;
        }
        else
        if(rtn > 0) {
            std::tm tm = {};
            std::istringstream ss(rfs_auth.expire_time);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            if(ss.fail()) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "refresh_token expire_time parse failed.");
                return -1;
            }

            std::time_t expire_time  = std::mktime(&tm);
            std::time_t current_time = std::time(nullptr);
            if(expire_time > current_time) {
                // access_token 갱신
                if(_refreshAccessToken(cod_auth.value, rfs_auth, acs_auth) < 0) {
                    UTILLOG(LOGLV_ERR, __FUNCTION__, "_refreshAccessToken failed.");
                    return -1;
                }

                // db update
                if(queryUpdateApiAuth(_global.dbmysql.mysql, acs_auth, 24) != 0) {
                    UTILLOG(LOGLV_ERR, __FUNCTION__, "queryUpdateApiAuth error.[access_token]");
                    return -1;
                }

                if(queryUpdateApiAuth(_global.dbmysql.mysql, rfs_auth, 0) != 0) {
                    UTILLOG(LOGLV_ERR, __FUNCTION__, "queryUpdateApiAuth error.[refresh_token]");
                    return -1;
                }

                return 0;
            }
        }
    }

    // access_token 발급
    if(_issueAccessToken(cod_auth.value, acs_auth, rfs_auth) < 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "_issueAccessToken failed.");
        return -1;
    }

    // insert 하기 전 삭제
    {
        if(queryDeleteApiAuthByName(_global.dbmysql.mysql, "access_token") < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "queryDeleteApiAuthByName error.[access_token]");
            return -1;
        }

        if(queryDeleteApiAuthByName(_global.dbmysql.mysql, "refresh_token") < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "queryDeleteApiAuthByName error.[refresh_token]");
            return -1;
        }
    }

    // db insert
    {
        if(queryInsertApiAuth(_global.dbmysql.mysql, acs_auth, 24) != 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "queryInsertApiAuth error.[access_token]");
            return -1;
        }

        if(queryInsertApiAuth(_global.dbmysql.mysql, rfs_auth, 720) != 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "queryInsertApiAuth error.[refresh_token]");
            return -1;
        }
    }

    return 0;
}
///////////////////////////////////////////////////////////////////////////////

// Sync DB Streamer Info //////////////////////////////////////////////////////
int _getLiveStatus(STREAMER_INFO& org_info, STREAMER_INFO& cur_info)
{
    HTTPS https;

    std::string host = utilGetIniVar(_global.ini_path, "interface.ini", "CHZZK", "API_ADDRESS");
    httpsSetHeader(https, "Host", host);
    httpsSetHeader(https, "User-Agent", "Chtice/1.0");
    httpsSetHeader(https, "Connection", "close");

    httpsSetRequestType(https, REQUEST_TYPE_GET);

    httpsSetHost(https, host);

    std::string path = "/polling/v3.1/channels/" + org_info.streaming_channel_id + "/live-status";
    httpsSetPath(https, path);

    // address 는 세팅할 필요없음
    httpsSetAddress(https, "");

    std::string s_port = utilGetIniVar(_global.ini_path, "interface.ini", "CHZZK", "PORT");
    int port = s_port.empty() ? 443 : std::stoi(s_port);
    httpsSetPort(https, port);


    int rtn = httpsRequestCall(https);
    if(rtn < 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "httpsRequestCall function error[%s]", https.res.error_msg.c_str());
        return -1;
    }
    else
    if(rtn != 200) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "httpsRequestCall Bad Response[%d]", rtn);
        return -1;
    }

    cur_info = org_info;

    try {
        json res_json;
        if(httpsGetParseJson(https.res.body, res_json) < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "httpsGetParseJson error");
            return -1;
        }

        if((!res_json.contains("code")) || (res_json["code"] != 200)) {
            int code = -1;
            if(res_json["code"].is_number()) {
                code = res_json["code"].get<int>();
            }

            std::string msg = "error.";
            if((res_json.contains("message")) && (res_json["message"].is_string())) {
                msg = res_json["message"].get<std::string>();
            }

            UTILLOG(LOGLV_ERR, __FUNCTION__, "API 응답 코드 에러[%d][%s]", code, msg.c_str());
            return -1;
        }

        if((!res_json.contains("content")) || (res_json["content"].is_null())) {
            UTILLOG(LOGLV_DBG, __FUNCTION__, "content 값이 null 입니다.");
            return 0;
        }

        auto content = res_json["content"];

        if((content.contains("liveTitle")) && (content["liveTitle"].is_string())) {
            cur_info.streaming_title = content["liveTitle"];
        }

        if((content.contains("liveCategory")) && (content["liveCategory"].is_string())) {
            cur_info.streaming_category = content["liveCategory"];
        }

        if((content.contains("status")) && (content["status"].is_string())) {
            cur_info.streaming_status = content["status"] == "OPEN" ? 'O' : 'C';
        }

        if((content.contains("openDate")) && (content["openDate"].is_string())) {
            cur_info.open_time = content["openDate"];
        }

        if((content.contains("closeDate")) && (content["closeDate"].is_string())) {
            cur_info.close_time = content["closeDate"];
        }
    }
    catch (const json::exception& e) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "치지직 API JSON 파싱 실패: %s", e.what());
        return -1;
    }

    return 0;
}

int batchSyncStreamerInfoALL()
{
    std::vector<STREAMER_INFO> list;
    int count = querySelectStreamerInfoAll(_global.dbmysql.mysql, list);
    if(count > 0) {
        STREAMER_INFO cur_info;
        for(auto& org_info : list) {
            cur_info = {};
            if(_getLiveStatus(org_info, cur_info) < 0) {
                // error message
                break;
            }

            if(queryUpdateStreamerInfoLiveStatus(_global.dbmysql.mysql, cur_info) != 0) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "queryUpdateStreamerInfo error");
                break;
            }
        }
    }
    else
    if(count == 0) {
        UTILLOG(LOGLV_NOR, __FUNCTION__, "No streamer data found in STREAMER_INFO (0 rows)");
    }
    else {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "querySelectStreamerInfoAll error");
        return -1;
    }

    return 0;
}
///////////////////////////////////////////////////////////////////////////////
