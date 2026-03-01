//===- toolchain/CodeGen/MachineModuleInfoImpls.cpp ----------------------------===//
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
// This file implements object-file format specific implementations of
// MachineModuleInfoImpl.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineModuleInfoImpls.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Module.h"
#include "vm/core/MC/MCSymbol.h"

using namespace vm::core;

//===----------------------------------------------------------------------===//
// MachineModuleInfoMachO
//===----------------------------------------------------------------------===//

// Out of line virtual method.
void MachineModuleInfoMachO::anchor() {}
void MachineModuleInfoELF::anchor() {}
void MachineModuleInfoCOFF::anchor() {}
void MachineModuleInfoWasm::anchor() {}

using PairTy = std::pair<MCSymbol *, MachineModuleInfoImpl::StubValueTy>;
static int SortSymbolPair(const PairTy *LHS, const PairTy *RHS) {
  return LHS->first->getName().compare(RHS->first->getName());
}

MachineModuleInfoImpl::SymbolListTy MachineModuleInfoImpl::getSortedStubs(
    DenseMap<MCSymbol *, MachineModuleInfoImpl::StubValueTy> &Map) {
  MachineModuleInfoImpl::SymbolListTy List(Map.begin(), Map.end());

  array_pod_sort(List.begin(), List.end(), SortSymbolPair);

  Map.clear();
  return List;
}

using ExprStubPairTy = std::pair<MCSymbol *, const MCExpr *>;
static int SortAuthStubPair(const ExprStubPairTy *LHS,
                            const ExprStubPairTy *RHS) {
  return LHS->first->getName().compare(RHS->first->getName());
}

MachineModuleInfoImpl::ExprStubListTy MachineModuleInfoImpl::getSortedExprStubs(
    DenseMap<MCSymbol *, const MCExpr *> &ExprStubs) {
  MachineModuleInfoImpl::ExprStubListTy List(ExprStubs.begin(),
                                             ExprStubs.end());

  array_pod_sort(List.begin(), List.end(), SortAuthStubPair);

  ExprStubs.clear();
  return List;
}

MachineModuleInfoELF::MachineModuleInfoELF(const MachineModuleInfo &MMI) {
  const Module *M = MMI.getModule();
  const auto *Flag = mdconst::extract_or_null<ConstantInt>(
      M->getModuleFlag("ptrauth-sign-personality"));
  HasSignedPersonality = Flag && Flag->getZExtValue() == 1;
}
