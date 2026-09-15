#pragma once

#include <dpp/dpp.h>

#include "common.h"
#include "dbquery.h"
#include "https.h"

typedef struct _GlobalVar {
    std::atomic_bool is_running;
    std::string bot_token;
    std::string env_path;
    std::string ini_path;
    std::unique_ptr<dpp::cluster> bot;
    DBMYSQL dbmysql;
} GlobalVar;

int mainInitailize();
int mainTerminate();
int mainProcess();
int daemonProcess();
int sendStreamingNotice(std::vector<NOTICE_INFO>& list);

// Bot ////////////////////////////////////////////////////////////////////////
int botInitailize();
int createSlashCommand();
int onSlashCommandHelp(const dpp::slashcommand_t& event);
int onSlashCommandRegister(const dpp::slashcommand_t& event);
int onSlashCommandRelease(const dpp::slashcommand_t& event);
int onSlashCommandSelect(const dpp::slashcommand_t& event);
int onSlashCommandNoticeList(const dpp::slashcommand_t& event);
///////////////////////////////////////////////////////////////////////////////

