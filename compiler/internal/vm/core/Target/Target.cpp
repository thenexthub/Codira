//===-- Target.cpp --------------------------------------------------------===//
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
// This file implements the common infrastructure (including C bindings) for
// libLLVMTarget.a, which implements target information.
//
//===----------------------------------------------------------------------===//

#include "vm/core-c/Target.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/LegacyPassManager.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Value.h"
#include "vm/core/InitializePasses.h"
#include <cstring>

using namespace vm::core;

inline TargetLibraryInfoImpl *unwrap(LLVMTargetLibraryInfoRef P) {
  return reinterpret_cast<TargetLibraryInfoImpl*>(P);
}

inline LLVMTargetLibraryInfoRef wrap(const TargetLibraryInfoImpl *P) {
  TargetLibraryInfoImpl *X = const_cast<TargetLibraryInfoImpl*>(P);
  return reinterpret_cast<LLVMTargetLibraryInfoRef>(X);
}

void toolchain::initializeTarget(PassRegistry &Registry) {
  initializeTargetLibraryInfoWrapperPassPass(Registry);
  initializeRuntimeLibraryInfoWrapperPass(Registry);
  initializeTargetTransformInfoWrapperPassPass(Registry);
}

LLVMTargetDataRef LLVMGetModuleDataLayout(LLVMModuleRef M) {
  return wrap(&unwrap(M)->getDataLayout());
}

void LLVMSetModuleDataLayout(LLVMModuleRef M, LLVMTargetDataRef DL) {
  unwrap(M)->setDataLayout(*unwrap(DL));
}

LLVMTargetDataRef LLVMCreateTargetData(const char *StringRep) {
  return wrap(new DataLayout(StringRep));
}

void LLVMDisposeTargetData(LLVMTargetDataRef TD) {
  delete unwrap(TD);
}

void LLVMAddTargetLibraryInfo(LLVMTargetLibraryInfoRef TLI,
                              LLVMPassManagerRef PM) {
  unwrap(PM)->add(new TargetLibraryInfoWrapperPass(*unwrap(TLI)));
}

char *LLVMCopyStringRepOfTargetData(LLVMTargetDataRef TD) {
  std::string StringRep = unwrap(TD)->getStringRepresentation();
  return strdup(StringRep.c_str());
}

LLVMByteOrdering LLVMByteOrder(LLVMTargetDataRef TD) {
  return unwrap(TD)->isLittleEndian() ? LLVMLittleEndian : LLVMBigEndian;
}

unsigned LLVMPointerSize(LLVMTargetDataRef TD) {
  return unwrap(TD)->getPointerSize(0);
}

unsigned LLVMPointerSizeForAS(LLVMTargetDataRef TD, unsigned AS) {
  return unwrap(TD)->getPointerSize(AS);
}

LLVMTypeRef LLVMIntPtrType(LLVMTargetDataRef TD) {
  return wrap(unwrap(TD)->getIntPtrType(*unwrap(getGlobalContextForCAPI())));
}

LLVMTypeRef LLVMIntPtrTypeForAS(LLVMTargetDataRef TD, unsigned AS) {
  return wrap(
      unwrap(TD)->getIntPtrType(*unwrap(getGlobalContextForCAPI()), AS));
}

LLVMTypeRef LLVMIntPtrTypeInContext(LLVMContextRef C, LLVMTargetDataRef TD) {
  return wrap(unwrap(TD)->getIntPtrType(*unwrap(C)));
}

LLVMTypeRef LLVMIntPtrTypeForASInContext(LLVMContextRef C, LLVMTargetDataRef TD, unsigned AS) {
  return wrap(unwrap(TD)->getIntPtrType(*unwrap(C), AS));
}

unsigned long long LLVMSizeOfTypeInBits(LLVMTargetDataRef TD, LLVMTypeRef Ty) {
  return unwrap(TD)->getTypeSizeInBits(unwrap(Ty));
}

unsigned long long LLVMStoreSizeOfType(LLVMTargetDataRef TD, LLVMTypeRef Ty) {
  return unwrap(TD)->getTypeStoreSize(unwrap(Ty));
}

unsigned long long LLVMABISizeOfType(LLVMTargetDataRef TD, LLVMTypeRef Ty) {
  return unwrap(TD)->getTypeAllocSize(unwrap(Ty));
}

unsigned LLVMABIAlignmentOfType(LLVMTargetDataRef TD, LLVMTypeRef Ty) {
  return unwrap(TD)->getABITypeAlign(unwrap(Ty)).value();
}

unsigned LLVMCallFrameAlignmentOfType(LLVMTargetDataRef TD, LLVMTypeRef Ty) {
  return unwrap(TD)->getABITypeAlign(unwrap(Ty)).value();
}

unsigned LLVMPreferredAlignmentOfType(LLVMTargetDataRef TD, LLVMTypeRef Ty) {
  return unwrap(TD)->getPrefTypeAlign(unwrap(Ty)).value();
}

unsigned LLVMPreferredAlignmentOfGlobal(LLVMTargetDataRef TD,
                                        LLVMValueRef GlobalVar) {
  return unwrap(TD)
      ->getPreferredAlign(unwrap<GlobalVariable>(GlobalVar))
      .value();
}

unsigned LLVMElementAtOffset(LLVMTargetDataRef TD, LLVMTypeRef StructTy,
                             unsigned long long Offset) {
  StructType *STy = unwrap<StructType>(StructTy);
  return unwrap(TD)->getStructLayout(STy)->getElementContainingOffset(Offset);
}

unsigned long long LLVMOffsetOfElement(LLVMTargetDataRef TD, LLVMTypeRef StructTy,
                                       unsigned Element) {
  StructType *STy = unwrap<StructType>(StructTy);
  return unwrap(TD)->getStructLayout(STy)->getElementOffset(Element);
}
