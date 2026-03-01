//===- Assumptions.cpp ------ Collection of helpers for assumptions -------===//
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
//  This file implements helper functions for accessing assumption infomration
//  inside of the "toolchain.assume" metadata.
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/Assumptions.h"
#include "vm/core/ADT/SetOperations.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/IR/Attributes.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/InstrTypes.h"

using namespace vm::core;

static bool hasAssumption(const Attribute &A,
                          const KnownAssumptionString &AssumptionStr) {
  if (!A.isValid())
    return false;
  assert(A.isStringAttribute() && "Expected a string attribute!");

  SmallVector<StringRef, 8> Strings;
  A.getValueAsString().split(Strings, ",");

  return toolchain::is_contained(Strings, AssumptionStr);
}

static DenseSet<StringRef> getAssumptions(const Attribute &A) {
  if (!A.isValid())
    return DenseSet<StringRef>();
  assert(A.isStringAttribute() && "Expected a string attribute!");

  DenseSet<StringRef> Assumptions;
  SmallVector<StringRef, 8> Strings;
  A.getValueAsString().split(Strings, ",");

  Assumptions.insert_range(Strings);
  return Assumptions;
}

template <typename AttrSite>
static bool addAssumptionsImpl(AttrSite &Site,
                               const DenseSet<StringRef> &Assumptions) {
  if (Assumptions.empty())
    return false;

  DenseSet<StringRef> CurAssumptions = getAssumptions(Site);

  if (!set_union(CurAssumptions, Assumptions))
    return false;

  LLVMContext &Ctx = Site.getContext();
  Site.addFnAttr(toolchain::Attribute::get(
      Ctx, toolchain::AssumptionAttrKey,
      toolchain::join(CurAssumptions.begin(), CurAssumptions.end(), ",")));

  return true;
}

bool toolchain::hasAssumption(const Function &F,
                         const KnownAssumptionString &AssumptionStr) {
  const Attribute &A = F.getFnAttribute(AssumptionAttrKey);
  return ::hasAssumption(A, AssumptionStr);
}

bool toolchain::hasAssumption(const CallBase &CB,
                         const KnownAssumptionString &AssumptionStr) {
  if (Function *F = CB.getCalledFunction())
    if (hasAssumption(*F, AssumptionStr))
      return true;

  const Attribute &A = CB.getFnAttr(AssumptionAttrKey);
  return ::hasAssumption(A, AssumptionStr);
}

DenseSet<StringRef> toolchain::getAssumptions(const Function &F) {
  const Attribute &A = F.getFnAttribute(AssumptionAttrKey);
  return ::getAssumptions(A);
}

DenseSet<StringRef> toolchain::getAssumptions(const CallBase &CB) {
  const Attribute &A = CB.getFnAttr(AssumptionAttrKey);
  return ::getAssumptions(A);
}

bool toolchain::addAssumptions(Function &F, const DenseSet<StringRef> &Assumptions) {
  return ::addAssumptionsImpl(F, Assumptions);
}

bool toolchain::addAssumptions(CallBase &CB,
                          const DenseSet<StringRef> &Assumptions) {
  return ::addAssumptionsImpl(CB, Assumptions);
}

StringSet<> &toolchain::getKnownAssumptionStrings() {
  static StringSet<> Object({
      "omp_no_openmp",            // OpenMP 5.1
      "omp_no_openmp_routines",   // OpenMP 5.1
      "omp_no_parallelism",       // OpenMP 5.1
      "omp_no_openmp_constructs", // OpenMP 6.0
      "ompx_spmd_amenable",       // OpenMPOpt extension
      "ompx_no_call_asm",         // OpenMPOpt extension
      "ompx_aligned_barrier",     // OpenMPOpt extension
  });

  return Object;
}
