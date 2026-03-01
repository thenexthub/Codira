//===-- Bitcode/Reader/ValueList.h - Number values --------------*- C++ -*-===//
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
// This class gives values and types Unique ID's.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_BITCODE_READER_VALUELIST_H
#define LLVM_LIB_BITCODE_READER_VALUELIST_H

#include "vm/core/IR/ValueHandle.h"
#include "vm/core/Support/Error.h"
#include <cassert>
#include <utility>
#include <vector>

namespace vm::core {

class Error;
class Type;
class Value;

class BitcodeReaderValueList {
  /// Maps Value ID to pair of Value* and Type ID.
  std::vector<std::pair<WeakTrackingVH, unsigned>> ValuePtrs;

  /// Maximum number of valid references. Forward references exceeding the
  /// maximum must be invalid.
  unsigned RefsUpperBound;

  using MaterializeValueFnTy =
      std::function<Expected<Value *>(unsigned, BasicBlock *)>;
  MaterializeValueFnTy MaterializeValueFn;

public:
  BitcodeReaderValueList(size_t RefsUpperBound,
                         MaterializeValueFnTy MaterializeValueFn)
      : RefsUpperBound(std::min((size_t)std::numeric_limits<unsigned>::max(),
                                RefsUpperBound)),
        MaterializeValueFn(MaterializeValueFn) {}

  // vector compatibility methods
  unsigned size() const { return ValuePtrs.size(); }
  void resize(unsigned N) {
    ValuePtrs.resize(N);
  }
  void push_back(Value *V, unsigned TypeID) {
    ValuePtrs.emplace_back(V, TypeID);
  }

  void clear() {
    ValuePtrs.clear();
  }

  Value *operator[](unsigned i) const {
    assert(i < ValuePtrs.size());
    return ValuePtrs[i].first;
  }

  unsigned getTypeID(unsigned ValNo) const {
    assert(ValNo < ValuePtrs.size());
    return ValuePtrs[ValNo].second;
  }

  Value *back() const { return ValuePtrs.back().first; }
  void pop_back() {
    ValuePtrs.pop_back();
  }
  bool empty() const { return ValuePtrs.empty(); }

  void shrinkTo(unsigned N) {
    assert(N <= size() && "Invalid shrinkTo request!");
    ValuePtrs.resize(N);
  }

  void replaceValueWithoutRAUW(unsigned ValNo, Value *NewV) {
    assert(ValNo < ValuePtrs.size());
    ValuePtrs[ValNo].first = NewV;
  }

  Value *getValueFwdRef(unsigned Idx, Type *Ty, unsigned TyID,
                        BasicBlock *ConstExprInsertBB);

  Error assignValue(unsigned Idx, Value *V, unsigned TypeID);
};

} // end namespace vm::core

#endif // LLVM_LIB_BITCODE_READER_VALUELIST_H
