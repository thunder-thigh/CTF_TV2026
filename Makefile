# -- Compiler --
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17

# -- Directories --
SRC_DIR := src
BLD_DIR := build

# -- Sources --
SERVICE_SRCS := $(SRC_DIR)/register.cpp	\
				$(SRC_DIR)/CTF_Server.cpp \
				$(SRC_DIR)/DB_operations.cpp \
				$(SRC_DIR)/hash.cpp	\
				$(SRC_DIR)/rate_limit.cpp

# -- Objects --
SERVICE_OBJS := $(BLD_DIR)/register.o	\
				$(BLD_DIR)/CTF_Server.o \
				$(BLD_DIR)/DB_operations.o \
				$(BLD_DIR)/hash.o		\
				$(BLD_DIR)/rate_limit.o

OBJS := $(SERVICE_OBJS)
BIN := CTF_Server

# -- Targets --
.PHONY: all build run

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^  -l sqlite3 -lssl -lcrypto
$(BLD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BLD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BLD_DIR):
	mkdir -p $(BLD_DIR)

run: all
	./$(BIN)

clean:
	rm -rf $(BLD_DIR) $(BIN)
