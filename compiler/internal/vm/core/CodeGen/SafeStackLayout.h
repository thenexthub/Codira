//===- SafeStackLayout.h - SafeStack frame layout --------------*- C++ -*--===//
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

#ifndef LLVM_LIB_CODEGEN_SAFESTACKLAYOUT_H
#define LLVM_LIB_CODEGEN_SAFESTACKLAYOUT_H

#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/Analysis/StackLifetime.h"
#include "vm/core/Support/Alignment.h"

namespace vm::core {

class raw_ostream;
class Value;

namespace safestack {

/// Compute the layout of an unsafe stack frame.
class StackLayout {
  Align MaxAlignment;

  struct StackRegion {
    unsigned Start;
    unsigned End;
    StackLifetime::LiveRange Range;

    StackRegion(unsigned Start, unsigned End,
                const StackLifetime::LiveRange &Range)
        : Start(Start), End(End), Range(Range) {}
  };

  /// The list of current stack regions, sorted by StackRegion::Start.
  SmallVector<StackRegion, 16> Regions;

  struct StackObject {
    const Value *Handle;
    unsigned Size;
    Align Alignment;
    StackLifetime::LiveRange Range;
  };

  SmallVector<StackObject, 8> StackObjects;

  DenseMap<const Value *, unsigned> ObjectOffsets;
  DenseMap<const Value *, Align> ObjectAlignments;

  void layoutObject(StackObject &Obj);

public:
  StackLayout(Align StackAlignment) : MaxAlignment(StackAlignment) {}

  /// Add an object to the stack frame. Value pointer is opaque and used as a
  /// handle to retrieve the object's offset in the frame later.
  void addObject(const Value *V, unsigned Size, Align Alignment,
                 const StackLifetime::LiveRange &Range);

  /// Run the layout computation for all previously added objects.
  void computeLayout();

  /// Returns the offset to the object start in the stack frame.
  unsigned getObjectOffset(const Value *V) { return ObjectOffsets[V]; }

  /// Returns the alignment of the object
  Align getObjectAlignment(const Value *V) { return ObjectAlignments[V]; }

  /// Returns the size of the entire frame.
  unsigned getFrameSize() { return Regions.empty() ? 0 : Regions.back().End; }

  /// Returns the alignment of the frame.
  Align getFrameAlignment() { return MaxAlignment; }

  void print(raw_ostream &OS);
};

} // end namespace safestack

} // end namespace vm::core

#endif // LLVM_LIB_CODEGEN_SAFESTACKLAYOUT_H
