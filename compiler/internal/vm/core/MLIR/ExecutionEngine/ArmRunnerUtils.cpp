//===- ArmRunnerUtils.cpp - Utilities for configuring architecture properties //
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

#include "vm/core/Support/MathExtras.h"
#include <iostream>
#include <stdint.h>
#include <string_view>

#if (defined(_WIN32) || defined(__CYGWIN__))
#define MLIR_ARMRUNNERUTILS_EXPORTED __declspec(dllexport)
#else
#define MLIR_ARMRUNNERUTILS_EXPORTED __attribute__((visibility("default")))
#endif

#ifdef __linux__
#include <sys/prctl.h>
#endif

extern "C" {

// Defines for prctl() calls. These may not necessarily exist in the host
// <sys/prctl.h>, but will still be useable under emulation.
//
// https://www.kernel.org/doc/html/v5.3/arm64/sve.html#prctl-extensions
#ifndef PR_SVE_SET_VL
#define PR_SVE_SET_VL 50
#endif
// https://docs.kernel.org/arch/arm64/sme.html#prctl-extensions
#ifndef PR_SME_SET_VL
#define PR_SME_SET_VL 63
#endif
// Note: This mask is the same as both PR_SME_VL_LEN_MASK and
// PR_SVE_VL_LEN_MASK.
#define PR_VL_LEN_MASK 0xffff

/// Sets the vector length (streaming or not, as indicated by `option`) to
/// `bits`.
///
/// Caveat emptor: If a function has allocated stack slots for SVE registers
/// (e.g. slots for callee-saved SVE registers or spill slots) changing
/// the vector length is tricky and error prone - it may cause incorrect stack
/// deallocation or incorrect access to stack slots.
///
/// The recommended strategy is to call `setArmVectorLength` only from functions
/// that do not access SVE registers, either by themselves or by inlining other
/// functions.
static void setArmVectorLength(std::string_view helperName, int option,
                               uint32_t bits) {
#if defined(__linux__) && defined(__aarch64__)
  if (bits < 128 || bits > 2048 || !toolchain::isPowerOf2_32(bits)) {
    std::cerr << "[error] Attempted to set an invalid vector length (" << bits
              << "-bit)" << std::endl;
    abort();
  }
  uint32_t vl = bits / 8;
  if (auto ret = prctl(option, vl & PR_VL_LEN_MASK); ret < 0) {
    std::cerr << "[error] prctl failed (" << ret << ")" << std::endl;
    abort();
  }
#else
  std::cerr << "[error] " << helperName << " is unsupported" << std::endl;
  abort();
#endif
}

/// Sets the SVE vector length (in bits) to `bits`.
void MLIR_ARMRUNNERUTILS_EXPORTED setArmVLBits(uint32_t bits) {
  setArmVectorLength(__func__, PR_SVE_SET_VL, bits);
}

/// Sets the SME streaming vector length (in bits) to `bits`.
void MLIR_ARMRUNNERUTILS_EXPORTED setArmSVLBits(uint32_t bits) {
  setArmVectorLength(__func__, PR_SME_SET_VL, bits);
}
}
