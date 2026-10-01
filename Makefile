# Space Game build file (macOS, Homebrew raylib)
# Usage: make | make run | make test | make BUILD=debug | make clean

TARGET    := space-game
BUILD     ?= release
SRC_DIR   := src
BUILD_DIR := build/$(BUILD)
BIN_DIR   := bin

CXX       ?= clang++
CXXSTD    := -std=c++20
WARNINGS  := -Wall -Wextra -Wpedantic -Wshadow
INCLUDES  := -I$(SRC_DIR)

ifeq ($(BUILD),debug)
OPT := -O0 -g -DDEBUG
else
OPT := -O2 -DNDEBUG
endif

RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib 2>/dev/null)
ifeq ($(RAYLIB_LIBS),)
BREW_PREFIX   := $(shell brew --prefix raylib 2>/dev/null)
RAYLIB_CFLAGS := -I$(BREW_PREFIX)/include
RAYLIB_LIBS   := -L$(BREW_PREFIX)/lib -lraylib
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
PLATFORM_LIBS := -framework IOKit -framework Cocoa -framework OpenGL
endif

CXXFLAGS := $(CXXSTD) $(WARNINGS) $(OPT) $(INCLUDES) $(RAYLIB_CFLAGS) -MMD -MP
LDLIBS   := $(RAYLIB_LIBS) $(PLATFORM_LIBS)

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all run test clean rebuild

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJS)
	mkdir -p $(@D)
	$(CXX) $(OBJS) -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(BIN_DIR)/$(TARGET)

test:
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXSTD) $(WARNINGS) $(INCLUDES) $(RAYLIB_CFLAGS) tests/physics_test.cpp src/physics/PhysicsWorld.cpp -o $(BUILD_DIR)/physics_test
	./$(BUILD_DIR)/physics_test

clean:
	rm -rf build $(BIN_DIR)

rebuild: clean all

-include $(DEPS)
