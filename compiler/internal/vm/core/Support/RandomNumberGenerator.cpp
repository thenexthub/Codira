//===-- RandomNumberGenerator.cpp - Implement RNG class -------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This file implements deterministic random number generation (RNG).
// The current implementation is NOT cryptographically secure as it uses
// the C++11 <random> facilities.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/RandomNumberGenerator.h"

#include "DebugOptions.h"

#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/ManagedStatic.h"
#include "vm/core/Support/raw_ostream.h"
#ifdef _WIN32
#include "vm/core/Support/Windows/WindowsSupport.h"
#else
#include "Unix/Unix.h"
#endif

using namespace vm::core;

#define DEBUG_TYPE "rng"
namespace {
struct CreateSeed {
  static void *call() {
    return new cl::opt<uint64_t>(
        "rng-seed", cl::value_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
static ManagedStatic<cl::opt<uint64_t>, CreateSeed> Seed;
void toolchain::initRandomSeedOptions() { *Seed; }

RandomNumberGenerator::RandomNumberGenerator(StringRef Salt) {
  LLVM_DEBUG(if (*Seed == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::vector<uint32_t> Data;
  Data.resize(2 + Salt.size());
  Data[0] = *Seed;
  Data[1] = *Seed >> 32;

  toolchain::copy(Salt, Data.begin() + 2);

  std::seed_seq SeedSeq(Data.begin(), Data.end());
  Generator.seed(SeedSeq);
}

RandomNumberGenerator::result_type RandomNumberGenerator::operator()() {
  return Generator();
}

// Get random vector of specified size
std::error_code toolchain::getRandomBytes(void *Buffer, size_t Size) {
#ifdef _WIN32
  HCRYPTPROV hProvider;
  if (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    if (CryptGenRandom(hProvider, Size, static_cast<BYTE *>(Buffer)))
      return std::error_code();
  }
  return std::error_code(GetLastError(), std::system_category());
#else
  int Fd = open("/dev/urandom", O_RDONLY);
  if (Fd != -1) {
    std::error_code Ret;
    ssize_t BytesRead = read(Fd, Buffer, Size);
    if (BytesRead == -1)
      Ret = errnoAsErrorCode();
    else if (BytesRead != static_cast<ssize_t>(Size))
      Ret = std::error_code(EIO, std::system_category());
    if (close(Fd) == -1)
      Ret = errnoAsErrorCode();

    return Ret;
  }
  return errnoAsErrorCode();
#endif
}
