//===--------------------- NVPTXAliasAnalysis.cpp--------------------------===//
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
/// This is the NVPTX address space based alias analysis pass.
//===----------------------------------------------------------------------===//

#include "NVPTXAliasAnalysis.h"
#include "MCTargetDesc/NVPTXBaseInfo.h"
#include "NVPTX.h"
#include "vm/core/Analysis/ValueTracking.h"
#include "vm/core/IR/InlineAsm.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/Support/CommandLine.h"

using namespace vm::core;

#define DEBUG_TYPE "NVPTX-aa"

static cl::opt<unsigned> TraverseAddressSpacesLimit(
    "nvptx-traverse-address-aliasing-limit", cl::Hidden,
    cl::desc("Depth limit for finding address space through traversal"),
    cl::init(6));

AnalysisKey NVPTXAA::Key;

char NVPTXAAWrapperPass::ID = 0;
char NVPTXExternalAAWrapper::ID = 0;

INITIALIZE_PASS(NVPTXAAWrapperPass, "nvptx-aa",
                "NVPTX Address space based Alias Analysis", false, true)

INITIALIZE_PASS(NVPTXExternalAAWrapper, "nvptx-aa-wrapper",
                "NVPTX Address space based Alias Analysis Wrapper", false, true)

ImmutablePass *toolchain::createNVPTXAAWrapperPass() {
  return new NVPTXAAWrapperPass();
}

ImmutablePass *toolchain::createNVPTXExternalAAWrapperPass() {
  return new NVPTXExternalAAWrapper();
}

NVPTXAAWrapperPass::NVPTXAAWrapperPass() : ImmutablePass(ID) {}

void NVPTXAAWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

static unsigned getAddressSpace(const Value *V, unsigned MaxLookup) {
  // Find the first non-generic address space traversing the UD chain.
  // It is undefined behaviour if a pointer belongs to more than one
  // non-overlapping address spaces along a valid execution path.
  auto GetAS = [](const Value *V) -> unsigned {
    if (const auto *PTy = dyn_cast<PointerType>(V->getType()))
      return PTy->getAddressSpace();
    return ADDRESS_SPACE_GENERIC;
  };
  while (MaxLookup-- && GetAS(V) == ADDRESS_SPACE_GENERIC) {
    const Value *NewV = getUnderlyingObject(V, 1);
    if (NewV == V)
      break;
    V = NewV;
  }
  return GetAS(V);
}

static AliasResult::Kind getAliasResult(unsigned AS1, unsigned AS2) {
  if ((AS1 == ADDRESS_SPACE_GENERIC) || (AS2 == ADDRESS_SPACE_GENERIC))
    return AliasResult::MayAlias;

  // PTX s6.4.1.1. Generic Addressing:
  // A generic address maps to global memory unless it falls within
  // the window for const, local, or shared memory. The Kernel
  // Function Parameters (.param) window is contained within the
  // .global window.
  //
  // Therefore a global pointer may alias with a param pointer on some
  // GPUs via addrspacecast(param->generic->global) when cvta.param
  // instruction is used (PTX 7.7+ and SM_70+).
  //
  // TODO: cvta.param is not yet supported. We need to change aliasing
  // rules once it is added.

  // Distributed shared memory aliases with shared memory.
  if (((AS1 == ADDRESS_SPACE_SHARED) &&
       (AS2 == ADDRESS_SPACE_SHARED_CLUSTER)) ||
      ((AS1 == ADDRESS_SPACE_SHARED_CLUSTER) && (AS2 == ADDRESS_SPACE_SHARED)))
    return AliasResult::MayAlias;

  return (AS1 == AS2 ? AliasResult::MayAlias : AliasResult::NoAlias);
}

AliasResult NVPTXAAResult::alias(const MemoryLocation &Loc1,
                                 const MemoryLocation &Loc2, AAQueryInfo &AAQI,
                                 const Instruction *) {
  unsigned AS1 = getAddressSpace(Loc1.Ptr, TraverseAddressSpacesLimit);
  unsigned AS2 = getAddressSpace(Loc2.Ptr, TraverseAddressSpacesLimit);

  return getAliasResult(AS1, AS2);
}

// TODO: .param address space may be writable in presence of cvta.param, but
// this instruction is currently not supported. NVPTXLowerArgs also does not
// allow any writes to .param pointers.
static bool isConstOrParam(unsigned AS) {
  return AS == AddressSpace::ADDRESS_SPACE_CONST ||
         AS == AddressSpace::ADDRESS_SPACE_PARAM;
}

ModRefInfo NVPTXAAResult::getModRefInfoMask(const MemoryLocation &Loc,
                                            AAQueryInfo &AAQI,
                                            bool IgnoreLocals) {
  if (isConstOrParam(getAddressSpace(Loc.Ptr, TraverseAddressSpacesLimit)))
    return ModRefInfo::NoModRef;

  return ModRefInfo::ModRef;
}

MemoryEffects NVPTXAAResult::getMemoryEffects(const CallBase *Call,
                                              AAQueryInfo &AAQI) {
  // Inline assembly with no side-effect or memory clobbers should not
  // indirectly access memory in the PTX specification.
  if (const auto *IA = dyn_cast<InlineAsm>(Call->getCalledOperand())) {
    // Volatile is translated as side-effects.
    if (IA->hasSideEffects())
      return MemoryEffects::unknown();

    for (const InlineAsm::ConstraintInfo &Constraint : IA->ParseConstraints()) {
      // Indirect constraints (e.g. =*m) are unsupported in inline PTX.
      if (Constraint.isIndirect)
        return MemoryEffects::unknown();

      // Memory clobbers prevent optimization.
      if ((Constraint.Type & InlineAsm::ConstraintPrefix::isClobber) &&
          any_of(Constraint.Codes,
                 [](const auto &Code) { return Code == "{memory}"; }))
        return MemoryEffects::unknown();
    }
    return MemoryEffects::none();
  }

  return MemoryEffects::unknown();
}
