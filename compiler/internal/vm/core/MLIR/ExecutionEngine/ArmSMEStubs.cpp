//===- ArmSMEStub.cpp - ArmSME ABI routine stubs --------------------------===//
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

#include "vm/core/Support/Compiler.h"
#include <cstdint>
#include <iostream>

#if (defined(_WIN32) || defined(__CYGWIN__))
#ifndef MLIR_ARMSMEABISTUBS_EXPORTED
#ifdef mlir_arm_sme_abi_stubs_EXPORTS
// We are building this library
#define MLIR_ARMSMEABISTUBS_EXPORTED __declspec(dllexport)
#else
// We are using this library
#define MLIR_ARMSMEABISTUBS_EXPORTED __declspec(dllimport)
#endif // mlir_arm_sme_abi_stubs_EXPORTS
#endif // MLIR_ARMSMEABISTUBS_EXPORTED
#else
#define MLIR_ARMSMEABISTUBS_EXPORTED                                           \
  __attribute__((visibility("default"))) LLVM_ATTRIBUTE_WEAK
#endif // (defined(_WIN32) || defined(__CYGWIN__))

// The actual implementation of these routines is in:
// compiler-rt/lib/builtins/aarch64/sme-abi.S. These stubs allow the current
// ArmSME tests to run without depending on compiler-rt. This works as we don't
// rely on nested ZA-enabled calls at the moment. The use of these stubs can be
// overridden by setting the ARM_SME_ABI_ROUTINES_SHLIB CMake cache variable to
// a path to an alternate implementation.

extern "C" {

bool MLIR_ARMSMEABISTUBS_EXPORTED __aarch64_sme_accessible() {
  // The ArmSME tests are run within an emulator so we assume SME is available.
  return true;
}

struct sme_state {
  int64_t x0;
  int64_t x1;
};

sme_state MLIR_ARMSMEABISTUBS_EXPORTED __arm_sme_state() {
  std::cerr << "[warning] __arm_sme_state() stubbed!\n";
  return sme_state{};
}

void MLIR_ARMSMEABISTUBS_EXPORTED __arm_tpidr2_restore() {
  std::cerr << "[warning] __arm_tpidr2_restore() stubbed!\n";
}

void MLIR_ARMSMEABISTUBS_EXPORTED __arm_tpidr2_save() {
  std::cerr << "[warning] __arm_tpidr2_save() stubbed!\n";
}

void MLIR_ARMSMEABISTUBS_EXPORTED __arm_za_disable() {
  std::cerr << "[warning] __arm_za_disable() stubbed!\n";
}
}
