#pragma once

#include <string>
#include <map>
#include <vector>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <nlohmann/json.hpp>
#include <arpa/inet.h>

#include "util.h"

using json = nlohmann::json;

enum {
    REQUEST_TYPE_GET ,
    REQUEST_TYPE_POST
};

enum {
    BODY_TYPE_ENCODED,
    BODY_TYPE_JSON
};

typedef struct _HttpResponse {
    int status_code;
    std::string body;
    std::string error_msg;
} HttpResponse;

typedef struct _HTTPS {
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> bodys;
    int request_type;
    int body_type;
    std::string host;
    std::string path;
    std::string address;
    int port;

    // 아래는 호출할 때 채우지 않는다...
    SSL_CTX* ctx;
    int sock;
    SSL* ssl;

    struct sockaddr_in server_addr;
    HttpResponse res;
} HTTPS;

int httpsSetHeader(HTTPS& https, const std::string& key, const std::string& value);
int httpsSetBody(HTTPS& https, const std::string& key, const std::string& value);
int httpsSetRequestType(HTTPS& https, int type);
int httpsSetBodyType(HTTPS& https, int type);
int httpsSetHost(HTTPS& https, std::string host);
int httpsSetPath(HTTPS& https, std::string path);
int httpsSetAddress(HTTPS& https, std::string address);
int httpsSetPort(HTTPS& https, int port);
int httpsRequestCall(HTTPS& https); 
int httpsGetParseJson(const std::string& body, json& output);

