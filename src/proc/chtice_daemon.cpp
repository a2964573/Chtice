#include "chtice_daemon.h"


GlobalVar _global = {};

int main(int argc, char* argv[])
{
    UTILLOG(LOGLV_NOR, __FUNCTION__, "=== Chtice Daemon Process Start ===");

    // 프로세스 시작
    _global.is_running = true;

    if(mainInitailize() < 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "메인 초기화 실패");
        return -1;
    }

    mainProcess();
    mainTerminate();
    return 0;
}

int mainInitailize()
{
    UTILLOG(LOGLV_NOR, __FUNCTION__, "main initailize.");

    // 프로세스 종료신호 설정
    commonRegisterSignalHandler(&_global.is_running);

    const char* project_conf = std::getenv("PROJECT_CONF");
    _global.env_path = project_conf ? project_conf : ABSOLUTE_PATH_CONF;
    _global.ini_path = project_conf ? project_conf : ABSOLUTE_PATH_CONF;

    dbmysqlSetEnvFile(_global.dbmysql, _global.env_path, ".env");
    dbmysqlSetIniFile(_global.dbmysql, _global.ini_path, "interface.ini");

    if(dbmysqlConnect(_global.dbmysql) != 0 ) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "DB connect failed.");
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
    int count;
    std::vector<STREAMER_INFO> list;

    while(_global.is_running) {
        list.clear();

        count = querySelectStreamerInfoAll(_global.dbmysql.mysql, list);
        if(count > 0) {
            pollLiveStatus(list);
        }
        else
        if(count == 0) {
            UTILLOG(LOGLV_NOR, __FUNCTION__, "No streamer data found in STREAMER_INFO (0 rows)");
        }
        else {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "querySelectStreamerInfoAll error");
            break;
        }

        commonProcessWaitSecond(30);
    }

    return 0;
}

int pollLiveStatus(std::vector<STREAMER_INFO>& list)
{
    STREAMER_INFO cur_info;
    for(auto& org_info : list) {
        cur_info = {};
        if(chticeGetLiveStatus(org_info, cur_info) < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "[%s] chticeGetLiveStatus error", org_info.streamer_name.c_str());
            continue;
        }

        if((org_info.streaming_title    == cur_info.streaming_title   ) &&
           (org_info.streaming_category == cur_info.streaming_category) &&
           (org_info.streaming_status   == cur_info.streaming_status  )
        ) {
            // pass
            continue;
        }

        // save function
        UTILLOG(LOGLV_DBG, __FUNCTION__, "[%s] streaming_status change [%c][%c]"
            , org_info.streamer_name.c_str()
            , org_info.streaming_status
            , cur_info.streaming_status
        );

        if(queryUpdateStreamerInfoLiveStatus(_global.dbmysql.mysql, cur_info) != 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "queryUpdateStreamerInfo error");
            break;
        }

        if(cur_info.streaming_status == STREAMING_STATUS_CLOSE) {
            if(queryUpdateNoticeInfoNoticeState(_global.dbmysql.mysql, NOTICE_STATE_WAIT, cur_info.streaming_channel_id) != 0) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "queryUpdateNoticeInfoNoticeState error");
                break;
            }
        }
    }

    return 0;
}

int chticeGetLiveStatus(STREAMER_INFO& org_info, STREAMER_INFO& cur_info)
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

