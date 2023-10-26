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

SERVER_SRCS = $(wildcard $(SERVER_DIR)/*.cpp) $(wildcard $(UTILS_DIR)/*.cpp)
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)

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
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(CLIENT_TARGET) $(SERVER_TARGET)

.PHONY: all clean

all: $(CLIENT_TARGET) $(SERVER_TARGET)

# Dependencies
$(CLIENT_OBJS): $(CLIENT_SRCS)
$(SERVER_OBJS): $(SERVER_SRCS)

