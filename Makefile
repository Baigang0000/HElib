# Makefile for Binary CKKS Homomorphic Encryption Implementation
# Author: Binary CKKS Implementation Team
# Description: Builds the binary CKKS scheme using HElib

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -O3 -g -Wall -Wextra -pthread

# Library paths - adjust these based on your HElib installation
HELIB_DIR = /usr/local
HELIB_INCLUDE = $(HELIB_DIR)/include
HELIB_LIB = $(HELIB_DIR)/lib

# NTL and GMP paths - adjust if needed
NTL_INCLUDE = /usr/local/include
NTL_LIB = /usr/local/lib
GMP_LIB = /usr/local/lib

# Include directories
INCLUDES = -I$(HELIB_INCLUDE) -I$(NTL_INCLUDE) -I.

# Library linking
LIBS = -L$(HELIB_LIB) -L$(NTL_LIB) -L$(GMP_LIB) -lhelib -lntl -lgmp -lm

# Source files
SOURCES = binary_ckks.cpp
HEADERS = binary_ckks.h
EXAMPLE_SOURCES = binary_ckks_example.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)
EXAMPLE_OBJECTS = $(EXAMPLE_SOURCES:.cpp=.o)

# Executable names
EXAMPLE_EXEC = binary_ckks_demo
LIBRARY = libbinaryckks.a

# Default target
all: $(EXAMPLE_EXEC) $(LIBRARY)

# Build the main example
$(EXAMPLE_EXEC): $(OBJECTS) $(EXAMPLE_OBJECTS)
	@echo "Linking $(EXAMPLE_EXEC)..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "Build successful! Run with ./$(EXAMPLE_EXEC)"

# Build static library
$(LIBRARY): $(OBJECTS)
	@echo "Creating static library $(LIBRARY)..."
	ar rcs $@ $^
	@echo "Library created: $(LIBRARY)"

# Compile source files
%.o: %.cpp $(HEADERS)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(OBJECTS) $(EXAMPLE_OBJECTS) $(EXAMPLE_EXEC) $(LIBRARY)
	@echo "Clean complete"

# Install library and headers (requires sudo)
install: $(LIBRARY)
	@echo "Installing binary CKKS library..."
	sudo cp $(LIBRARY) /usr/local/lib/
	sudo cp $(HEADERS) /usr/local/include/
	sudo ldconfig
	@echo "Installation complete"

# Uninstall library and headers (requires sudo)
uninstall:
	@echo "Uninstalling binary CKKS library..."
	sudo rm -f /usr/local/lib/$(LIBRARY)
	sudo rm -f /usr/local/include/$(HEADERS)
	sudo ldconfig
	@echo "Uninstall complete"

# Run the example
run: $(EXAMPLE_EXEC)
	@echo "Running Binary CKKS demo..."
	./$(EXAMPLE_EXEC)

# Check if HElib is properly installed
check-deps:
	@echo "Checking dependencies..."
	@echo "Looking for HElib headers in $(HELIB_INCLUDE)..."
	@if [ -f "$(HELIB_INCLUDE)/helib/helib.h" ]; then \
		echo "✓ HElib headers found"; \
	else \
		echo "✗ HElib headers not found. Please install HElib or adjust HELIB_INCLUDE path"; \
		exit 1; \
	fi
	@echo "Looking for HElib library in $(HELIB_LIB)..."
	@if [ -f "$(HELIB_LIB)/libhelib.a" ] || [ -f "$(HELIB_LIB)/libhelib.so" ]; then \
		echo "✓ HElib library found"; \
	else \
		echo "✗ HElib library not found. Please install HElib or adjust HELIB_LIB path"; \
		exit 1; \
	fi
	@echo "Looking for NTL library..."
	@if ldconfig -p | grep -q libntl; then \
		echo "✓ NTL library found"; \
	else \
		echo "✗ NTL library not found. Please install NTL"; \
		exit 1; \
	fi
	@echo "Looking for GMP library..."
	@if ldconfig -p | grep -q libgmp; then \
		echo "✓ GMP library found"; \
	else \
		echo "✗ GMP library not found. Please install GMP"; \
		exit 1; \
	fi
	@echo "All dependencies check passed!"

# Build with debug information
debug: CXXFLAGS += -DDEBUG -g3 -O0
debug: $(EXAMPLE_EXEC)
	@echo "Debug build complete"

# Build with maximum optimization
release: CXXFLAGS += -DNDEBUG -O3 -march=native
release: $(EXAMPLE_EXEC)
	@echo "Release build complete"

# Create documentation (requires doxygen)
docs:
	@echo "Generating documentation..."
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile; \
		echo "Documentation generated in docs/"; \
	else \
		echo "Doxygen not found. Please install doxygen to generate documentation"; \
	fi

# Run unit tests (when implemented)
test: $(EXAMPLE_EXEC)
	@echo "Running tests..."
	./$(EXAMPLE_EXEC)

# Performance benchmark
benchmark: release
	@echo "Running performance benchmark..."
	./$(EXAMPLE_EXEC)

# Memory check with valgrind
memcheck: debug
	@if command -v valgrind >/dev/null 2>&1; then \
		echo "Running memory check with valgrind..."; \
		valgrind --leak-check=full --show-leak-kinds=all ./$(EXAMPLE_EXEC); \
	else \
		echo "Valgrind not found. Please install valgrind for memory checking"; \
	fi

# Static analysis with cppcheck
analyze:
	@if command -v cppcheck >/dev/null 2>&1; then \
		echo "Running static analysis with cppcheck..."; \
		cppcheck --enable=all --std=c++17 $(SOURCES) $(EXAMPLE_SOURCES); \
	else \
		echo "Cppcheck not found. Please install cppcheck for static analysis"; \
	fi

# Format code with clang-format
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "Formatting code with clang-format..."; \
		clang-format -i $(SOURCES) $(HEADERS) $(EXAMPLE_SOURCES); \
		echo "Code formatting complete"; \
	else \
		echo "clang-format not found. Please install clang-format for code formatting"; \
	fi

# Print help
help:
	@echo "Binary CKKS Homomorphic Encryption Makefile"
	@echo "============================================"
	@echo ""
	@echo "Available targets:"
	@echo "  all         - Build everything (default)"
	@echo "  $(EXAMPLE_EXEC) - Build the demo executable"
	@echo "  $(LIBRARY)  - Build the static library"
	@echo "  clean       - Remove build artifacts"
	@echo "  install     - Install library and headers (requires sudo)"
	@echo "  uninstall   - Remove installed files (requires sudo)"
	@echo "  run         - Run the demo program"
	@echo "  check-deps  - Check if all dependencies are installed"
	@echo "  debug       - Build with debug information"
	@echo "  release     - Build with maximum optimization"
	@echo "  docs        - Generate documentation (requires doxygen)"
	@echo "  test        - Run tests"
	@echo "  benchmark   - Run performance benchmark"
	@echo "  memcheck    - Run memory check with valgrind"
	@echo "  analyze     - Run static analysis with cppcheck"
	@echo "  format      - Format code with clang-format"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Configuration:"
	@echo "  HELIB_DIR   = $(HELIB_DIR)"
	@echo "  NTL_LIB     = $(NTL_LIB)"
	@echo "  GMP_LIB     = $(GMP_LIB)"
	@echo ""
	@echo "To customize paths, edit the Makefile or use:"
	@echo "  make HELIB_DIR=/path/to/helib [target]"

# Make sure certain targets are not files
.PHONY: all clean install uninstall run check-deps debug release docs test benchmark memcheck analyze format help

# Show variables for debugging
print-vars:
	@echo "CXX       = $(CXX)"
	@echo "CXXFLAGS  = $(CXXFLAGS)"
	@echo "INCLUDES  = $(INCLUDES)"
	@echo "LIBS      = $(LIBS)"
	@echo "SOURCES   = $(SOURCES)"
	@echo "OBJECTS   = $(OBJECTS)"

# Quick setup for Ubuntu/Debian systems (requires sudo)
ubuntu-setup:
	@echo "Setting up dependencies for Ubuntu/Debian..."
	sudo apt-get update
	sudo apt-get install -y build-essential cmake git
	sudo apt-get install -y libgmp-dev libntl-dev
	@echo "Basic dependencies installed. You still need to install HElib manually."
	@echo "Please visit: https://github.com/homenc/HElib for HElib installation instructions."

# Quick setup for macOS systems (requires Homebrew)
macos-setup:
	@echo "Setting up dependencies for macOS..."
	@if command -v brew >/dev/null 2>&1; then \
		brew install gmp ntl cmake; \
		echo "Basic dependencies installed. You still need to install HElib manually."; \
		echo "Please visit: https://github.com/homenc/HElib for HElib installation instructions."; \
	else \
		echo "Homebrew not found. Please install Homebrew first: https://brew.sh/"; \
	fi