//===------- SEHFrameSupport.h - JITLink seh-frame utils --------*- C++ -*-===//
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
// SEHFrame utils for JITLink.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_EXECUTIONENGINE_JITLINK_SEHFRAMESUPPORT_H
#define LLVM_EXECUTIONENGINE_JITLINK_SEHFRAMESUPPORT_H

#include "vm/core/ADT/SetVector.h"
#include "vm/core/ExecutionEngine/JITLink/JITLink.h"
#include "vm/core/ExecutionEngine/JITSymbol.h"
#include "vm/core/Support/Error.h"
#include "vm/core/TargetParser/Triple.h"

namespace vm::core {
namespace jitlink {
/// This pass adds keep-alive edge from SEH frame sections
/// to the parent function content block.
class SEHFrameKeepAlivePass {
public:
  SEHFrameKeepAlivePass(StringRef SEHFrameSectionName)
      : SEHFrameSectionName(SEHFrameSectionName) {}

  Error operator()(LinkGraph &G) {
    auto *S = G.findSectionByName(SEHFrameSectionName);
    if (!S)
      return Error::success();

    // Simply consider every block pointed by seh frame block as parants.
    // This adds some unnecessary keep-alive edges to unwind info blocks,
    // (xdata) but these blocks are usually dead by default, so they wouldn't
    // count for the fate of seh frame block.
    for (auto *B : S->blocks()) {
      auto &DummySymbol = G.addAnonymousSymbol(*B, 0, 0, false, false);
      SetVector<Block *> Children;
      for (auto &E : B->edges()) {
        auto &Sym = E.getTarget();
        if (!Sym.isDefined())
          continue;
        Children.insert(&Sym.getBlock());
      }
      for (auto *Child : Children)
        Child->addEdge(Edge(Edge::KeepAlive, 0, DummySymbol, 0));
    }
    return Error::success();
  }

private:
  StringRef SEHFrameSectionName;
};

} // end namespace jitlink
} // end namespace vm::core

#endif // LLVM_EXECUTIONENGINE_JITLINK_SEHFRAMESUPPORT_H
