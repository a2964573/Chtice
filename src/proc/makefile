CXX = g++

APP_DIR = $(PROJECT_APP)
PROC_BUILD_DIR = $(PROJECT_BUILD)/proc
LIB_BUILD_DIR = $(PROJECT_BUILD)/lib
LIB_SRC_DIR = $(PROJECT_ROOT)/src/lib

CXXFLAGS = -Wall -g -std=c++20 -I$(LIB_SRC_DIR)

COMMON_LDFLAGS = -L$(LIB_BUILD_DIR) -lutil -Wl,-rpath=$(LIB_BUILD_DIR)
BOT_LDFLAGS = $(COMMON_LDFLAGS) -L/usr/lib64/mysql -ldpp -pthread -lmysqlclient -ldbmysql -lhttps
DAEMON_LDFLAGS = $(COMMON_LDFLAGS) -L/usr/lib64/mysql -lssl -lcrypto -pthread -lmysqlclient -ldbmysql -lhttps
BATCH_LDFLAGS = $(COMMON_LDFLAGS) -L/usr/lib64/mysql -lssl -lcrypto -pthread -lmysqlclient -ldbmysql -lhttps

TARGETS = $(APP_DIR)/chtice_daemon $(APP_DIR)/chtice_bot $(APP_DIR)/chtice_batch

all: $(TARGETS)

$(PROC_BUILD_DIR):
	mkdir -p $(PROC_BUILD_DIR)

$(APP_DIR):
	mkdir -p $(APP_DIR)

# common
$(PROC_BUILD_DIR)/common.o: common.cpp common.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# dbquery
$(PROC_BUILD_DIR)/dbquery.o: dbquery.cpp dbquery.h common.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# chtice_daemon
$(PROC_BUILD_DIR)/chtice_daemon.o: chtice_daemon.cpp chtice_daemon.h dbquery.h common.h | $(PROC_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(APP_DIR)/chtice_daemon: $(PROC_BUILD_DIR)/chtice_daemon.o $(PROC_BUILD_DIR)/dbquery.o $(PROC_BUILD_DIR)/common.o | $(APP_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(DAEMON_LDFLAGS)

# chtice_bot
$(PROC_BUILD_DIR)/chtice_bot.o: chtice_bot.cpp chtice_bot.h dbquery.h common.h | $(PROC_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(APP_DIR)/chtice_bot: $(PROC_BUILD_DIR)/chtice_bot.o $(PROC_BUILD_DIR)/dbquery.o $(PROC_BUILD_DIR)/common.o | $(APP_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BOT_LDFLAGS)

# chtice_batch
$(PROC_BUILD_DIR)/chtice_batch.o: chtice_batch.cpp chtice_batch.h dbquery.h common.h | $(PROC_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(APP_DIR)/chtice_batch: $(PROC_BUILD_DIR)/chtice_batch.o $(PROC_BUILD_DIR)/dbquery.o $(PROC_BUILD_DIR)/common.o | $(APP_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(BATCH_LDFLAGS)

clean:
	rm -f $(PROC_BUILD_DIR)/*.o $(TARGETS)

