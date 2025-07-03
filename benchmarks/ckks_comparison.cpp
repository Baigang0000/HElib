/* Copyright (C) 2023 Performance Comparison Study
 * This program is Licensed under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ckks_common.h"
#include "../simple_binary_ckks.h"

#include <helib/helib.h>
#include <helib/debugging.h>

#include <benchmark/benchmark.h>

#include <iostream>
#include <memory>
#include <fstream>
#include <iomanip>

namespace {

/**
 * Binary CKKS Wrapper for Benchmark Framework
 */
struct BinaryCKKSMeta {
  std::unique_ptr<SimpleBinaryCKKS> scheme;
  std::unique_ptr<SimpleBinaryCKKSKeys> keys;
  long ring_dim;
  long security_level;
  
  BinaryCKKSMeta(long _ring_dim, long _security) 
    : ring_dim(_ring_dim), security_level(_security) {
    scheme = std::make_unique<SimpleBinaryCKKS>(security_level, ring_dim);
    keys = std::make_unique<SimpleBinaryCKKSKeys>(scheme->keyGen());
  }
};

/**
 * Combined benchmark parameters for comparison
 */
struct ComparisonParams {
  long m;        // Ring dimension
  long r;        // Precision (for standard CKKS)
  long L;        // Bits (for standard CKKS)
  long security; // Security level (for binary CKKS)
  
  ComparisonParams(long _m, long _r, long _L, long _security)
    : m(_m), r(_r), L(_L), security(_security) {}
};

// Global benchmark state for comparison
struct ComparisonMeta {
  std::unique_ptr<ContextAndKeys> standard_data;
  std::unique_ptr<BinaryCKKSMeta> binary_data;
  
  ComparisonMeta& operator()(ComparisonParams& params) {
    // Initialize standard CKKS if needed
    Params std_params(params.m, params.r, params.L);
    if (standard_data == nullptr || standard_data->params != std_params) {
      standard_data = std::make_unique<ContextAndKeys>(std_params);
    }
    
    // Initialize binary CKKS if needed  
    if (binary_data == nullptr || 
        binary_data->ring_dim != params.m || 
        binary_data->security_level != params.security) {
      binary_data = std::make_unique<BinaryCKKSMeta>(params.m, params.security);
    }
    
    return *this;
  }
};

// ===================== Standard CKKS Benchmarks =====================

static void standard_ckks_keygen(benchmark::State& state, ComparisonMeta& meta) {
  for (auto _ : state) {
    state.PauseTiming();
    helib::Context context = meta.standard_data->context;
    state.ResumeTiming();
    
    helib::SecKey sk(context);
    sk.GenSecKey();
    helib::addSome1DMatrices(sk);
    
    benchmark::DoNotOptimize(sk);
  }
}

static void standard_ckks_encrypt(benchmark::State& state, ComparisonMeta& meta) {
  helib::Ptxt<helib::CKKS> ptxt(meta.standard_data->context);
  ptxt.random();
  
  for (auto _ : state) {
    helib::Ctxt ctxt(meta.standard_data->publicKey);
    meta.standard_data->publicKey.Encrypt(ctxt, ptxt);
    benchmark::DoNotOptimize(ctxt);
  }
}

static void standard_ckks_decrypt(benchmark::State& state, ComparisonMeta& meta) {
  helib::Ptxt<helib::CKKS> ptxt(meta.standard_data->context);
  ptxt.random();
  helib::Ctxt ctxt(meta.standard_data->publicKey);
  meta.standard_data->publicKey.Encrypt(ctxt, ptxt);
  
  for (auto _ : state) {
    helib::Ptxt<helib::CKKS> decrypted_result(meta.standard_data->context);
    meta.standard_data->secretKey.Decrypt(decrypted_result, ctxt);
    benchmark::DoNotOptimize(decrypted_result);
  }
}

static void standard_ckks_add(benchmark::State& state, ComparisonMeta& meta) {
  helib::Ptxt<helib::CKKS> ptxt1(meta.standard_data->context);
  helib::Ptxt<helib::CKKS> ptxt2(meta.standard_data->context);
  ptxt1.random();
  ptxt2.random();

  helib::Ctxt ctxt1(meta.standard_data->publicKey);
  helib::Ctxt ctxt2(meta.standard_data->publicKey);
  meta.standard_data->publicKey.Encrypt(ctxt1, ptxt1);
  meta.standard_data->publicKey.Encrypt(ctxt2, ptxt2);
  
  for (auto _ : state) {
    state.PauseTiming();
    auto copy(ctxt1);
    state.ResumeTiming();
    
    copy += ctxt2;
    benchmark::DoNotOptimize(copy);
  }
}

static void standard_ckks_multiply(benchmark::State& state, ComparisonMeta& meta) {
  helib::Ptxt<helib::CKKS> ptxt1(meta.standard_data->context);
  helib::Ptxt<helib::CKKS> ptxt2(meta.standard_data->context);
  ptxt1.random();
  ptxt2.random();

  helib::Ctxt ctxt1(meta.standard_data->publicKey);
  helib::Ctxt ctxt2(meta.standard_data->publicKey);
  meta.standard_data->publicKey.Encrypt(ctxt1, ptxt1);
  meta.standard_data->publicKey.Encrypt(ctxt2, ptxt2);
  
  for (auto _ : state) {
    state.PauseTiming();
    auto copy(ctxt1);
    state.ResumeTiming();
    
    copy.multiplyBy(ctxt2);
    benchmark::DoNotOptimize(copy);
  }
}

// ===================== Binary CKKS Benchmarks =====================

static void binary_ckks_keygen(benchmark::State& state, ComparisonMeta& meta) {
  for (auto _ : state) {
    SimpleBinaryCKKS scheme(meta.binary_data->security_level, meta.binary_data->ring_dim);
    SimpleBinaryCKKSKeys keys = scheme.keyGen();
    benchmark::DoNotOptimize(keys);
  }
}

static void binary_ckks_encrypt(benchmark::State& state, ComparisonMeta& meta) {
  // Generate test data
  std::vector<long> data(16);
  for (size_t i = 0; i < data.size(); ++i) {
    data[i] = rand() % 2;
  }
  SimpleBinaryPoly encoded = meta.binary_data->scheme->encode(data);
  
  for (auto _ : state) {
    SimpleBinaryCKKSCiphertext ctxt = meta.binary_data->scheme->encrypt(encoded, *meta.binary_data->keys);
    benchmark::DoNotOptimize(ctxt);
  }
}

static void binary_ckks_decrypt(benchmark::State& state, ComparisonMeta& meta) {
  // Generate test data and encrypt
  std::vector<long> data(16);
  for (size_t i = 0; i < data.size(); ++i) {
    data[i] = rand() % 2;
  }
  SimpleBinaryPoly encoded = meta.binary_data->scheme->encode(data);
  SimpleBinaryCKKSCiphertext ctxt = meta.binary_data->scheme->encrypt(encoded, *meta.binary_data->keys);
  
  for (auto _ : state) {
    SimpleBinaryPoly decrypted = meta.binary_data->scheme->decrypt(ctxt, *meta.binary_data->keys);
    benchmark::DoNotOptimize(decrypted);
  }
}

static void binary_ckks_add(benchmark::State& state, ComparisonMeta& meta) {
  // Generate test data
  std::vector<long> data1(16), data2(16);
  for (size_t i = 0; i < data1.size(); ++i) {
    data1[i] = rand() % 2;
    data2[i] = rand() % 2;
  }
  
  SimpleBinaryPoly encoded1 = meta.binary_data->scheme->encode(data1);
  SimpleBinaryPoly encoded2 = meta.binary_data->scheme->encode(data2);
  
  SimpleBinaryCKKSCiphertext ctxt1 = meta.binary_data->scheme->encrypt(encoded1, *meta.binary_data->keys);
  SimpleBinaryCKKSCiphertext ctxt2 = meta.binary_data->scheme->encrypt(encoded2, *meta.binary_data->keys);
  
  for (auto _ : state) {
    SimpleBinaryCKKSCiphertext result = meta.binary_data->scheme->add(ctxt1, ctxt2);
    benchmark::DoNotOptimize(result);
  }
}

static void binary_ckks_multiply(benchmark::State& state, ComparisonMeta& meta) {
  // Generate test data
  std::vector<long> data1(16), data2(16);
  for (size_t i = 0; i < data1.size(); ++i) {
    data1[i] = rand() % 2;
    data2[i] = rand() % 2;
  }
  
  SimpleBinaryPoly encoded1 = meta.binary_data->scheme->encode(data1);
  SimpleBinaryPoly encoded2 = meta.binary_data->scheme->encode(data2);
  
  SimpleBinaryCKKSCiphertext ctxt1 = meta.binary_data->scheme->encrypt(encoded1, *meta.binary_data->keys);
  SimpleBinaryCKKSCiphertext ctxt2 = meta.binary_data->scheme->encrypt(encoded2, *meta.binary_data->keys);
  
  for (auto _ : state) {
    SimpleBinaryCKKSCiphertext result = meta.binary_data->scheme->multiply(ctxt1, ctxt2, *meta.binary_data->keys);
    benchmark::DoNotOptimize(result);
  }
}

// ===================== Benchmark Registration =====================

ComparisonMeta comparison_meta;

// Small parameters for quick testing
ComparisonParams small_params(/*m=*/1024, /*r=*/1, /*L=*/360, /*security=*/128);

// Register Standard CKKS benchmarks
BENCHMARK_CAPTURE(standard_ckks_keygen, small_1024, comparison_meta(small_params));
BENCHMARK_CAPTURE(standard_ckks_encrypt, small_1024, comparison_meta(small_params));
BENCHMARK_CAPTURE(standard_ckks_decrypt, small_1024, comparison_meta(small_params));
BENCHMARK_CAPTURE(standard_ckks_add, small_1024, comparison_meta(small_params));
BENCHMARK_CAPTURE(standard_ckks_multiply, small_1024, comparison_meta(small_params));

// Register Binary CKKS benchmarks  
BENCHMARK_CAPTURE(binary_ckks_keygen, small_1024, comparison_meta(small_params));
BENCHMARK_CAPTURE(binary_ckks_encrypt, small_1024, comparison_meta(small_params));
BENCHMARK_CAPTURE(binary_ckks_decrypt, small_1024, comparison_meta(small_params));
BENCHMARK_CAPTURE(binary_ckks_add, small_1024, comparison_meta(small_params));
BENCHMARK_CAPTURE(binary_ckks_multiply, small_1024, comparison_meta(small_params));

// Medium parameters
ComparisonParams medium_params(/*m=*/2048, /*r=*/1, /*L=*/360, /*security=*/128);

BENCHMARK_CAPTURE(standard_ckks_keygen, medium_2048, comparison_meta(medium_params));
BENCHMARK_CAPTURE(standard_ckks_encrypt, medium_2048, comparison_meta(medium_params));
BENCHMARK_CAPTURE(standard_ckks_decrypt, medium_2048, comparison_meta(medium_params));
BENCHMARK_CAPTURE(standard_ckks_add, medium_2048, comparison_meta(medium_params));
BENCHMARK_CAPTURE(standard_ckks_multiply, medium_2048, comparison_meta(medium_params));

BENCHMARK_CAPTURE(binary_ckks_keygen, medium_2048, comparison_meta(medium_params));
BENCHMARK_CAPTURE(binary_ckks_encrypt, medium_2048, comparison_meta(medium_params));
BENCHMARK_CAPTURE(binary_ckks_decrypt, medium_2048, comparison_meta(medium_params));
BENCHMARK_CAPTURE(binary_ckks_add, medium_2048, comparison_meta(medium_params));
BENCHMARK_CAPTURE(binary_ckks_multiply, medium_2048, comparison_meta(medium_params));

// Large parameters (if system can handle)
ComparisonParams large_params(/*m=*/4096, /*r=*/1, /*L=*/360, /*security=*/128);

BENCHMARK_CAPTURE(standard_ckks_keygen, large_4096, comparison_meta(large_params));
BENCHMARK_CAPTURE(standard_ckks_encrypt, large_4096, comparison_meta(large_params));
BENCHMARK_CAPTURE(standard_ckks_decrypt, large_4096, comparison_meta(large_params));
BENCHMARK_CAPTURE(standard_ckks_add, large_4096, comparison_meta(large_params));
BENCHMARK_CAPTURE(standard_ckks_multiply, large_4096, comparison_meta(large_params));

BENCHMARK_CAPTURE(binary_ckks_keygen, large_4096, comparison_meta(large_params));
BENCHMARK_CAPTURE(binary_ckks_encrypt, large_4096, comparison_meta(large_params));
BENCHMARK_CAPTURE(binary_ckks_decrypt, large_4096, comparison_meta(large_params));
BENCHMARK_CAPTURE(binary_ckks_add, large_4096, comparison_meta(large_params));
BENCHMARK_CAPTURE(binary_ckks_multiply, large_4096, comparison_meta(large_params));

} // namespace

/**
 * Custom main to add result analysis
 */
int main(int argc, char** argv) {
  std::cout << "=== CKKS Standard vs Binary Variant Performance Comparison ===" << std::endl;
  std::cout << "Comparing HElib Standard CKKS with Binary CKKS over Z_2[x]/(x^n + 1)" << std::endl;
  std::cout << "=================================================================" << std::endl;
  
  // Initialize Google Benchmark
  benchmark::Initialize(&argc, argv);
  
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
    return 1;
  }
  
  // Run benchmarks with custom reporter
  benchmark::RunSpecifiedBenchmarks();
  
  std::cout << "\n=== Benchmark Analysis ===" << std::endl;
  std::cout << "Results show performance comparison between:" << std::endl;
  std::cout << "- Standard CKKS: Full-featured real/complex homomorphic encryption" << std::endl;
  std::cout << "- Binary CKKS: Simplified binary polynomial variant" << std::endl;
  std::cout << "\nFor detailed analysis, use benchmark output with --benchmark_format=json" << std::endl;
  std::cout << "and process with analysis tools." << std::endl;
  
  return 0;
}