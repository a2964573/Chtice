#include "https.h"

int httpsSetHeader(HTTPS& https, const std::string& key, const std::string& value)
{
    https.headers[key] = value;
    return 0;
}

int httpsSetBody(HTTPS& https, const std::string& key, const std::string& value)
{
    https.bodys[key] = value;
    return 0;
}

int httpsSetRequestType(HTTPS& https, int type)
{
    https.request_type = type;
    return 0;
}

int httpsSetBodyType(HTTPS& https, int type)
{
    https.body_type = type;
    return 0;
}

int httpsSetHost(HTTPS& https, std::string host)
{
    https.host = host;
    return 0;
}

int httpsSetPath(HTTPS& https, std::string path)
{
    https.path = path;
    return 0;
}

int httpsSetAddress(HTTPS& https, std::string address)
{
    https.address = address;
    return 0;
}

int httpsSetPort(HTTPS& https, int port)
{
    https.port = port;
    return 0;
}

int httpsFinish(HTTPS& https)
{
    if(https.ssl) {
        SSL_shutdown(https.ssl);
        SSL_free(https.ssl);
    }

    if(https.sock > 0) {
        close(https.sock);
    }

    if(https.ctx) {
        SSL_CTX_free(https.ctx);
    }

    return 0;
}

int httpsRequestCall(HTTPS& https)
{
    HttpResponse& res = https.res;
    res.status_code = -1;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    const SSL_METHOD* method = TLS_client_method();
    https.ctx = SSL_CTX_new(method);
    if(!https.ctx) {
        res.error_msg = "SSL 컨텍스트 생성 실패";
        UTILLOG(LOGLV_ERR, __FUNCTION__, res.error_msg.c_str());
        httpsFinish(https);
        return res.status_code;
    }

    https.sock = socket(AF_INET, SOCK_STREAM, 0);

    // timeout
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(https.sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    struct hostent* host_info = gethostbyname(https.host.c_str());
    if(!host_info) {
        res.error_msg = "호스트 이름 확인 실패: " + https.host;
        UTILLOG(LOGLV_ERR, __FUNCTION__, res.error_msg.c_str());
        httpsFinish(https);
        return res.status_code;
    }

    https.server_addr.sin_family = AF_INET;
    https.server_addr.sin_port = htons(https.port);
    https.server_addr.sin_addr = *((struct in_addr*)host_info->h_addr);
    memset(&(https.server_addr.sin_zero), 0, 8);

    UTILLOG(LOGLV_DBG, __FUNCTION__, "http ssl connect[%s:%d]"
        , inet_ntoa(https.server_addr.sin_addr), https.port);

    if(connect(https.sock, (struct sockaddr*)&https.server_addr, sizeof(struct sockaddr)) == -1) {
        res.error_msg = "TCP 서버 연결 실패";
        UTILLOG(LOGLV_ERR, __FUNCTION__, res.error_msg.c_str());
        httpsFinish(https);
        return res.status_code;
    }

    https.ssl = SSL_new(https.ctx);
    SSL_set_fd(https.ssl, https.sock);
    SSL_set_tlsext_host_name(https.ssl, https.host.c_str());

    if(SSL_connect(https.ssl) <= 0) {
        res.error_msg = "SSL 핸드셰이크 실패";
        UTILLOG(LOGLV_ERR, __FUNCTION__, res.error_msg.c_str());
        httpsFinish(https);
        return res.status_code;
    }

    // set request
    std::ostringstream request_stream;
    switch(https.request_type) {
        case REQUEST_TYPE_GET:
        {
            // Header check ///////////////////////////////////////////////////////////
            if(https.headers.find("Host") == https.headers.end()) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "Missing required HTTP header: Host");
                httpsFinish(https);
                return -1;
            }

            if(https.headers.find("User-Agent") == https.headers.end()) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "Missing required HTTP header: User-Agent");
                httpsFinish(https);
                return -1;
            }

            if(https.headers.find("Connection") == https.headers.end()) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "Missing required HTTP header: Connection");
                httpsFinish(https);
                return -1;
            }
            ///////////////////////////////////////////////////////////////////////////
            
            // set header
            request_stream << "GET " << https.path << " HTTP/1.0\r\n";

            for(const auto& header : https.headers) {
                request_stream << header.first << ": " << header.second << "\r\n";
            }
            request_stream << "\r\n";
        }
        break;
        case REQUEST_TYPE_POST:
        {
            // Header check ///////////////////////////////////////////////////////////
            if(https.headers.find("Host") == https.headers.end()) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "Missing required HTTP header: Host");
                httpsFinish(https);
                return -1;
            }   
            
            if(https.headers.find("User-Agent") == https.headers.end()) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "Missing required HTTP header: User-Agent");
                httpsFinish(https);
                return -1;
            }   
            
            if(https.headers.find("Connection") == https.headers.end()) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "Missing required HTTP header: Connection");
                httpsFinish(https);
                return -1;
            }

            if(https.headers.find("Content-Type") == https.headers.end()) {
                UTILLOG(LOGLV_ERR, __FUNCTION__, "Missing required HTTP header: Content-Type");
                httpsFinish(https);
                return -1;
            }
            ///////////////////////////////////////////////////////////////////////////
            
            // set body
            std::ostringstream body_stream;
            switch(https.body_type) {
                case BODY_TYPE_ENCODED:
                {
                    bool is_first = true;
                    for(const auto& item : https.bodys) {
                        body_stream << ((!is_first) ? "&" : "")
                                    << item.first
                                    << "="
                                    << item.second;
                        is_first = false;
                    }
                }
                break;
                case BODY_TYPE_JSON:
                {
                    json body_json;
                    for(const auto& item : https.bodys) {
                        body_json[item.first] = item.second;
                    }

                    body_stream << body_json.dump();
                }
                break;
                default:
                {
                    UTILLOG(LOGLV_ERR, __FUNCTION__, "Unkown body_type error.");
                    httpsFinish(https);
                    return -1;
                }
            }

            // set header
            request_stream << "POST " << https.path << " HTTP/1.0\r\n";
            
            for(const auto& item : https.headers) {
                request_stream << item.first << ": " << item.second << "\r\n";
            }
            request_stream << "Content-Length: " << body_stream.str().length() << "\r\n";
            request_stream << "\r\n";
            request_stream << body_stream.str();
        }
        break;
        default:
        {
            UTILLOG(LOGLV_ERR, __FUNCTION__, "Unkown request_type error.");
            httpsFinish(https);
            return -1;
        }
    }
    std::string request = request_stream.str();

    // Request check ///////////////////////////////////////////////////////////
    {
        std::istringstream iss(request);
        std::string line;

        UTILLOG(LOGLV_DBG, __FUNCTION__, "== [https request]====================");
        while(std::getline(iss, line)) {
            if(!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if(!line.empty()) {
                UTILLOG(LOGLV_DBG, __FUNCTION__, "%s", line.c_str());
            }
        }
        UTILLOG(LOGLV_DBG, __FUNCTION__, "======================================");
    }
    ///////////////////////////////////////////////////////////////////////////
    SSL_write(https.ssl, request.c_str(), request.length());

    // Receive response ////////////////////////////////////////////////////////
    {
        char buffer[4096];
        int bytes_read;
        std::string raw_response = "";

        while((bytes_read = SSL_read(https.ssl, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            raw_response += buffer;
        }

        size_t header_end = raw_response.find("\r\n\r\n");
        if(header_end != std::string::npos) {
            res.body = raw_response.substr(header_end + 4);

            size_t space1 = raw_response.find(' ');
            if(space1 != std::string::npos) {
                size_t space2 = raw_response.find(' ', space1 + 1);
                if(space2 != std::string::npos) {
                    try {
                        res.status_code = std::stoi(raw_response.substr(space1 + 1, space2 - space1 - 1));
                    }
                    catch(...) {
                        res.status_code = -1;
                    }
                }
            }
        }
        else {
            res.body = raw_response;
        }
    }
    ///////////////////////////////////////////////////////////////////////////

    httpsFinish(https);
    return res.status_code;
}

int httpsGetParseJson(const std::string& body, json& output)
{
    if(body.empty()) {
        return -1;
    }

    size_t start_pos = body.find('{');
    size_t end_pos   = body.rfind('}');

    if((start_pos == std::string::npos) || (end_pos == std::string::npos) || (start_pos > end_pos)) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "Body에서 JSON 형식을 찾을 수 없습니다.");
        return -1;
    }

    try {
        std::string pure_json = body.substr(start_pos, end_pos - start_pos + 1);
        output = json::parse(pure_json);
        return 0;
    }
    catch(const json::parse_error& e) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "JSON 파싱 에러: %s", e.what());
        return -1;
    }
}

