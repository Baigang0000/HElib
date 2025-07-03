# CKKS Standard vs Binary Variant Performance Comparison

This experiment compares the runtime performance of **Standard CKKS** (HElib implementation) and **Binary CKKS** variants for homomorphic encryption operations. This benchmarking framework is designed for academic research and conference paper submissions.

## 🎯 Objective

Compare the performance characteristics of:
- **Standard CKKS**: Real/complex number homomorphic encryption with scaling
- **Binary CKKS**: Simplified binary polynomial variant over Z₂[x]/(x^n + 1)

## 📋 Prerequisites

### System Requirements
- Linux/macOS (tested on Ubuntu 20.04+)
- GCC 8+ or Clang 10+
- CMake 3.10+
- Python 3.8+ (for analysis)

### Dependencies

#### 1. Install Base Dependencies
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake git libgmp-dev

# macOS
brew install cmake gmp
```

#### 2. Install NTL (Number Theory Library)
```bash
# Download and build NTL
wget https://shoup.net/ntl/ntl-11.5.1.tar.gz
tar -xzf ntl-11.5.1.tar.gz
cd ntl-11.5.1/src
./configure PREFIX=/usr/local
make
sudo make install
```

#### 3. Install HElib
```bash
# Clone HElib repository
git clone https://github.com/homenc/HElib.git
cd HElib
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

#### 4. Python Dependencies (for analysis)
```bash
pip install pandas matplotlib seaborn numpy
```

## 🚀 Quick Start

### 1. Clone and Build
```bash
# The experiment files should be in the HElib directory
cd /path/to/HElib

# Copy the benchmark files to HElib directory
# (ckks_performance_comparison.cpp, CMakeLists_benchmark.txt, etc.)

# Create build directory
mkdir benchmark_build && cd benchmark_build

# Build the benchmark
cmake -f ../CMakeLists_benchmark.txt ..
make -j$(nproc)
```

### 2. Run the Benchmark
```bash
# Run the comprehensive performance comparison
./ckks_performance_comparison

# This will generate CSV files with results:
# - ckks_comparison_1024_128.csv
# - ckks_comparison_2048_128.csv  
# - ckks_comparison_4096_128.csv
```

### 3. Analyze Results
```bash
# Generate publication-quality plots and tables
python3 ../analyze_results.py

# Or with custom directories:
python3 ../analyze_results.py --data-dir . --output-dir results/
```

## 📊 Output Files

The experiment generates several files for your conference paper:

### Data Files
- `ckks_comparison_<ringdim>_<security>.csv` - Raw benchmark data
  
### Analysis Outputs
- `performance_comparison.pdf` - Main comparison figure (4-panel layout)
- `detailed_analysis.pdf` - Detailed per-operation analysis
- `results_table.tex` - LaTeX table for paper inclusion

## 🔧 Configuration

### Adjusting Parameters

Edit the parameter sets in `ckks_performance_comparison.cpp`:

```cpp
vector<pair<long, long>> parameter_sets = {
    {1024, 128},   // (ring_dimension, security_level)
    {2048, 128},   
    {4096, 128},   
    {8192, 256},   // Add larger parameters if system allows
};
```

### Adjusting Iteration Counts

Modify iteration counts for different precision/time tradeoffs:

```cpp
// In runComprehensiveBenchmark():
results.standard_ckks["KeyGeneration"] = std_benchmark.benchmarkKeyGeneration(5);    // Increase for better statistics
results.standard_ckks["Encryption"] = std_benchmark.benchmarkEncryption(50);        // Increase for better precision
results.standard_ckks["Addition"] = std_benchmark.benchmarkAddition(200);           // Fast operation - more iterations
```

## 📈 Expected Results

### Typical Performance Characteristics

Based on the theoretical differences between Standard and Binary CKKS:

| Operation | Expected Binary CKKS Advantage | Reason |
|-----------|--------------------------------|--------|
| **Addition** | 2-5x faster | Simpler Z₂ arithmetic vs floating-point |
| **Multiplication** | 3-10x faster | No relinearization complexity, binary ops |
| **Key Generation** | 1.5-3x faster | Simpler key structure |
| **Encryption** | 2-4x faster | No scaling factor management |
| **Decryption** | 2-4x faster | Direct polynomial operations |

### Scaling Behavior
- **Standard CKKS**: O(n log n) operations with complex coefficient management
- **Binary CKKS**: O(n) or O(n log n) with simpler binary operations

## 📝 Paper Integration

### Figure Integration

Use the generated plots in your LaTeX paper:

```latex
\begin{figure}[htbp]
    \centering
    \includegraphics[width=\textwidth]{performance_comparison.pdf}
    \caption{Performance comparison between Standard CKKS and Binary CKKS variants 
             across different homomorphic operations.}
    \label{fig:ckks_comparison}
\end{figure}
```

### Table Integration

Include the generated LaTeX table:

```latex
\input{results_table.tex}
```

### Key Metrics to Report

1. **Average Speedup**: Overall performance improvement
2. **Operation-specific Analysis**: Which operations benefit most
3. **Scaling Analysis**: How performance changes with ring dimension  
4. **Memory Usage**: Binary variant typically uses less memory
5. **Security Considerations**: Discuss security-performance tradeoffs

## 🔍 Troubleshooting

### Common Issues

#### 1. HElib Not Found
```bash
# Make sure HElib is installed system-wide
sudo make install
# Or specify custom path in CMakeLists_benchmark.txt
```

#### 2. NTL Linking Errors  
```bash
# Ensure NTL is properly installed
ldconfig
# Check library path
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

#### 3. Memory Issues with Large Parameters
```bash
# Reduce ring dimensions or use smaller parameter sets
# Monitor memory usage: htop or top
```

#### 4. Performance Inconsistency
- Ensure system is not under load during benchmarking
- Run multiple times and average results
- Disable frequency scaling: `sudo cpupower frequency-set --governor performance`

### Debugging

Enable debug mode for detailed output:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
./ckks_performance_comparison
```

## 📋 Experiment Checklist

### Pre-experiment
- [ ] All dependencies installed correctly
- [ ] Benchmark compiles without errors  
- [ ] Test run with small parameters works
- [ ] System is idle (no heavy background processes)

### During Experiment
- [ ] Monitor memory usage
- [ ] Check for thermal throttling
- [ ] Verify output files are generated
- [ ] Watch for error messages

### Post-experiment  
- [ ] Verify CSV files contain expected data
- [ ] Generate and review plots
- [ ] Check LaTeX table formatting
- [ ] Validate results make theoretical sense

## 🤝 Contributing

If you extend this benchmark or find issues:

1. **Bug Reports**: Include system info, error messages, parameter sets
2. **Performance Data**: Share results from different architectures
3. **Extensions**: Additional operations, different parameter ranges
4. **Optimizations**: Improvements to binary CKKS implementation

## 📚 References

For your conference paper, cite relevant works:

```bibtex
@inproceedings{cheon2017homomorphic,
  title={Homomorphic encryption for arithmetic of approximate numbers},
  author={Cheon, Jung Hee and Kim, Andrey and Kim, Miran and Song, Yongsoo},
  booktitle={International Conference on the Theory and Application of Cryptology and Information Security},
  pages={409--437},
  year={2017},
  organization={Springer}
}

@misc{helib,
  title={HElib: An Implementation of homomorphic encryption},
  author={Halevi, Shai and Shoup, Victor},
  howpublished={\url{https://github.com/homenc/HElib}},
  year={2020}
}
```

## 📞 Support

For technical questions about this experiment:
- Check existing GitHub issues in the HElib repository
- Review HElib documentation
- Consider computational complexity differences between variants

---

**Good luck with your conference paper! 🎓**

*This benchmarking framework provides a solid foundation for comparing CKKS variants and should give you robust performance data for academic publication.*