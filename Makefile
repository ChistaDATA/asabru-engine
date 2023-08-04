# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -std=c++11

# Directories
SRC_DIR := src
BUILD_DIR := build
LIB_DIR := lib

# List of source files
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/**/*.cpp)

# Generate object file names from source files
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))
OBJ_FILES := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(OBJ_FILES))

# Name of the static library
LIBRARY_NAME := asabru-engine.a

# Targets
all: $(LIB_DIR)/$(LIBRARY_NAME)

$(LIB_DIR)/$(LIBRARY_NAME): $(OBJ_FILES)
	ar rcs $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%: $(SRC_DIR)/%
	mkdir -p $(@D)
	cp $< $@

clean:
	rm -rf $(BUILD_DIR)/* $(LIB_DIR)/$(LIBRARY_NAME)

.PHONY: all clean