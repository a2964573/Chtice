#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <format>
#include <filesystem>
#include <chrono>
#include <cstdarg>
#include <cstdlib>
#include <unistd.h>
#include <algorithm>
#include <vector>
#include <csignal>
#include <atomic>
#include <thread>

#include "util.h"

#define STATE_CLOSE    0
#define STATE_OPEN     1
#define STATE_OPENNING 2

#define ABSOLUTE_PATH_CONF "/home/chtice/prj/conf"

typedef struct _COMMON {
    std::atomic_bool* is_running;
} COMMON;

void commonRegisterSignalHandler(std::atomic_bool* is_running);
void commonProcessWaitSecond(int second);

