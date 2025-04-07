# Set ncurses installation path
NCURSES_PATH=~/local

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++14 -I$(NCURSES_PATH)/include
LDFLAGS = -L$(NCURSES_PATH)/lib -lncurses

# Directories
BIN_DIR = bin
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include

TARGET = $(BIN_DIR)/main

OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/game.o $(BUILD_DIR)/startGame.o

SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/game.cpp $(SRC_DIR)/startGame.cpp

all: install-ncurses directories $(TARGET)

# Install ncurses if not already installed
install-ncurses:
    chmod +x installNcurses.sh
    ./installNcurses.sh

directories:
    mkdir -p $(BIN_DIR)
    mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
    $(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp $(INCLUDE_DIR)/game.h
    $(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/game.o: $(SRC_DIR)/game.cpp $(INCLUDE_DIR)/game.h
    $(CXX) $(CXXFLAGS) -c $(SRC_DIR)/game.cpp -o $(BUILD_DIR)/game.o

$(BUILD_DIR)/startGame.o: $(SRC_DIR)/startGame.cpp $(INCLUDE_DIR)/startGame.h
    $(CXX) $(CXXFLAGS) -c $(SRC_DIR)/startGame.cpp -o $(BUILD_DIR)/startGame.o

clean:
    rm -rf $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all clean directories install-ncurses