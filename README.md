# Binary CKKS Homomorphic Encryption Implementation

A simplified educational implementation of the Binary CKKS homomorphic encryption scheme using NTL.

## Overview

This implementation demonstrates a binary variant of the CKKS (Cheon-Kim-Kim-Song) homomorphic encryption scheme that operates in the binary polynomial ring **Z₂[x]/(xⁿ + 1)**. It provides a clear, educational example of homomorphic encryption concepts without requiring complex dependencies.

## Features

✅ **Binary Polynomial Arithmetic** in Z₂[x]/(xⁿ + 1)  
✅ **Key Generation** with controlled Hamming weight  
✅ **Homomorphic Addition and Multiplication**  
✅ **Noise Management and Estimation**  
✅ **Performance Benchmarking**  
✅ **Educational Examples and Tests**  

## Dependencies

- **NTL** (Number Theory Library) ≥ 11.0.0
- **GMP** (GNU Multiple Precision Arithmetic Library) ≥ 6.0.0
- **C++17** compatible compiler

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential libntl-dev libgmp-dev
```

**macOS:**
```bash
brew install ntl gmp
```

## Building and Running

### Quick Start
```bash
# Check dependencies
make -f simple_makefile check-deps

# Build
make -f simple_makefile all

# Run demo
./simple_binary_ckks_demo
```

### Build Options
```bash
make -f simple_makefile debug    # Debug build
make -f simple_makefile release  # Optimized build
make -f simple_makefile clean    # Clean build files
```

## What the Demo Shows

The demonstration includes:

1. **Basic Operations**: Binary polynomial addition and multiplication
2. **Key Generation**: Cryptographic key creation with proper randomness
3. **Encryption/Decryption**: Basic functionality verification
4. **Homomorphic Operations**: Computing on encrypted data
5. **Performance Analysis**: Timing measurements
6. **Noise Management**: Tracking and controlling cryptographic noise

## Example Output

```
=== Testing Homomorphic Operations ===
Security parameter (lambda): 64
Ring dimension (n): 128
Generating keys...
Key generation completed in 2 ms

Data 1: [1, 0, 1, 1]
Data 2: [0, 1, 1, 0]

Expected Addition: [1, 1, 0, 1]
HE Addition Result: [1, 1, 0, 1]

Expected Multiplication: [0, 0, 1, 0]
HE Multiplication Result: [0, 0, 1, 0]

Correctness check:
Addition: ✓ PASS
Multiplication: ✓ PASS
```

## Implementation Details

### Security Parameters
- **Ring Dimension**: 128-512 (configurable)
- **Security Level**: 64-128 bits
- **Hamming Weight**: λ/4 (for secret key sparsity)
- **Noise Standard Deviation**: 3.2

### Core Components

1. **SimpleBinaryPoly**: Binary polynomial operations in Z₂[x]/(xⁿ + 1)
2. **SimpleBinaryCKKS**: Main scheme implementation
3. **Key Management**: Secure key generation and storage
4. **Noise Tracking**: Built-in noise estimation

### Binary Arithmetic
- Addition: XOR operation (a + b mod 2)
- Multiplication: Polynomial multiplication with reduction
- Ring Structure: x^n = -1 = 1 (in Z₂)

## Educational Value

This implementation serves as:
- **Learning Tool** for homomorphic encryption concepts
- **Research Platform** for experimenting with binary schemes
- **Performance Baseline** for optimization studies
- **Correctness Reference** for implementation verification

## Theoretical Background

Based on the binary variant of CKKS described in academic literature:

- **Security**: Ring Learning With Errors (RLWE) assumption
- **Encoding**: Direct coefficient placement (simplified)
- **Noise Growth**: Linear for addition, multiplicative for multiplication
- **Refresh**: Decrypt → Re-encode → Encrypt cycle

## Files Structure

```
simple_binary_ckks.h          # Header with class definitions
simple_binary_ckks.cpp        # Core implementation
simple_binary_ckks_demo.cpp   # Demonstration program
simple_makefile               # Build configuration
README.md                     # This documentation
```

## Performance Characteristics

**Typical Performance** (128-bit security, 256-bit ring):
- Key Generation: ~2-10ms
- Encryption: ~1-5ms per ciphertext
- Addition: ~1-10μs per operation
- Multiplication: ~1-50ms per operation

*Performance varies by hardware and parameters*

## Limitations and Future Work

### Current Limitations
- Simplified encoding (coefficient-wise)
- No bootstrapping support
- Limited to small ring dimensions
- Educational focus over optimization

### Potential Improvements
- [ ] SIMD-style slot-based encoding
- [ ] Optimized polynomial multiplication (NTT)
- [ ] Bootstrapping implementation
- [ ] Multi-threading support
- [ ] GPU acceleration

## Contributing

This implementation prioritizes:
1. **Clarity** over performance
2. **Correctness** over optimization
3. **Education** over production use

For production applications, consider full-featured libraries like HElib, SEAL, or Palisade.

## License

Educational implementation - see source files for specific licensing terms.

## Acknowledgments

- Based on CKKS scheme by Cheon, Kim, Kim, and Song
- Uses NTL library for number-theoretic operations
- Inspired by HElib architecture and design patterns

---

*This is an educational implementation intended for learning and research purposes.*
