#!/bin/bash

# CKKS Performance Comparison Experiment Runner
# This script automates building and running the performance comparison

set -e  # Exit on error

# Configuration
BUILD_DIR="benchmark_build"
OUTPUT_DIR="results"
CMAKE_BUILD_TYPE="Release"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_dependencies() {
    print_status "Checking dependencies..."
    
    # Check for required tools
    for cmd in cmake make g++ python3; do
        if ! command -v $cmd &> /dev/null; then
            print_error "Required tool '$cmd' is not installed"
            exit 1
        fi
    done
    
    # Check for required libraries
    if ! ldconfig -p | grep -q "libgmp"; then
        print_warning "GMP library not found in ldconfig, but may still work"
    fi
    
    if ! ldconfig -p | grep -q "libntl"; then
        print_warning "NTL library not found in ldconfig, but may still work"  
    fi
    
    if ! ldconfig -p | grep -q "libhelib"; then
        print_warning "HElib library not found in ldconfig, but may still work"
    fi
    
    # Check Python packages
    print_status "Checking Python dependencies..."
    python3 -c "import pandas, matplotlib, seaborn, numpy" 2>/dev/null || {
        print_error "Required Python packages not found. Install with:"
        echo "pip install pandas matplotlib seaborn numpy"
        exit 1
    }
    
    print_success "All dependencies checked"
}

build_benchmark() {
    print_status "Building CKKS performance comparison benchmark..."
    
    # Create build directory
    if [ -d "$BUILD_DIR" ]; then
        print_status "Cleaning existing build directory..."
        rm -rf "$BUILD_DIR"
    fi
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure and build
    print_status "Configuring with CMake..."
    cmake -f ../CMakeLists_benchmark.txt \
          -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
          .. || {
        print_error "CMake configuration failed"
        exit 1
    }
    
    print_status "Building (this may take a few minutes)..."
    make -j$(nproc) || {
        print_error "Build failed"
        exit 1
    }
    
    cd ..
    print_success "Build completed successfully"
}

run_benchmark() {
    print_status "Running CKKS performance comparison..."
    
    cd "$BUILD_DIR"
    
    # Check if benchmark executable exists
    if [ ! -f "./ckks_performance_comparison" ]; then
        print_error "Benchmark executable not found"
        exit 1
    fi
    
    # Set optimal CPU performance mode if possible
    if command -v cpupower &> /dev/null; then
        print_status "Setting CPU to performance mode..."
        sudo cpupower frequency-set --governor performance 2>/dev/null || {
            print_warning "Could not set CPU performance mode (requires sudo)"
        }
    fi
    
    # Run the benchmark
    print_status "Starting benchmark execution..."
    print_warning "This may take 10-30 minutes depending on your system..."
    
    time ./ckks_performance_comparison || {
        print_error "Benchmark execution failed"
        exit 1
    }
    
    cd ..
    print_success "Benchmark completed successfully"
}

analyze_results() {
    print_status "Analyzing results and generating plots..."
    
    # Create output directory
    mkdir -p "$OUTPUT_DIR"
    
    # Check if CSV files exist
    if ! ls ${BUILD_DIR}/ckks_comparison_*.csv 1> /dev/null 2>&1; then
        print_error "No result CSV files found"
        exit 1
    fi
    
    # Run analysis script
    python3 analyze_results.py \
        --data-dir "$BUILD_DIR" \
        --output-dir "$OUTPUT_DIR" || {
        print_error "Analysis failed"
        exit 1
    }
    
    print_success "Analysis completed"
    print_status "Results saved in: $OUTPUT_DIR/"
}

show_results() {
    print_success "Experiment completed successfully!"
    echo
    echo "Generated files:"
    echo "=================="
    
    # List data files
    echo "📊 Raw Data:"
    for file in ${BUILD_DIR}/ckks_comparison_*.csv; do
        if [ -f "$file" ]; then
            echo "  - $(basename $file)"
        fi
    done
    
    echo
    echo "📈 Analysis Outputs:"
    for file in performance_comparison.pdf detailed_analysis.pdf results_table.tex; do
        if [ -f "$OUTPUT_DIR/$file" ]; then
            echo "  - $OUTPUT_DIR/$file"
        fi
    done
    
    echo
    echo "📝 For your conference paper:"
    echo "  1. Use performance_comparison.pdf as your main figure"
    echo "  2. Include results_table.tex in your LaTeX document" 
    echo "  3. Reference detailed_analysis.pdf for supplementary material"
    echo
    print_success "Ready for publication! 🎓"
}

print_usage() {
    echo "CKKS Performance Comparison Experiment"
    echo "======================================"
    echo
    echo "Usage: $0 [OPTIONS]"
    echo
    echo "Options:"
    echo "  -h, --help        Show this help message"
    echo "  -c, --check       Check dependencies only"
    echo "  -b, --build       Build benchmark only"
    echo "  -r, --run         Run benchmark only (requires existing build)"
    echo "  -a, --analyze     Analyze results only (requires existing results)"
    echo "  --clean           Clean build directory and exit"
    echo
    echo "Default behavior (no options): Check deps, build, run, and analyze"
    echo
    echo "Examples:"
    echo "  $0                 # Full experiment pipeline"
    echo "  $0 --check        # Just check if dependencies are installed"
    echo "  $0 --build --run  # Build and run, but don't analyze"
    echo
}

# Parse command line arguments
SKIP_CHECK=false
SKIP_BUILD=false
SKIP_RUN=false
SKIP_ANALYZE=false
CLEAN_ONLY=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            print_usage
            exit 0
            ;;
        -c|--check)
            SKIP_BUILD=true
            SKIP_RUN=true
            SKIP_ANALYZE=true
            shift
            ;;
        -b|--build)
            SKIP_RUN=true
            SKIP_ANALYZE=true
            shift
            ;;
        -r|--run)
            SKIP_CHECK=true
            SKIP_BUILD=true
            SKIP_ANALYZE=true
            shift
            ;;
        -a|--analyze)
            SKIP_CHECK=true
            SKIP_BUILD=true
            SKIP_RUN=true
            shift
            ;;
        --clean)
            CLEAN_ONLY=true
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            print_usage
            exit 1
            ;;
    esac
done

# Main execution
echo "CKKS Standard vs Binary Variant Performance Comparison"
echo "======================================================"
echo

if [ "$CLEAN_ONLY" = true ]; then
    print_status "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    rm -rf "$OUTPUT_DIR"
    print_success "Cleaned successfully"
    exit 0
fi

# Execute pipeline steps
if [ "$SKIP_CHECK" = false ]; then
    check_dependencies
fi

if [ "$SKIP_BUILD" = false ]; then
    build_benchmark
fi

if [ "$SKIP_RUN" = false ]; then
    run_benchmark
fi

if [ "$SKIP_ANALYZE" = false ]; then
    analyze_results
    show_results
fi

print_success "Experiment pipeline completed!"