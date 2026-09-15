#include "common.h"


COMMON _common = {};

void _commonSignalHandler(int signum) {
    if(signum == SIGINT || signum == SIGTERM) {
        *_common.is_running = false;
    }
}

void commonRegisterSignalHandler(std::atomic_bool* is_running) {
    _common.is_running = is_running;

    std::signal(SIGINT , _commonSignalHandler);
    std::signal(SIGTERM, _commonSignalHandler);
}

void commonProcessWaitSecond(int second)
{
    int sec;
    for(sec = 0; ((sec < 30) && (_common.is_running)); sec++) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

