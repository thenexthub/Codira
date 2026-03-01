//===- SplitModule.cpp - Split a module into partitions -------------------===//
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
// This file defines the function toolchain::SplitModule, which splits a module
// into multiple linkable partitions. It can be used to implement parallel code
// generation for link-time optimization.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/SplitModule.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/EquivalenceClasses.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/IR/Comdat.h"
#include "vm/core/IR/Constant.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/GlobalAlias.h"
#include "vm/core/IR/GlobalObject.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/User.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/MD5.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Transforms/Utils/Cloning.h"
#include "vm/core/Transforms/Utils/ValueMapper.h"
#include <cassert>
#include <iterator>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

using namespace vm::core;

#define DEBUG_TYPE "split-module"

namespace {

using ClusterMapType = EquivalenceClasses<const GlobalValue *>;
using ComdatMembersType = DenseMap<const Comdat *, const GlobalValue *>;
using ClusterIDMapType = DenseMap<const GlobalValue *, unsigned>;

bool compareClusters(const std::pair<unsigned, unsigned> &A,
                     const std::pair<unsigned, unsigned> &B) {
  if (A.second || B.second)
    return A.second > B.second;
  return A.first > B.first;
}

using BalancingQueueType =
    std::priority_queue<std::pair<unsigned, unsigned>,
                        std::vector<std::pair<unsigned, unsigned>>,
                        decltype(compareClusters) *>;

} // end anonymous namespace

static void addNonConstUser(ClusterMapType &GVtoClusterMap,
                            const GlobalValue *GV, const User *U) {
  assert((!isa<Constant>(U) || isa<GlobalValue>(U)) && "Bad user");

  if (const Instruction *I = dyn_cast<Instruction>(U)) {
    const GlobalValue *F = I->getParent()->getParent();
    GVtoClusterMap.unionSets(GV, F);
  } else if (const GlobalValue *GVU = dyn_cast<GlobalValue>(U)) {
    GVtoClusterMap.unionSets(GV, GVU);
  } else {
    llvm_unreachable("Underimplemented use case");
  }
}

// Adds all GlobalValue users of V to the same cluster as GV.
static void addAllGlobalValueUsers(ClusterMapType &GVtoClusterMap,
                                   const GlobalValue *GV, const Value *V) {
  for (const auto *U : V->users()) {
    SmallVector<const User *, 4> Worklist;
    Worklist.push_back(U);
    while (!Worklist.empty()) {
      const User *UU = Worklist.pop_back_val();
      // For each constant that is not a GV (a pure const) recurse.
      if (isa<Constant>(UU) && !isa<GlobalValue>(UU)) {
        Worklist.append(UU->user_begin(), UU->user_end());
        continue;
      }
      addNonConstUser(GVtoClusterMap, GV, UU);
    }
  }
}

static const GlobalObject *getGVPartitioningRoot(const GlobalValue *GV) {
  const GlobalObject *GO = GV->getAliaseeObject();
  if (const auto *GI = dyn_cast_or_null<GlobalIFunc>(GO))
    GO = GI->getResolverFunction();
  return GO;
}

// Find partitions for module in the way that no locals need to be
// globalized.
// Try to balance pack those partitions into N files since this roughly equals
// thread balancing for the backend codegen step.
static void findPartitions(Module &M, ClusterIDMapType &ClusterIDMap,
                           unsigned N) {
  // At this point module should have the proper mix of globals and locals.
  // As we attempt to partition this module, we must not change any
  // locals to globals.
  LLVM_DEBUG(dbgs() << "Partition module with (" << M.size()
                    << ") functions\n");
  ClusterMapType GVtoClusterMap;
  ComdatMembersType ComdatMembers;

  auto recordGVSet = [&GVtoClusterMap, &ComdatMembers](GlobalValue &GV) {
    if (GV.isDeclaration())
      return;

    if (!GV.hasName())
      GV.setName("__llvmsplit_unnamed");

    // Comdat groups must not be partitioned. For comdat groups that contain
    // locals, record all their members here so we can keep them together.
    // Comdat groups that only contain external globals are already handled by
    // the MD5-based partitioning.
    if (const Comdat *C = GV.getComdat()) {
      auto &Member = ComdatMembers[C];
      if (Member)
        GVtoClusterMap.unionSets(Member, &GV);
      else
        Member = &GV;
    }

    // Aliases should not be separated from their aliasees and ifuncs should
    // not be separated from their resolvers regardless of linkage.
    if (const GlobalObject *Root = getGVPartitioningRoot(&GV))
      if (&GV != Root)
        GVtoClusterMap.unionSets(&GV, Root);

    if (const Function *F = dyn_cast<Function>(&GV)) {
      for (const BasicBlock &BB : *F) {
        BlockAddress *BA = BlockAddress::lookup(&BB);
        if (!BA || !BA->isConstantUsed())
          continue;
        addAllGlobalValueUsers(GVtoClusterMap, F, BA);
      }
    }

    if (GV.hasLocalLinkage())
      addAllGlobalValueUsers(GVtoClusterMap, &GV, &GV);
  };

  toolchain::for_each(M.functions(), recordGVSet);
  toolchain::for_each(M.globals(), recordGVSet);
  toolchain::for_each(M.aliases(), recordGVSet);

  // Assigned all GVs to merged clusters while balancing number of objects in
  // each.
  BalancingQueueType BalancingQueue(compareClusters);
  // Pre-populate priority queue with N slot blanks.
  for (unsigned i = 0; i < N; ++i)
    BalancingQueue.push(std::make_pair(i, 0));

  SmallPtrSet<const GlobalValue *, 32> Visited;

  // To guarantee determinism, we have to sort SCC according to size.
  // When size is the same, use leader's name.
  for (const auto &C : GVtoClusterMap) {
    if (!C->isLeader())
      continue;

    unsigned CurrentClusterID = BalancingQueue.top().first;
    unsigned CurrentClusterSize = BalancingQueue.top().second;
    BalancingQueue.pop();

    LLVM_DEBUG(dbgs() << "Root[" << CurrentClusterID << "] cluster_size("
                      << std::distance(GVtoClusterMap.member_begin(*C),
                                       GVtoClusterMap.member_end())
                      << ") ----> " << C->getData()->getName() << "\n");

    for (ClusterMapType::member_iterator MI = GVtoClusterMap.findLeader(*C);
         MI != GVtoClusterMap.member_end(); ++MI) {
      if (!Visited.insert(*MI).second)
        continue;
      LLVM_DEBUG(dbgs() << "----> " << (*MI)->getName()
                        << ((*MI)->hasLocalLinkage() ? " l " : " e ") << "\n");
      Visited.insert(*MI);
      ClusterIDMap[*MI] = CurrentClusterID;
      CurrentClusterSize++;
    }
    // Add this set size to the number of entries in this cluster.
    BalancingQueue.push(std::make_pair(CurrentClusterID, CurrentClusterSize));
  }
}

static void externalize(GlobalValue *GV) {
  if (GV->hasLocalLinkage()) {
    GV->setLinkage(GlobalValue::ExternalLinkage);
    GV->setVisibility(GlobalValue::HiddenVisibility);
  }

  // Unnamed entities must be named consistently between modules. setName will
  // give a distinct name to each such entity.
  if (!GV->hasName())
    GV->setName("__llvmsplit_unnamed");
}

// Returns whether GV should be in partition (0-based) I of N.
static bool isInPartition(const GlobalValue *GV, unsigned I, unsigned N) {
  if (const GlobalObject *Root = getGVPartitioningRoot(GV))
    GV = Root;

  StringRef Name;
  if (const Comdat *C = GV->getComdat())
    Name = C->getName();
  else
    Name = GV->getName();

  // Partition by MD5 hash. We only need a few bits for evenness as the number
  // of partitions will generally be in the 1-2 figure range; the low 16 bits
  // are enough.
  MD5 H;
  MD5::MD5Result R;
  H.update(Name);
  H.final(R);
  return (R[0] | (R[1] << 8)) % N == I;
}

void toolchain::SplitModule(
    Module &M, unsigned N,
    function_ref<void(std::unique_ptr<Module> MPart)> ModuleCallback,
    bool PreserveLocals, bool RoundRobin) {
  if (!PreserveLocals) {
    for (Function &F : M)
      externalize(&F);
    for (GlobalVariable &GV : M.globals())
      externalize(&GV);
    for (GlobalAlias &GA : M.aliases())
      externalize(&GA);
    for (GlobalIFunc &GIF : M.ifuncs())
      externalize(&GIF);
  }

  // This performs splitting without a need for externalization, which might not
  // always be possible.
  ClusterIDMapType ClusterIDMap;
  findPartitions(M, ClusterIDMap, N);

  // Find functions not mapped to modules in ClusterIDMap and count functions
  // per module. Map unmapped functions using round-robin so that they skip
  // being distributed by isInPartition() based on function name hashes below.
  // This provides better uniformity of distribution of functions to modules
  // in some cases - for example when the number of functions equals to N.
  if (RoundRobin) {
    DenseMap<unsigned, unsigned> ModuleFunctionCount;
    SmallVector<const GlobalValue *> UnmappedFunctions;
    for (const auto &F : M.functions()) {
      if (F.isDeclaration() ||
          F.getLinkage() != GlobalValue::LinkageTypes::ExternalLinkage)
        continue;
      auto It = ClusterIDMap.find(&F);
      if (It == ClusterIDMap.end())
        UnmappedFunctions.push_back(&F);
      else
        ++ModuleFunctionCount[It->second];
    }
    BalancingQueueType BalancingQueue(compareClusters);
    for (unsigned I = 0; I < N; ++I) {
      if (auto It = ModuleFunctionCount.find(I);
          It != ModuleFunctionCount.end())
        BalancingQueue.push(*It);
      else
        BalancingQueue.push({I, 0});
    }
    for (const auto *const F : UnmappedFunctions) {
      const unsigned I = BalancingQueue.top().first;
      const unsigned Count = BalancingQueue.top().second;
      BalancingQueue.pop();
      ClusterIDMap.insert({F, I});
      BalancingQueue.push({I, Count + 1});
    }
  }

  // FIXME: We should be able to reuse M as the last partition instead of
  // cloning it. Note that the callers at the moment expect the module to
  // be preserved, so will need some adjustments as well.
  for (unsigned I = 0; I < N; ++I) {
    ValueToValueMapTy VMap;
    std::unique_ptr<Module> MPart(
        CloneModule(M, VMap, [&](const GlobalValue *GV) {
          if (auto It = ClusterIDMap.find(GV); It != ClusterIDMap.end())
            return It->second == I;
          else
            return isInPartition(GV, I, N);
        }));
    if (I != 0)
      MPart->setModuleInlineAsm("");
    ModuleCallback(std::move(MPart));
  }
}
