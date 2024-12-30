# Compiler and flags
CXX = g++
CXXFLAGS = -g -Iinclude -std=c++17

# Libraries
LIB = -lSDL2 -lSDL2_image -lGL -ldl

# Output binary
BIN = voxels

# Directories
SRC_DIR = src
BUILD_DIR = build
GLAD_DIR = $(SRC_DIR)/glad
RENDERING_DIR = $(SRC_DIR)/rendering
PHYSICS_DIR = $(SRC_DIR)/physics
GAMEPLAY_DIR = $(SRC_DIR)/gameplay
WORLD_DIR = $(SRC_DIR)/world
UTILITIES_DIR = $(SRC_DIR)/utilities
TEST_DIR = tests

# Source files
GLAD_SRC = $(wildcard $(GLAD_DIR)/*.c)
RENDERING_SRC = $(wildcard $(RENDERING_DIR)/*.cpp)
PHYSICS_SRC = $(wildcard $(PHYSICS_DIR)/*.cpp)
GAMEPLAY_SRC = $(wildcard $(GAMEPLAY_DIR)/*.cpp)
WORLD_SRC = $(wildcard $(WORLD_DIR)/*.cpp)
UTILITIES_SRC = $(wildcard $(UTILITIES_DIR)/*.cpp)
SRC = $(GLAD_SRC) $(RENDERING_SRC) $(PHYSICS_SRC) $(GAMEPLAY_SRC) $(WORLD_SRC) $(UTILITIES_SRC)

# Object files
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC)))

# Test files
TEST_SRC = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJ = $(patsubst $(TEST_DIR)/%.cpp, $(BUILD_DIR)/tests/%.o, $(TEST_SRC))
TEST_BIN = $(BUILD_DIR)/test_runner

# Default target
all: $(BIN)

# Linking step
$(BIN): main.cpp $(OBJ)
	$(CXX) -o $@ $^ $(LIB) $(CXXFLAGS)

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test files
$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build test binary
$(TEST_BIN): $(TEST_OBJ) $(OBJ)
	$(CXX) -o $@ $^ $(LIB) $(CXXFLAGS)

# Run the program
run: $(BIN)
	./$(BIN)

# Run tests
test: $(TEST_BIN)
	./$(TEST_BIN)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN)