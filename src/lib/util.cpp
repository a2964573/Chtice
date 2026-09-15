#include "util.h"


std::string utilGetEnvVar(const std::string& file_path, const std::string& file_name, const std::string& key)
{
    std::filesystem::path path(file_path);
    std::filesystem::path env_file = path / file_name;
    std::ifstream ifs(env_file);
    if(!ifs.is_open()) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, ".env 파일을 찾을 수 없습니다.");
        return "";
    }

    std::string line;
    size_t delimiter_pos = 0;
    while(std::getline(ifs, line)) {
        if(line.empty() || line[0] == '#') {
            continue;
        }

        delimiter_pos = line.find('=');
        if(delimiter_pos != std::string::npos) {
            std::string current_key = line.substr(0, delimiter_pos);
            if(current_key == key) {
                return line.substr(delimiter_pos + 1);
            }
        }
    }
    
    UTILLOG(LOGLV_ERR, __FUNCTION__, ".env 파일에서 %s 키를 찾을 수 없습니다.", key.c_str());
    return "";
}

int utilTrim(std::string& str)
{
    if(str.empty()) {
        return -1;
    }

    int first_pos = -1;
    int last_pos  =  0;
    int len = str.length();
    int pos;
    char c;
    for(pos = 0; pos < len; pos++) {
        c = str[pos];
        if((c != 0x20) && (c != '\t') && (c != '\r') && (c != '\n')) {
            if(first_pos == -1) {
                first_pos = pos;
            }
            last_pos = pos;
        }
    }

    if(first_pos >= 0) {
        str = str.substr(first_pos, last_pos - first_pos + 1);
    }
    else {
        str.clear();
    }

    return str.length();
}

std::string utilGetIniVar(
    const std::string& file_path,
    const std::string& file_name,
    const std::string& section,
    const std::string& key
)
{
    std::filesystem::path path(file_path);
    std::filesystem::path ini_file = path / file_name;
    std::ifstream file(ini_file);
    if(!file.is_open()) {
        return "";
    }

    std::string line;
    std::string current_section = "";

    size_t delim_pos;
    std::string current_key;
    std::string var;
    while(std::getline(file, line)) {
        if(utilTrim(line) <= 0) {
            continue;
        }

        if((line[0] == ';') || (line[0] == '#')) {
            continue;
        }

        if((line.front() == '[') && (line.back() == ']')) {
            current_section = line.substr(1, line.length() - 2);
            utilTrim(current_section);
            continue;
        }

        if(current_section == section) {
            delim_pos = line.find('=');
            if(delim_pos == std::string::npos) {
                continue;
            }

            current_key = line.substr(0, delim_pos);
            var = line.substr(delim_pos + 1);

            utilTrim(current_key);
            utilTrim(var);

            if(current_key == key) {
                return var;
            }
            else {
                current_key = "";
                var = "";
            }
        }
    }

    return "";
}

int utilLogging(const char* file_name, const char* func_name, const int line, int level, const char* format, ...)
{
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;

    struct tm tm_buf;
    localtime_r(&now_c, &tm_buf);

    std::string date = std::format("{:04d}{:02d}{:02d}", tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday);
    std::string time = std::format("{:02d}:{:02d}:{:02d}.{:06d}", tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec, now_ms.count());

    const char* level_str = "UNK";
    switch(level) {
        case LOGLV_NOR:
        {
            level_str = "NOR";
        }
        break;
        case LOGLV_DBG:
        {
            level_str = "DBG";
        }
        break;
        case LOGLV_ERR:
        {
            level_str = "ERR";
        }
        break;
    }

    int msg_len = 0;
    va_list args;
    va_list args_copy;

    va_start(args, format);
    va_copy(args_copy, args);
    msg_len = vsnprintf(nullptr, 0, format, args);
    va_end(args);

    std::string user_msg(msg_len, '\0');
    vsnprintf(user_msg.data(), msg_len + 1, format, args_copy);
    va_end(args_copy);

    std::filesystem::path p(file_name);
    std::string file_nm = p.stem().string(); 

    std::string log_line = std::format("{}|{}|{:<8}|{:<16}|{:<20}|{:<4}|{}|{}",
        date, time, getpid(), file_nm, func_name, line, level_str, user_msg);

    char* project_log = std::getenv(EXPORT_PROJECT_LOG);
    std::filesystem::path log_path = std::filesystem::path(project_log ? project_log : ABSOLUTE_PATH_LOG) / date;
    if(!std::filesystem::exists(log_path)) {
        std::filesystem::create_directories(log_path);
    }

    std::filesystem::path log_file = log_path / std::format("{}.{}.log", file_nm, date);
    std::ofstream ofs(log_file, std::ios::app);
    if(ofs.is_open()) {
        ofs << log_line << '\n';
        ofs.flush();
    }

    if(level == LOGLV_ERR) {
        std::filesystem::path err_file = log_path / std::format("error.{}.log", date);
        std::ofstream efs(err_file, std::ios::app);
        if(efs.is_open()) {
            efs << log_line << '\n';
            efs.flush();
        }
    }

    return log_line.length();
}

int utilExecutePopenCommand(const char* cmd, std::string& output)
{
    std::array<char, 256> buffer;
    std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen(cmd, "r"), pclose);

    if(!pipe) {
        UTILLOG(LOGLV_ERR, __FUNCTION__, "popen failed!");
        return -1;
    }

    output.clear();
    while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        output += buffer.data();
    }

    return output.length();
}

