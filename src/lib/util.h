#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <cstdarg>
#include <cstdlib>
#include <format>
#include <unistd.h>

std::string utilGetEnvVar(const std::string& file_path, const std::string& file_name, const std::string& key);
int utilTrim(std::string& str);
std::string utilGetIniVar(const std::string& file_path, const std::string& file_name, const std::string& section, const std::string& key);

#define LOGLV_NOR 0
#define LOGLV_DBG 1
#define LOGLV_ERR 2

#define EXPORT_PROJECT_LOG "PROJECT_LOG"
#define ABSOLUTE_PATH_LOG  "/home/chtice/prj/log"

#define UTILLOG(level, func, format, ...) utilLogging(__FILE__, func, __LINE__, level, format, ##__VA_ARGS__)
int utilLogging(const char* file_name, const char* func_name, const int line, int level, const char* format, ...);

int utilExecutePopenCommand(const char* cmd, std::string& output);

