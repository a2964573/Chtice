CXX = g++
CXXFLAGS = -Wall -g -std=c++20 -fPIC

LIB_BUILD_DIR = $(PROJECT_BUILD)/lib

TARGET_UTIL   = $(LIB_BUILD_DIR)/libutil.so
TARGET_SERVER = $(LIB_BUILD_DIR)/libserver.so
TARGET_CLIENT = $(LIB_BUILD_DIR)/libclient.so
TARGET_HTTPS  = $(LIB_BUILD_DIR)/libhttps.so
TARGET_DBMYSQL= $(LIB_BUILD_DIR)/libdbmysql.so

TARGETS = $(TARGET_UTIL) $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_HTTPS) $(TARGET_DBMYSQL)

all: $(TARGETS)

$(LIB_BUILD_DIR):
	mkdir -p $(LIB_BUILD_DIR)

# util
$(LIB_BUILD_DIR)/util.o: util.cpp util.h | $(LIB_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET_UTIL): $(LIB_BUILD_DIR)/util.o
	$(CXX) -shared -o $@ $^

# server
$(LIB_BUILD_DIR)/server.o: server.cpp server.h util.h | $(LIB_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET_SERVER): $(LIB_BUILD_DIR)/server.o
	$(CXX) -shared -o $@ $^

# client
$(LIB_BUILD_DIR)/client.o: client.cpp client.h util.h | $(LIB_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET_CLIENT): $(LIB_BUILD_DIR)/client.o
	$(CXX) -shared -o $@ $^

# https
$(LIB_BUILD_DIR)/https.o: https.cpp https.h util.h | $(LIB_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET_HTTPS): $(LIB_BUILD_DIR)/https.o
	$(CXX) -shared -o $@ $^

# dbmysql
$(LIB_BUILD_DIR)/dbmysql.o: dbmysql.cpp dbmysql.h util.h | $(LIB_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET_DBMYSQL): $(LIB_BUILD_DIR)/dbmysql.o
	$(CXX) -shared -o $@ $^

clean:
	rm -f $(LIB_BUILD_DIR)/*.o $(LIB_BUILD_DIR)/*.so

