//===--------------- PerGraphGOTAndPLTStubBuilder.h -------------*- C++ -*-===//
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
// Construct GOT and PLT entries for each graph.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_EXECUTIONENGINE_JITLINK_PERGRAPHGOTANDPLTSTUBSBUILDER_H
#define LLVM_EXECUTIONENGINE_JITLINK_PERGRAPHGOTANDPLTSTUBSBUILDER_H

#include "vm/core/ExecutionEngine/JITLink/JITLink.h"
#include "vm/core/Support/Debug.h"

#define DEBUG_TYPE "jitlink"

namespace vm::core {
namespace jitlink {

/// Per-object GOT and PLT Stub builder.
///
/// Constructs GOT entries and PLT stubs in every graph for referenced symbols.
/// Building these blocks in every graph is likely to lead to duplicate entries
/// in the JITLinkDylib, but allows graphs to be trivially removed independently
/// without affecting other graphs (since those other graphs will have their own
/// copies of any required entries).
template <typename BuilderImplT>
class PerGraphGOTAndPLTStubsBuilder {
public:
  PerGraphGOTAndPLTStubsBuilder(LinkGraph &G) : G(G) {}

  static Error asPass(LinkGraph &G) { return BuilderImplT(G).run(); }

  Error run() {
    LLVM_DEBUG(dbgs() << "Running Per-Graph GOT and Stubs builder:\n");

    // We're going to be adding new blocks, but we don't want to iterate over
    // the new ones, so build a worklist.
    std::vector<Block *> Worklist(G.blocks().begin(), G.blocks().end());

    for (auto *B : Worklist)
      for (auto &E : B->edges()) {
        if (impl().isGOTEdgeToFix(E)) {
          LLVM_DEBUG({
            dbgs() << "  Fixing " << G.getEdgeKindName(E.getKind())
                   << " edge at " << B->getFixupAddress(E) << " ("
                   << B->getAddress() << " + "
                   << formatv("{0:x}", E.getOffset()) << ")\n";
          });
          impl().fixGOTEdge(E, getGOTEntry(E.getTarget()));
        } else if (impl().isExternalBranchEdge(E)) {
          LLVM_DEBUG({
            dbgs() << "  Fixing " << G.getEdgeKindName(E.getKind())
                   << " edge at " << B->getFixupAddress(E) << " ("
                   << B->getAddress() << " + "
                   << formatv("{0:x}", E.getOffset()) << ")\n";
          });
          impl().fixPLTEdge(E, getPLTStub(E.getTarget()));
        }
      }

    return Error::success();
  }

protected:
  Symbol &getGOTEntry(Symbol &Target) {
    assert(Target.hasName() && "GOT edge cannot point to anonymous target");

    auto GOTEntryI = GOTEntries.find(Target.getName());

    // Build the entry if it doesn't exist.
    if (GOTEntryI == GOTEntries.end()) {
      auto &GOTEntry = impl().createGOTEntry(Target);
      LLVM_DEBUG({
        dbgs() << "    Created GOT entry for " << Target.getName() << ": "
               << GOTEntry << "\n";
      });
      GOTEntryI =
          GOTEntries.insert(std::make_pair(Target.getName(), &GOTEntry)).first;
    }

    assert(GOTEntryI != GOTEntries.end() && "Could not get GOT entry symbol");
    LLVM_DEBUG(
        { dbgs() << "    Using GOT entry " << *GOTEntryI->second << "\n"; });
    return *GOTEntryI->second;
  }

  Symbol &getPLTStub(Symbol &Target) {
    assert(Target.hasName() &&
           "External branch edge can not point to an anonymous target");
    auto StubI = PLTStubs.find(Target.getName());

    if (StubI == PLTStubs.end()) {
      auto &StubSymbol = impl().createPLTStub(Target);
      LLVM_DEBUG({
        dbgs() << "    Created PLT stub for " << Target.getName() << ": "
               << StubSymbol << "\n";
      });
      StubI =
          PLTStubs.insert(std::make_pair(Target.getName(), &StubSymbol)).first;
    }

    assert(StubI != PLTStubs.end() && "Count not get stub symbol");
    LLVM_DEBUG({ dbgs() << "    Using PLT stub " << *StubI->second << "\n"; });
    return *StubI->second;
  }

  LinkGraph &G;

private:
  BuilderImplT &impl() { return static_cast<BuilderImplT &>(*this); }

  DenseMap<orc::SymbolStringPtr, Symbol *> GOTEntries;
  DenseMap<orc::SymbolStringPtr, Symbol *> PLTStubs;
};

} // end namespace jitlink
} // end namespace vm::core

#undef DEBUG_TYPE

#endif // LLVM_EXECUTIONENGINE_JITLINK_PERGRAPHGOTANDPLTSTUBSBUILDER_H
