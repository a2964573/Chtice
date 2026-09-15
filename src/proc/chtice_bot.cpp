#include "chtice_bot.h"


GlobalVar _global = {};

int main(int argc, char* argv[])
{
    UTILLOG(LOGLV_NOR, __FUNCTION__, "=== Chtice Discord Slash Command Bot Start ===");

    // 포로세스 시작
    _global.is_running = true;

    if(mainInitailize() < 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "main initailize failed.");
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
        UTILLOG(LOGLV_ERR, __FUNCTION__, "DB connect filed.");
        return -1;
    }

    _global.bot_token = utilGetEnvVar(_global.env_path, ".env", "DISCORD_BOT_TOKEN");
    if(_global.bot_token == "") {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "bot tokken find failed.");
        return -1;
    }

    if(botInitailize() < 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "bot initailize filed.");
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
    // bot
    _global.bot->start(dpp::st_return);

    // notice
    int count;
    std::vector<NOTICE_INFO> list;

    while(_global.is_running) {
        list.clear();

        count = querySelectNoticeInfoForSend(_global.dbmysql.mysql, list);
        if(count > 0) {
            sendStreamingNotice(list);
        }
        else
        if(count == 0) {
            UTILLOG(LOGLV_NOR, __FUNCTION__, "No notice data found in NOTICE_INFO (0 rows)");
        }
        else {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "querySelectNoticeInfoAll error");
            break;
        }

        commonProcessWaitSecond(30);
    }

    return 0;
}

int sendStreamingNotice(std::vector<NOTICE_INFO>& list)
{
    STREAMER_INFO sinfo;
    for(auto& ninfo : list) {
        sinfo = {};
        if(querySelectStreamerInfoByStreamingChannelId(_global.dbmysql.mysql, ninfo.streaming_channel_id, sinfo) < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "querySelectStreamerInfo error");
            continue;
        }

        // 1. 유저 멘션 텍스트
        std::string mention_text = std::string("<@")
                                 + std::to_string(ninfo.mention_id)
                                 + "> "
                                 + sinfo.streamer_name
                                 + " 방송이 시작되었습니다!";

        // 2. 디스코드 Embed 생성
        dpp::embed embed;

        // 색상 (치지직 녹색)
        embed.set_color(0x00FFA3);

        std::string profile = sinfo.streamer_profile.length() > 0 ? sinfo.streamer_profile : "";
        embed.set_author(sinfo.streamer_name, sinfo.streaming_channel_url, profile);

        // 제목
        embed.set_title(sinfo.streaming_title);

        // 방송 url 바로가기
        embed.set_url(sinfo.streaming_channel_url);

        // 카테고리
        embed.set_description("**카테고리:** " + sinfo.streaming_category);

        // 방송시작시간
        std::string footer_text = std::string(sinfo.open_time);
        embed.set_footer(dpp::embed_footer().set_text(footer_text));

        // 3. 메시지 객체 생성 후 전송
        dpp::message msg(ninfo.notice_channel_id, mention_text);
        msg.add_embed(embed);

        _global.bot->message_create(msg);
        UTILLOG(LOGLV_DBG, __FUNCTION__, "디스코드 Embed 전송 완료!");

        if(queryUpdateNoticeInfoNoticeState(_global.dbmysql.mysql, NOTICE_STATE_SEND, ninfo.streaming_channel_id) != 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "queryUpdateNoticeInfoNoticeState error");
            continue;
        }
    }

    return 0;
}

// Bot ////////////////////////////////////////////////////////////////////////
int botInitailize()
{
    _global.bot = std::make_unique<dpp::cluster>(_global.bot_token);

    if(createSlashCommand() < 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "createSlashCommand failed.");
        return -1;
    }

    _global.bot->on_slashcommand([](const dpp::slashcommand_t& event) {
        // 사용자 명령어
        if(event.command.get_command_name() == "도움말") {
            onSlashCommandHelp(event);
        }
        else
        if(event.command.get_command_name() == "등록") {
            if(onSlashCommandRegister(event) < 0) {
                dpp::message err_msg("요청주신 처리가 실패하였습니다, 다시 확인 해주세요.");
                err_msg.set_flags(dpp::m_ephemeral);
                event.reply(err_msg);
            }
        }
        else
        if(event.command.get_command_name() == "해제") {
            if(onSlashCommandRelease(event) < 0) {
                dpp::message err_msg("요청주신 처리가 실패하였습니다, 다시 확인 해주세요.");
                err_msg.set_flags(dpp::m_ephemeral);
                event.reply(err_msg);
            }
        }
        else
        if(event.command.get_command_name() == "조회") {
            if(onSlashCommandSelect(event) < 0) {
                dpp::message err_msg("요청주신 처리가 실패하였습니다, 다시 확인 해주세요.");
                err_msg.set_flags(dpp::m_ephemeral);
                event.reply(err_msg);
            }
        }
        else
        if(event.command.get_command_name() == "목록") {
            if(onSlashCommandNoticeList(event) < 0) {
                dpp::message err_msg("요청주신 처리가 실패하였습니다, 다시 확인 해주세요.");
                err_msg.set_flags(dpp::m_ephemeral);
                event.reply(err_msg);
            }
        }
        // 관리자 명령어
        else
        if(event.command.get_command_name() == "sync_all") {
            std::string bot_admin_id = utilGetEnvVar(_global.env_path, ".env", "BOT_ADMIN_ID");  
            std::string user_id = std::to_string(event.command.get_issuing_user().id);
            if(user_id == bot_admin_id) {
                std::string result;
                if(utilExecutePopenCommand("chtice_batch sync", result) < 0) {
                    dpp::message err_msg("요청주신 처리가 실패하였습니다, 다시 확인 해주세요.");
                    err_msg.set_flags(dpp::m_ephemeral);
                    event.reply(err_msg);
                }
                else {
                    dpp::message msg("처리가 완료되었습니다.");
                    msg.set_flags(dpp::m_ephemeral);
                    event.reply(msg);
                }
            }
            else {
                dpp::message err_msg("잘못된 명령어 입니다.");
                err_msg.set_flags(dpp::m_ephemeral);
                event.reply(err_msg);
            }
        }
        else {
            dpp::user user = event.command.get_issuing_user();

            uint64_t channel_id = event.command.channel_id;
            uint64_t user_id = user.id;

            UTILLOG(LOGLV_ERR, __FUNCTION__, "Unknown slashcommand.[%s] ChannelID:[%llu] UserID:[%llu] UserName:[%s]",
                event.command.get_command_name().c_str(),
                (unsigned long long)channel_id,
                (unsigned long long)user_id,
                user.username.c_str()
            );

            dpp::message err_msg("잘못된 명령어 입니다.");
            err_msg.set_flags(dpp::m_ephemeral);
            event.reply(err_msg);
        }
    });

    return 0;
}

int createSlashCommand()
{
    // 봇 로그인 및 슬래시 명령어 등록
    _global.bot->on_ready([](const dpp::ready_t& event) {
        UTILLOG(LOGLV_NOR, __FUNCTION__, "[Discord] 봇 로그인 완료: %s", _global.bot->me.username.c_str());

        if(dpp::run_once<struct register_bot_commands>()) {
            std::vector<dpp::slashcommand> commands;

            // 사용자 명령어
            // [도움말] 명령어
            dpp::slashcommand cmd_help("도움말", "명령어 목록과 사용방법을 확인합니다.", _global.bot->me.id);
            commands.push_back(cmd_help);

            // [등록] 명령어
            dpp::slashcommand cmd_register("등록", "새로운 치지직 스트리머 방송 알림을 등록합니다.", _global.bot->me.id);
            cmd_register.add_option(dpp::command_option(dpp::co_string, "치지직id", "치지직 채널 고유 해시 ID (32자)", true));
            cmd_register.add_option(dpp::command_option(dpp::co_channel, "알림채널", "알림 메시지를 띄울 채널 선택", true));
            commands.push_back(cmd_register);

            // [해제] 명령어
            dpp::slashcommand cmd_release("해제", "더 이상 해당 스트리머의 방송 알림을 받지 않습니다.", _global.bot->me.id);
            cmd_release.add_option(dpp::command_option(dpp::co_string, "스트리머이름", "조회할 스트리머의 이름", true));
            commands.push_back(cmd_release);

            // [조회] 명령어
            dpp::slashcommand cmd_search("조회", "등록된 치지직 스트리머의 방송 상태를 조회합니다.", _global.bot->me.id);
            cmd_search.add_option(dpp::command_option(dpp::co_string, "스트리머이름", "조회할 스트리머의 이름", true));
            commands.push_back(cmd_search);

            // [목록] 명령어
            dpp::slashcommand cmd_nlist("목록", "자신이 등록한 알림 목록을 조회합니다.", _global.bot->me.id);
            commands.push_back(cmd_nlist);

            // 관리자 명령어
            // [갱신]
            dpp::slashcommand cmd_sync("sync_all", "등록된 전체 스트리머 정보를 갱신합니다.", _global.bot->me.id);
            cmd_sync.set_default_permissions(dpp::p_administrator);
            commands.push_back(cmd_sync);

            _global.bot->global_bulk_command_create(commands);
            
            UTILLOG(LOGLV_NOR, __FUNCTION__, "[Discord] 글로벌 슬래시 명령어 %d개 일괄 등록 완료", commands.size());
        }
    });

    return 0;
}

int onSlashCommandHelp(const dpp::slashcommand_t& event)
{
    dpp::message msg;
    msg.set_flags(dpp::m_ephemeral);

    dpp::embed embed;
    embed.set_color(0x00FFA3);
    embed.set_title("🟢 Chtice 도움말");
    embed.set_description("스트리머의 방송 상태를 확인하고, 알림을 받을 수 있는 디스코드 봇입니다.\n");

    embed.add_field(
        "💡 `/도움말`",
        "지금 보고 계신 이 명령어 안내 창을 띄웁니다.\n",
        false
    );

    embed.add_field(
        "📌 `/등록`",
        "새로운 치지직 스트리머의 방송 알림을 특정 채널에 등록합니다.\n"
        "**매개변수:** `치지직id`, `알림채널`\n"
        "> **※ `치지직id`란?**\n"
        "> 스트리머 채널 주소 끝에 있는 **32자리 고유 문자열**입니다.\n"
        "> 예) `https://chzzk.naver.com/`**`abcdefghijklmnopqrstuvwxyz012789`**\n",
        false
    );

    embed.add_field(
        "🗑️ `/해제`",
        "더 이상 해당 스트리머의 방송 알림을 받지 않습니다.\n"
        "**매개변수:** `스트리머이름`\n",
        false
    );

    embed.add_field(
        "🔍 `/조회`",
        "등록된 스트리머의 현재 방송 상태(방송 중/종료, 카테고리 등)를 확인합니다.\n"
        "**매개변수:** `스트리머이름`\n",
        false
    );

    embed.add_field(
        "📋 `/목록`",
        "알림 등록된 스트리머 목록을 확인합니다.\n",
        false
    );

    msg.add_embed(embed);
    event.reply(msg);

    return 0;
}

int _getCannelInfo(STREAMER_INFO& info)
{
    HTTPS https;

    std::string host          = utilGetIniVar(_global.ini_path, "interface.ini", "CHZZK", "OPENAPI_ADDRESS");
    std::string client_id     = utilGetEnvVar(_global.env_path, ".env", "CHZZK_CLIENT_ID"    );  
    std::string client_secret = utilGetEnvVar(_global.env_path, ".env", "CHZZK_CLIENT_SECRET");

    httpsSetHeader(https, "Host"         , host                     );
    httpsSetHeader(https, "User-Agent"   , "Chtice/1.0"             );
    httpsSetHeader(https, "Connection"   , "close"                  );
    httpsSetHeader(https, "Client-Id"    , client_id                );
    httpsSetHeader(https, "Client-Secret", client_secret            );
    httpsSetHeader(https, "Content-Type" , "application/json"       );

    httpsSetRequestType(https, REQUEST_TYPE_GET);

    httpsSetHost(https, host);

    std::string path = "/open/v1/channels?channelIds=" + info.streaming_channel_id;
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
        auto data = content["data"][0];

        if((data.contains("channelName")) && (data["channelName"].is_string())) {
            info.streamer_name = data["channelName"];
        }

        if((data.contains("channelImageUrl")) && (data["channelImageUrl"].is_string())) {
            info.streamer_profile = data["channelImageUrl"];
        }
    }
    catch(const json::exception& e) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "치지직 API JSON 파싱 실패: %s", e.what());
        return -1;
    }

    return 0;
}

int onSlashCommandRegister(const dpp::slashcommand_t& event)
{
    // STREAMER_INFO
    STREAMER_INFO sinfo;
    {
        sinfo.streaming_channel_id = std::get<std::string>(event.get_parameter("치지직id"));
        int rtn = querySelectStreamerInfoByStreamingChannelId(_global.dbmysql.mysql, sinfo.streaming_channel_id, sinfo);
        if(rtn < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "querySelectStreamerInfoByStreamingChannelId error");
            return -1;
        }
        else
        if(rtn == 0) {
            if(_getCannelInfo(sinfo) < 0) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "_getCannelInfo failed.");
                return -1;
            }

            sinfo.streaming_channel_url = "https://chzzk.naver.com/live/" + sinfo.streaming_channel_id;

            if(queryInsertStreamerInfo(_global.dbmysql.mysql, sinfo) != 0) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "queryInsertStreamerInfo error");
                return -1;
            }
        }
    }

    // NOTICE_INFO
    NOTICE_INFO ninfo;
    {
        // 스트리밍채널ID
        ninfo.streaming_channel_id = sinfo.streaming_channel_id;

        // 알림채널ID
        dpp::snowflake notice_channel = std::get<dpp::snowflake>(event.get_parameter("알림채널"));
        ninfo.notice_channel_id = notice_channel;

        // 알림수신자ID
        unsigned long long notice_user= event.command.get_issuing_user().id;
        ninfo.mention_id = notice_user;

        // 알림상태
        ninfo.notice_state = NOTICE_STATE_WAIT;

        // 등록자ID
        ninfo.regist_id = event.command.get_issuing_user().id;
    }

    if(queryInsertNoticeInfo(_global.dbmysql.mysql, ninfo) != 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "queryInsertNoticeInfo error");
        return -1;
    }

    dpp::message msg(std::string("✅ **") + sinfo.streamer_name + "** 님의 방송 알림이 성공적으로 등록되었습니다!\n"
        + "- 대상 유저: <@" + std::to_string(ninfo.mention_id) + ">\n"
        + "- 알림 채널: <#" + std::to_string(ninfo.notice_channel_id) + ">\n"
    );
    msg.set_flags(dpp::m_ephemeral);
    event.reply(msg);

    return 0;
}

int onSlashCommandRelease(const dpp::slashcommand_t& event)
{
    STREAMER_INFO sinfo;
    {       
        sinfo.streamer_name = std::get<std::string>(event.get_parameter("스트리머이름"));
        int rtn = querySelectStreamerInfoByStreamerName(_global.dbmysql.mysql, sinfo.streamer_name, sinfo);
        if(rtn < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "querySelectStreamerInfoByStreamerName error");
            return -1;
        }       
        else    
        if(rtn == 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "No streamer data found in streamer_name(0 rows)[%s]", sinfo.streamer_name.c_str());
            return -1;
        }
    }

    unsigned long long user_id = event.command.get_issuing_user().id;
    if(queryDeleteNoticeInfoUnitByMentionId(_global.dbmysql.mysql, sinfo.streaming_channel_id, user_id) != 0) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "queryDeleteNoticeInfoUnitByMentionId error");
        return -1;
    }

    std::string mention_text = "<@" + std::to_string(user_id) + "> 님의 "
                                    "**" + sinfo.streamer_name + "** 님의 방송 알림이 해제되었습니다";
    dpp::message msg(mention_text);
    msg.set_flags(dpp::m_ephemeral);

    event.reply(msg);

    return 0;
}

int onSlashCommandSelect(const dpp::slashcommand_t& event)
{
    STREAMER_INFO sinfo;
    {
        sinfo.streamer_name = std::get<std::string>(event.get_parameter("스트리머이름"));
        int rtn = querySelectStreamerInfoByStreamerName(_global.dbmysql.mysql, sinfo.streamer_name, sinfo);
        if(rtn < 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "querySelectStreamerInfoByStreamerName error");
            return -1;
        }
        else
        if(rtn == 0) {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "No streamer data found in streamer_name(0 rows)[%s]", sinfo.streamer_name.c_str());
            return -1;
        }
    }

    std::string mention_text = "**" + sinfo.streamer_name + "** 조회";
    dpp::message msg(mention_text);

    msg.set_flags(dpp::m_ephemeral);

    dpp::embed embed;

    std::string profile = sinfo.streamer_profile.length() > 0 ? sinfo.streamer_profile : "";
    embed.set_author(sinfo.streamer_name, sinfo.streaming_channel_url, profile);

    std::string desc = "";

    if(sinfo.streaming_status == STREAMING_STATUS_OPEN) {
        embed.set_color(0x00FFA3);

        desc += "- **방송상태:** 🟢 방송 중\n";
        desc += "- **방송제목:** [" + sinfo.streaming_title + "](" + sinfo.streaming_channel_url + ")\n";
        desc += "- **카테고리:** " + sinfo.streaming_category + "\n";
        desc += "- **시작시간:** " + sinfo.open_time;
    }
    else {
        embed.set_color(0x808080);

        desc += "- **방송상태:** 🔴 방송 종료\n";
        desc += "- **방송제목:** [" + sinfo.streaming_title + "](" + sinfo.streaming_channel_url + ")\n";
        desc += "- **카테고리:** " + sinfo.streaming_category + "\n";
        desc += "- **최근방송:** " + sinfo.open_time + "\n";
        desc += "- **방송종료:** " + sinfo.close_time;
    }

    embed.set_description(desc);

    msg.add_embed(embed);

    event.reply(msg);

    return 0;
}


int onSlashCommandNoticeList(const dpp::slashcommand_t& event)
{
    STREAMER_INFO sinfo;
    std::vector<STREAMER_INFO> slist;
    std::vector<NOTICE_INFO>   nlist;

    int rtn;
    int count;
    unsigned long long user_id = event.command.get_issuing_user().id;

    dpp::message msg;
    msg.set_flags(dpp::m_ephemeral);

    count = querySelectNoticeInfoAllByMentionId(_global.dbmysql.mysql, user_id, nlist);
    if(count > 0) {
        for(auto& ninfo : nlist) {
            sinfo = {};
            rtn = querySelectStreamerInfoByStreamingChannelId(_global.dbmysql.mysql, ninfo.streaming_channel_id, sinfo);
            if(rtn < 0) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "querySelectStreamerInfoByStreamingChannelId error");
                break;
            }
            else
            if(rtn == 0) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "error, No streamer data found in STREAMER_INFO (0 rows)");
                continue;
            }

            slist.push_back(sinfo);
        }
    }
    else
    if(count == 0) {
        UTILLOG(LOGLV_NOR, __FUNCTION__, "No notice data found in NOTICE_INFO (0 rows)");
        msg.set_content("등록된 스트리머가 없습니다.");
        event.reply(msg);
        return 0;
    }
    else {
         UTILLOG(LOGLV_ERR, __FUNCTION__, "querySelectNoticeInfoAllByMentionId error");
        return -1;
    }

    // Massege
    dpp::embed embed;
    embed.set_title("📋 현재 등록된 치지직 스트리머 목록");
    embed.set_color(0x00FFA3);

    std::string desc = "";
    desc += "총 **" + std::to_string(slist.size()) + "**명의 스트리머가 등록되어 있습니다.\n\n";

    for(auto& ssinfo : slist) {
        std::string status_icon = (ssinfo.streaming_status == STREAMING_STATUS_OPEN) ? "🟢" : "🔴";

        desc += status_icon + " **" + ssinfo.streamer_name + "**";
        desc += " `(" + ssinfo.streaming_category + ")`\n";
    }

    embed.set_description(desc);
    msg.add_embed(embed);

    event.reply(msg);

    return 0;
}
///////////////////////////////////////////////////////////////////////////////

