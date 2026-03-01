//===- AliasAnalysis.cpp - Alias Analysis for MLIR ------------------------===//
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

#include "mlir/Analysis/AliasAnalysis.h"
#include "mlir/Analysis/AliasAnalysis/LocalAliasAnalysis.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/Value.h"
#include "mlir/Support/LLVM.h"
#include <memory>

using namespace mlir;

//===----------------------------------------------------------------------===//
// AliasResult
//===----------------------------------------------------------------------===//

/// Merge this alias result with `other` and return a new result that
/// represents the conservative merge of both results.
AliasResult AliasResult::merge(AliasResult other) const {
  if (kind == other.kind)
    return *this;
  // A mix of PartialAlias and MustAlias is PartialAlias.
  if ((isPartial() && other.isMust()) || (other.isPartial() && isMust()))
    return PartialAlias;
  // Otherwise, don't assume anything.
  return MayAlias;
}

void AliasResult::print(raw_ostream &os) const {
  switch (kind) {
  case Kind::NoAlias:
    os << "NoAlias";
    break;
  case Kind::MayAlias:
    os << "MayAlias";
    break;
  case Kind::PartialAlias:
    os << "PartialAlias";
    break;
  case Kind::MustAlias:
    os << "MustAlias";
    break;
  }
}

//===----------------------------------------------------------------------===//
// ModRefResult
//===----------------------------------------------------------------------===//

void ModRefResult::print(raw_ostream &os) const {
  switch (kind) {
  case Kind::NoModRef:
    os << "NoModRef";
    break;
  case Kind::Ref:
    os << "Ref";
    break;
  case Kind::Mod:
    os << "Mod";
    break;
  case Kind::ModRef:
    os << "ModRef";
    break;
  }
}

//===----------------------------------------------------------------------===//
// AliasAnalysis
//===----------------------------------------------------------------------===//

AliasAnalysis::AliasAnalysis(Operation *op) {
  addAnalysisImplementation(LocalAliasAnalysis());
}

AliasResult AliasAnalysis::alias(Value lhs, Value rhs) {
  // Check each of the alias analysis implemenations for an alias result.
  for (const std::unique_ptr<Concept> &aliasImpl : aliasImpls) {
    AliasResult result = aliasImpl->alias(lhs, rhs);
    if (!result.isMay())
      return result;
  }
  return AliasResult::MayAlias;
}

ModRefResult AliasAnalysis::getModRef(Operation *op, Value location) {
  // Compute the mod-ref behavior by refining a top `ModRef` result with each of
  // the alias analysis implementations. We early exit at the point where we
  // refine down to a `NoModRef`.
  ModRefResult result = ModRefResult::getModAndRef();
  for (const std::unique_ptr<Concept> &aliasImpl : aliasImpls) {
    result = result.intersect(aliasImpl->getModRef(op, location));
    if (result.isNoModRef())
      return result;
  }
  return result;
}
