# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3

# Output directory and executable
OUTPUT_DIR = output
EXEC = $(OUTPUT_DIR)/nnue_engine

# Sources
SOURCES = src/NNUE_Engine.cpp \
          src/nnue.cpp

# Default target
all: $(EXEC)

# Create output directory
$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Build executable
$(EXEC): $(SOURCES) | $(OUTPUT_DIR)
	@echo "Building NNUE Engine..."
	$(CXX) $(CXXFLAGS) -I./include $(SOURCES) -o $@
	@echo "✓ NNUE_Engine executable created: $@"

# Convenience target
nnue_engine: $(EXEC)

# Run target
run_engine: $(EXEC)
	@echo "Running engine..."
	./$(EXEC)

# Help
help:
	@echo "Build:"
	@echo "  make nnue_engine"
	@echo ""
	@echo "Run:"
	@echo "  make run_engine"

.PHONY: all nnue_engine run_engine help
