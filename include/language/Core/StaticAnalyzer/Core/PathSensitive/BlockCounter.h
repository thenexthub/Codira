//==- BlockCounter.h - ADT for counting block visits ---------------*- C++ -*-//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//  This file defines BlockCounter, an abstract data type used to count
//  the number of times a given block has been visited along a path
//  analyzed by CoreEngine.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_BLOCKCOUNTER_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_BLOCKCOUNTER_H

#include "toolchain/Support/Allocator.h"

namespace language::Core {

class StackFrameContext;

namespace ento {

/// \class BlockCounter
/// An abstract data type used to count the number of times a given
/// block has been visited along a path analyzed by CoreEngine.
class BlockCounter {
  void *Data;

  BlockCounter(void *D) : Data(D) {}

public:
  BlockCounter() : Data(nullptr) {}

  unsigned getNumVisited(const StackFrameContext *CallSite,
                         unsigned BlockID) const;

  class Factory {
    void *F;
  public:
    Factory(toolchain::BumpPtrAllocator& Alloc);
    ~Factory();

    BlockCounter GetEmptyCounter();
    BlockCounter IncrementCount(BlockCounter BC,
                                  const StackFrameContext *CallSite,
                                  unsigned BlockID);
  };

  friend class Factory;
};

} // end GR namespace

} // end clang namespace

#endif
