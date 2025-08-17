//==- BlockCounter.h - ADT for counting block visits -------------*- C++ -*-//
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

#include "language/Core/StaticAnalyzer/Core/PathSensitive/BlockCounter.h"
#include "toolchain/ADT/ImmutableMap.h"

using namespace language::Core;
using namespace ento;

namespace {

class CountKey {
  const StackFrameContext *CallSite;
  unsigned BlockID;

public:
  CountKey(const StackFrameContext *CS, unsigned ID)
    : CallSite(CS), BlockID(ID) {}

  bool operator==(const CountKey &RHS) const {
    return (CallSite == RHS.CallSite) && (BlockID == RHS.BlockID);
  }

  bool operator<(const CountKey &RHS) const {
    return std::tie(CallSite, BlockID) < std::tie(RHS.CallSite, RHS.BlockID);
  }

  void Profile(toolchain::FoldingSetNodeID &ID) const {
    ID.AddPointer(CallSite);
    ID.AddInteger(BlockID);
  }
};

}

typedef toolchain::ImmutableMap<CountKey, unsigned> CountMap;

static inline CountMap GetMap(void *D) {
  return CountMap(static_cast<CountMap::TreeTy*>(D));
}

static inline CountMap::Factory& GetFactory(void *F) {
  return *static_cast<CountMap::Factory*>(F);
}

unsigned BlockCounter::getNumVisited(const StackFrameContext *CallSite,
                                       unsigned BlockID) const {
  CountMap M = GetMap(Data);
  CountMap::data_type* T = M.lookup(CountKey(CallSite, BlockID));
  return T ? *T : 0;
}

BlockCounter::Factory::Factory(toolchain::BumpPtrAllocator& Alloc) {
  F = new CountMap::Factory(Alloc);
}

BlockCounter::Factory::~Factory() {
  delete static_cast<CountMap::Factory*>(F);
}

BlockCounter
BlockCounter::Factory::IncrementCount(BlockCounter BC,
                                        const StackFrameContext *CallSite,
                                        unsigned BlockID) {
  return BlockCounter(GetFactory(F).add(GetMap(BC.Data),
                                          CountKey(CallSite, BlockID),
                             BC.getNumVisited(CallSite, BlockID)+1).getRoot());
}

BlockCounter
BlockCounter::Factory::GetEmptyCounter() {
  return BlockCounter(GetFactory(F).getEmptyMap().getRoot());
}
