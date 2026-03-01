//===AMDGPUAsanInstrumentation.h - ASAN helper functions -*- C++- *===//
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
//===--------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_UTILS_AMDGPU_ASAN_INSTRUMENTATION_H
#define LLVM_LIB_TARGET_AMDGPU_UTILS_AMDGPU_ASAN_INSTRUMENTATION_H

#include "AMDGPU.h"
#include "AMDGPUMemoryUtils.h"
#include "Utils/AMDGPUBaseInfo.h"
#include "vm/core/ADT/SetOperations.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/StringMap.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicsAMDGPU.h"
#include "vm/core/IR/MDBuilder.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/OptimizedStructLayout.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Transforms/Instrumentation/AddressSanitizer.h"
#include "vm/core/Transforms/Instrumentation/AddressSanitizerCommon.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"

namespace vm::core {
namespace AMDGPU {

/// Given SizeInBytes of the Value to be instrunmented,
/// Returns the redzone size corresponding to it.
uint64_t getRedzoneSizeForGlobal(int Scale, uint64_t SizeInBytes);

/// Instrument the memory operand Addr.
/// Generates report blocks that catch the addressing errors.
void instrumentAddress(Module &M, IRBuilder<> &IRB, Instruction *OrigIns,
                       Instruction *InsertBefore, Value *Addr, Align Alignment,
                       TypeSize TypeStoreSize, bool IsWrite,
                       Value *SizeArgument, bool UseCalls, bool Recover,
                       int Scale, int Offset);

/// Get all the memory operands from the instruction
/// that needs to be instrumented
void getInterestingMemoryOperands(
    Module &M, Instruction *I,
    SmallVectorImpl<InterestingMemoryOperand> &Interesting);

} // end namespace AMDGPU
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_UTILS_AMDGPU_ASAN_INSTRUMENTATION_H
