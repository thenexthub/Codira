//===- ProvenanceAnalysis.h - ObjC ARC Optimization -------------*- C++ -*-===//
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
/// \file
///
/// This file declares a special form of Alias Analysis called ``Provenance
/// Analysis''. The word ``provenance'' refers to the history of the ownership
/// of an object. Thus ``Provenance Analysis'' is an analysis which attempts to
/// use various techniques to determine if locally
///
/// WARNING: This file knows about certain library functions. It recognizes them
/// by name, and hardwires knowledge of their semantics.
///
/// WARNING: This file knows about how certain Objective-C library functions are
/// used. Naive LLVM IR transformations which would otherwise be
/// behavior-preserving may break these assumptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_OBJCARC_PROVENANCEANALYSIS_H
#define LLVM_LIB_TRANSFORMS_OBJCARC_PROVENANCEANALYSIS_H

#include "vm/core/ADT/DenseMap.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/ValueHandle.h"
#include <utility>

namespace vm::core {

class AAResults;
class PHINode;
class SelectInst;
class Value;

namespace objcarc {

/// This is similar to BasicAliasAnalysis, and it uses many of the same
/// techniques, except it uses special ObjC-specific reasoning about pointer
/// relationships.
///
/// In this context ``Provenance'' is defined as the history of an object's
/// ownership. Thus ``Provenance Analysis'' is defined by using the notion of
/// an ``independent provenance source'' of a pointer to determine whether or
/// not two pointers have the same provenance source and thus could
/// potentially be related.
class ProvenanceAnalysis {
  AAResults *AA;

  using ValuePairTy = std::pair<const Value *, const Value *>;
  using CachedResultsTy = DenseMap<ValuePairTy, bool>;

  CachedResultsTy CachedResults;

  DenseMap<const Value *, std::pair<WeakVH, WeakTrackingVH>>
      UnderlyingObjCPtrCache;

  bool relatedCheck(const Value *A, const Value *B);
  bool relatedSelect(const SelectInst *A, const Value *B);
  bool relatedPHI(const PHINode *A, const Value *B);

public:
  ProvenanceAnalysis() = default;
  ProvenanceAnalysis(const ProvenanceAnalysis &) = delete;
  ProvenanceAnalysis &operator=(const ProvenanceAnalysis &) = delete;

  void setAA(AAResults *aa) { AA = aa; }

  AAResults *getAA() const { return AA; }

  bool related(const Value *A, const Value *B);

  void clear() {
    CachedResults.clear();
    UnderlyingObjCPtrCache.clear();
  }
};

} // end namespace objcarc

} // end namespace vm::core

#endif // LLVM_LIB_TRANSFORMS_OBJCARC_PROVENANCEANALYSIS_H
