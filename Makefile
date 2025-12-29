# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3

OUTPUT = output # store the exec

# Targets
TARGETS = $(OUTPUT)/nnue_engine


# Source files for each phase
Resources = src/NNUE_Engine.cpp \
				src/nnue.cpp

# Default target
all: $(OUTPUT) $(TARGETS) 

$(OUTPUT):
	mkdir -p $(OUTPUT)

# Phase 1 executable - compile directly without .o files
$(OUTPUT)/nnue_engine: $(Resources)
	@echo "Building NNUE Engine..."
	mkdir -p output
	$(CXX) $(CXXFLAGS) -I./include $(Resources) -o $@
	@echo "✓ NNUE_Engine executable created: $@"	

nnue_engine: $(OUTPUT)/nnue_engine
	

# Test targets with proper paths
run_engine: $(OUTPUT)/nnue_engine
	@echo "Running engine tests..."
	./$(OUTPUT)/nnue_engine

# Help target
help:
	@echo "════════════════════════════════════════════════════════"
	@echo "  Makefile Help - Chess Engine"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	@echo "Build Targets:"
	@echo "  make nnue_engine           - Build NNUE_Engine"
	@echo ""
	@echo "Test Targets:"
	@echo "  make run_engine      - Run the Engine executable"
	@echo ""
	@echo ""
# 	@echo "Maintenance:"
# 	@echo "  make clean            - Remove all executables"
# 	@echo "  make check_deps       - Check if dependencies are installed"
# 	@echo "  make install_deps     - Install required dependencies"
# 	@echo ""
# 	@echo "Executables will be created in: $(BIN_DIR)/"
# 	@echo "Test results will be saved in: output/"
	@echo ""

# .PHONY: all phase1 phase2 phase3 clean test run_phase1 run_phase2 run_phase3_normal run_phase3_iitb \
#         rebuild generate_test_phase3 run_generated_test_phase3 install_deps check_deps help