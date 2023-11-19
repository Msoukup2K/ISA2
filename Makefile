CXX = g++
CXXFLAGS = -std=c++17 -MMD -Wall -Wextra -pedantic

.PHONY: all clean

# Directories
CLIENT_DIR = client
SERVER_DIR = server
UTILS_DIR = utils

# Executables
CLIENT_TARGET = tftp-client
SERVER_TARGET = tftp-server

# Source and header files
CLIENT_SRCS = $(wildcard $(CLIENT_DIR)/*.cpp) $(wildcard $(UTILS_DIR)/*.cpp)
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
CLIENT_DEPS = $(CLIENT_OBJS:.o=.d)

SERVER_SRCS = $(wildcard $(SERVER_DIR)/*.cpp) $(wildcard $(UTILS_DIR)/*.cpp)
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
SERVER_DEPS = $(SERVER_OBJS:.o=.d)

UTILS_SRCS = $(wildcard $(UTILS_DIR)/*.cpp)
UTILS_OBJS = $(UTILS_SRCS:.cpp=.o)
UTILS_DEPS = $(UTILS_OBJS:.o=.d)

# Include
INC = -I $(UTILS_DIR)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) $(INC) $^ -o $@

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) $(INC) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

# Clean
clean:
	rm -f $(CLIENT_OBJS) $(CLIENT_DEPS) $(SERVER_OBJS) $(SERVER_DEPS) $(UTILS_OBJS) $(UTILS_DEPS) $(CLIENT_TARGET) $(SERVER_TARGET)

.PHONY: all clean

all: $(CLIENT_TARGET) $(SERVER_TARGET)

# Dependencies
-include $(CLIENT_DEPS)
-include $(SERVER_DEPS)
-include $(UTILS_DEPS)

