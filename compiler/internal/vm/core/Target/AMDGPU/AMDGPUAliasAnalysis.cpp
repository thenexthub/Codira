//===- AMDGPUAliasAnalysis ------------------------------------------------===//
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
/// \file
/// This is the AMGPU address space based alias analysis pass.
//===----------------------------------------------------------------------===//

#include "AMDGPUAliasAnalysis.h"
#include "AMDGPU.h"
#include "vm/core/Analysis/ValueTracking.h"
#include "vm/core/IR/Instructions.h"

using namespace vm::core;

#define DEBUG_TYPE "amdgpu-aa"

AnalysisKey AMDGPUAA::Key;

// Register this pass...
char AMDGPUAAWrapperPass::ID = 0;
char AMDGPUExternalAAWrapper::ID = 0;

INITIALIZE_PASS(AMDGPUAAWrapperPass, "amdgpu-aa",
                "AMDGPU Address space based Alias Analysis", false, true)

INITIALIZE_PASS(AMDGPUExternalAAWrapper, "amdgpu-aa-wrapper",
                "AMDGPU Address space based Alias Analysis Wrapper", false, true)

ImmutablePass *toolchain::createAMDGPUAAWrapperPass() {
  return new AMDGPUAAWrapperPass();
}

ImmutablePass *toolchain::createAMDGPUExternalAAWrapperPass() {
  return new AMDGPUExternalAAWrapper();
}

AMDGPUAAWrapperPass::AMDGPUAAWrapperPass() : ImmutablePass(ID) {}

void AMDGPUAAWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

AliasResult AMDGPUAAResult::alias(const MemoryLocation &LocA,
                                  const MemoryLocation &LocB, AAQueryInfo &AAQI,
                                  const Instruction *) {
  unsigned asA = LocA.Ptr->getType()->getPointerAddressSpace();
  unsigned asB = LocB.Ptr->getType()->getPointerAddressSpace();

  if (!AMDGPU::addrspacesMayAlias(asA, asB))
    return AliasResult::NoAlias;

  // In general, FLAT (generic) pointers could be aliased to LOCAL or PRIVATE
  // pointers. However, as LOCAL or PRIVATE pointers point to local objects, in
  // certain cases, it's still viable to check whether a FLAT pointer won't
  // alias to a LOCAL or PRIVATE pointer.
  MemoryLocation A = LocA;
  MemoryLocation B = LocB;
  // Canonicalize the location order to simplify the following alias check.
  if (asA != AMDGPUAS::FLAT_ADDRESS) {
    std::swap(asA, asB);
    std::swap(A, B);
  }
  if (asA == AMDGPUAS::FLAT_ADDRESS &&
      (asB == AMDGPUAS::LOCAL_ADDRESS || asB == AMDGPUAS::PRIVATE_ADDRESS)) {
    const auto *ObjA =
        getUnderlyingObject(A.Ptr->stripPointerCastsForAliasAnalysis());
    if (const LoadInst *LI = dyn_cast<LoadInst>(ObjA)) {
      // If a generic pointer is loaded from the constant address space, it
      // could only be a GLOBAL or CONSTANT one as that address space is solely
      // prepared on the host side, where only GLOBAL or CONSTANT variables are
      // visible. Note that this even holds for regular functions.
      if (LI->getPointerAddressSpace() == AMDGPUAS::CONSTANT_ADDRESS)
        return AliasResult::NoAlias;
    } else if (const Argument *Arg = dyn_cast<Argument>(ObjA)) {
      const Function *F = Arg->getParent();
      switch (F->getCallingConv()) {
      case CallingConv::AMDGPU_KERNEL: {
        // In the kernel function, kernel arguments won't alias to (local)
        // variables in shared or private address space.
        const auto *ObjB =
            getUnderlyingObject(B.Ptr->stripPointerCastsForAliasAnalysis());
        return ObjA != ObjB && isIdentifiedObject(ObjB) ? AliasResult::NoAlias
                                                        : AliasResult::MayAlias;
      }
      default:
        // TODO: In the regular function, if that local variable in the
        // location B is not captured, that argument pointer won't alias to it
        // as well.
        break;
      }
    }
  }

  return AliasResult::MayAlias;
}

ModRefInfo AMDGPUAAResult::getModRefInfoMask(const MemoryLocation &Loc,
                                             AAQueryInfo &AAQI,
                                             bool IgnoreLocals) {
  unsigned AS = Loc.Ptr->getType()->getPointerAddressSpace();
  if (AS == AMDGPUAS::CONSTANT_ADDRESS ||
      AS == AMDGPUAS::CONSTANT_ADDRESS_32BIT)
    return ModRefInfo::NoModRef;

  const Value *Base = getUnderlyingObject(Loc.Ptr);
  AS = Base->getType()->getPointerAddressSpace();
  if (AS == AMDGPUAS::CONSTANT_ADDRESS ||
      AS == AMDGPUAS::CONSTANT_ADDRESS_32BIT)
    return ModRefInfo::NoModRef;

  return ModRefInfo::ModRef;
}
