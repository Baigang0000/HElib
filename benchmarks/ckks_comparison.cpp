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
 * Binary CKKS Meta structure for benchmark framework
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

struct BinaryMeta {
  std::unique_ptr<BinaryCKKSMeta> data;
  
  BinaryMeta& operator()(Params& params) {
    // Use ring dimension from params.m and default security level
    long security = 128; // Default security level
    if (data == nullptr || 
        data->ring_dim != params.m || 
        data->security_level != security) {
      data = std::make_unique<BinaryCKKSMeta>(params.m, security);
    }
    return *this;
  }
};

// ===================== Standard CKKS Benchmarks =====================

static void standard_ckks_keygen(benchmark::State& state, Meta& meta) {
  for (auto _ : state) {
    state.PauseTiming();
    helib::Context context = meta.data->context;
    state.ResumeTiming();
    
    helib::SecKey sk(context);
    sk.GenSecKey();
    helib::addSome1DMatrices(sk);
    
    benchmark::DoNotOptimize(sk);
  }
}

static void standard_ckks_encrypt(benchmark::State& state, Meta& meta) {
  helib::Ptxt<helib::CKKS> ptxt(meta.data->context);
  ptxt.random();
  
  for (auto _ : state) {
    helib::Ctxt ctxt(meta.data->publicKey);
    meta.data->publicKey.Encrypt(ctxt, ptxt);
    benchmark::DoNotOptimize(ctxt);
  }
}

static void standard_ckks_decrypt(benchmark::State& state, Meta& meta) {
  helib::Ptxt<helib::CKKS> ptxt(meta.data->context);
  ptxt.random();
  helib::Ctxt ctxt(meta.data->publicKey);
  meta.data->publicKey.Encrypt(ctxt, ptxt);
  
  for (auto _ : state) {
    helib::Ptxt<helib::CKKS> decrypted_result(meta.data->context);
    meta.data->secretKey.Decrypt(decrypted_result, ctxt);
    benchmark::DoNotOptimize(decrypted_result);
  }
}

static void standard_ckks_add(benchmark::State& state, Meta& meta) {
  helib::Ptxt<helib::CKKS> ptxt1(meta.data->context);
  helib::Ptxt<helib::CKKS> ptxt2(meta.data->context);
  ptxt1.random();
  ptxt2.random();

  helib::Ctxt ctxt1(meta.data->publicKey);
  helib::Ctxt ctxt2(meta.data->publicKey);
  meta.data->publicKey.Encrypt(ctxt1, ptxt1);
  meta.data->publicKey.Encrypt(ctxt2, ptxt2);
  
  for (auto _ : state) {
    state.PauseTiming();
    auto copy(ctxt1);
    state.ResumeTiming();
    
    copy += ctxt2;
    benchmark::DoNotOptimize(copy);
  }
}

static void standard_ckks_multiply(benchmark::State& state, Meta& meta) {
  helib::Ptxt<helib::CKKS> ptxt1(meta.data->context);
  helib::Ptxt<helib::CKKS> ptxt2(meta.data->context);
  ptxt1.random();
  ptxt2.random();

  helib::Ctxt ctxt1(meta.data->publicKey);
  helib::Ctxt ctxt2(meta.data->publicKey);
  meta.data->publicKey.Encrypt(ctxt1, ptxt1);
  meta.data->publicKey.Encrypt(ctxt2, ptxt2);
  
  for (auto _ : state) {
    state.PauseTiming();
    auto copy(ctxt1);
    state.ResumeTiming();
    
    copy.multiplyBy(ctxt2);
    benchmark::DoNotOptimize(copy);
  }
}

// ===================== Binary CKKS Benchmarks =====================

static void binary_ckks_keygen(benchmark::State& state, BinaryMeta& meta) {
  for (auto _ : state) {
    SimpleBinaryCKKS scheme(meta.data->security_level, meta.data->ring_dim);
    SimpleBinaryCKKSKeys keys = scheme.keyGen();
    benchmark::DoNotOptimize(keys);
  }
}

static void binary_ckks_encrypt(benchmark::State& state, BinaryMeta& meta) {
  // Generate test data
  std::vector<long> data(16);
  for (size_t i = 0; i < data.size(); ++i) {
    data[i] = rand() % 2;
  }
  SimpleBinaryPoly encoded = meta.data->scheme->encode(data);
  
  for (auto _ : state) {
    SimpleBinaryCKKSCiphertext ctxt = meta.data->scheme->encrypt(encoded, *meta.data->keys);
    benchmark::DoNotOptimize(ctxt);
  }
}

static void binary_ckks_decrypt(benchmark::State& state, BinaryMeta& meta) {
  // Generate test data and encrypt
  std::vector<long> data(16);
  for (size_t i = 0; i < data.size(); ++i) {
    data[i] = rand() % 2;
  }
  SimpleBinaryPoly encoded = meta.data->scheme->encode(data);
  SimpleBinaryCKKSCiphertext ctxt = meta.data->scheme->encrypt(encoded, *meta.data->keys);
  
  for (auto _ : state) {
    SimpleBinaryPoly decrypted = meta.data->scheme->decrypt(ctxt, *meta.data->keys);
    benchmark::DoNotOptimize(decrypted);
  }
}

static void binary_ckks_add(benchmark::State& state, BinaryMeta& meta) {
  // Generate test data
  std::vector<long> data1(16), data2(16);
  for (size_t i = 0; i < data1.size(); ++i) {
    data1[i] = rand() % 2;
    data2[i] = rand() % 2;
  }
  
  SimpleBinaryPoly encoded1 = meta.data->scheme->encode(data1);
  SimpleBinaryPoly encoded2 = meta.data->scheme->encode(data2);
  
  SimpleBinaryCKKSCiphertext ctxt1 = meta.data->scheme->encrypt(encoded1, *meta.data->keys);
  SimpleBinaryCKKSCiphertext ctxt2 = meta.data->scheme->encrypt(encoded2, *meta.data->keys);
  
  for (auto _ : state) {
    SimpleBinaryCKKSCiphertext result = meta.data->scheme->add(ctxt1, ctxt2);
    benchmark::DoNotOptimize(result);
  }
}

static void binary_ckks_multiply(benchmark::State& state, BinaryMeta& meta) {
  // Generate test data
  std::vector<long> data1(16), data2(16);
  for (size_t i = 0; i < data1.size(); ++i) {
    data1[i] = rand() % 2;
    data2[i] = rand() % 2;
  }
  
  SimpleBinaryPoly encoded1 = meta.data->scheme->encode(data1);
  SimpleBinaryPoly encoded2 = meta.data->scheme->encode(data2);
  
  SimpleBinaryCKKSCiphertext ctxt1 = meta.data->scheme->encrypt(encoded1, *meta.data->keys);
  SimpleBinaryCKKSCiphertext ctxt2 = meta.data->scheme->encrypt(encoded2, *meta.data->keys);
  
  for (auto _ : state) {
    SimpleBinaryCKKSCiphertext result = meta.data->scheme->multiply(ctxt1, ctxt2, *meta.data->keys);
    benchmark::DoNotOptimize(result);
  }
}

// ===================== Benchmark Registration =====================

// Meta objects for standard and binary CKKS
Meta standard_meta;
BinaryMeta binary_meta;

// Small parameters for quick testing
Params small_params(/*m=*/1024, /*r=*/1, /*L=*/360);

// Medium parameters
Params medium_params(/*m=*/2048, /*r=*/1, /*L=*/360);

// Large parameters (if system can handle)
Params large_params(/*m=*/4096, /*r=*/1, /*L=*/360);

// Register Standard CKKS benchmarks using the correct HE_BENCH_CAPTURE pattern
HE_BENCH_CAPTURE(standard_ckks_keygen, small_params, standard_meta);
HE_BENCH_CAPTURE(standard_ckks_encrypt, small_params, standard_meta);
HE_BENCH_CAPTURE(standard_ckks_decrypt, small_params, standard_meta);
HE_BENCH_CAPTURE(standard_ckks_add, small_params, standard_meta);
HE_BENCH_CAPTURE(standard_ckks_multiply, small_params, standard_meta);

HE_BENCH_CAPTURE(standard_ckks_keygen, medium_params, standard_meta);
HE_BENCH_CAPTURE(standard_ckks_encrypt, medium_params, standard_meta);
HE_BENCH_CAPTURE(standard_ckks_decrypt, medium_params, standard_meta);
HE_BENCH_CAPTURE(standard_ckks_add, medium_params, standard_meta);
HE_BENCH_CAPTURE(standard_ckks_multiply, medium_params, standard_meta);

HE_BENCH_CAPTURE(standard_ckks_keygen, large_params, standard_meta);
HE_BENCH_CAPTURE(standard_ckks_encrypt, large_params, standard_meta);
HE_BENCH_CAPTURE(standard_ckks_decrypt, large_params, standard_meta);
HE_BENCH_CAPTURE(standard_ckks_add, large_params, standard_meta);
HE_BENCH_CAPTURE(standard_ckks_multiply, large_params, standard_meta);

// Register Binary CKKS benchmarks - use direct BENCHMARK_CAPTURE with correct params
BENCHMARK_CAPTURE(binary_ckks_keygen, small_params, binary_meta(small_params));
BENCHMARK_CAPTURE(binary_ckks_encrypt, small_params, binary_meta(small_params));
BENCHMARK_CAPTURE(binary_ckks_decrypt, small_params, binary_meta(small_params));
BENCHMARK_CAPTURE(binary_ckks_add, small_params, binary_meta(small_params));
BENCHMARK_CAPTURE(binary_ckks_multiply, small_params, binary_meta(small_params));

BENCHMARK_CAPTURE(binary_ckks_keygen, medium_params, binary_meta(medium_params));
BENCHMARK_CAPTURE(binary_ckks_encrypt, medium_params, binary_meta(medium_params));
BENCHMARK_CAPTURE(binary_ckks_decrypt, medium_params, binary_meta(medium_params));
BENCHMARK_CAPTURE(binary_ckks_add, medium_params, binary_meta(medium_params));
BENCHMARK_CAPTURE(binary_ckks_multiply, medium_params, binary_meta(medium_params));

BENCHMARK_CAPTURE(binary_ckks_keygen, large_params, binary_meta(large_params));
BENCHMARK_CAPTURE(binary_ckks_encrypt, large_params, binary_meta(large_params));
BENCHMARK_CAPTURE(binary_ckks_decrypt, large_params, binary_meta(large_params));
BENCHMARK_CAPTURE(binary_ckks_add, large_params, binary_meta(large_params));
BENCHMARK_CAPTURE(binary_ckks_multiply, large_params, binary_meta(large_params));

} // namespace

BENCHMARK_MAIN();