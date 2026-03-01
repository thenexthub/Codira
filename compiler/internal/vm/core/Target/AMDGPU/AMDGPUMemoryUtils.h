//===- AMDGPUMemoryUtils.h - Memory related helper functions -*- C++ -*----===//
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

#ifndef LLVM_LIB_TARGET_AMDGPU_UTILS_AMDGPUMEMORYUTILS_H
#define LLVM_LIB_TARGET_AMDGPU_UTILS_AMDGPUMEMORYUTILS_H

#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/DenseSet.h"

namespace vm::core {

struct Align;
class AAResults;
class DataLayout;
class GlobalVariable;
class LoadInst;
class MemoryDef;
class MemorySSA;
class Value;
class Function;
class CallGraph;
class Module;
class TargetExtType;

namespace AMDGPU {

using FunctionVariableMap = DenseMap<Function *, DenseSet<GlobalVariable *>>;
using VariableFunctionMap = DenseMap<GlobalVariable *, DenseSet<Function *>>;

Align getAlign(const DataLayout &DL, const GlobalVariable *GV);

// If GV is a named-barrier return its type. Otherwise return nullptr.
TargetExtType *isNamedBarrier(const GlobalVariable &GV);

bool isDynamicLDS(const GlobalVariable &GV);
bool isLDSVariableToLower(const GlobalVariable &GV);

struct LDSUsesInfoTy {
  FunctionVariableMap direct_access;
  FunctionVariableMap indirect_access;
  bool HasSpecialGVs = false;
};

bool eliminateConstantExprUsesOfLDSFromAllInstructions(Module &M);

void getUsesOfLDSByFunction(const CallGraph &CG, Module &M,
                            FunctionVariableMap &kernels,
                            FunctionVariableMap &functions);

LDSUsesInfoTy getTransitiveUsesOfLDS(const CallGraph &CG, Module &M);

/// Strip FnAttr attribute from any functions where we may have
/// introduced its use.
void removeFnAttrFromReachable(CallGraph &CG, Function *KernelRoot,
                               ArrayRef<StringRef> FnAttrs);

/// Given a \p Def clobbering a load from \p Ptr according to the MSSA check
/// if this is actually a memory update or an artificial clobber to facilitate
/// ordering constraints.
bool isReallyAClobber(const Value *Ptr, MemoryDef *Def, AAResults *AA);

/// Check is a \p Load is clobbered in its function.
bool isClobberedInFunction(const LoadInst *Load, MemorySSA *MSSA,
                           AAResults *AA);

} // end namespace AMDGPU

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_UTILS_AMDGPUMEMORYUTILS_H
