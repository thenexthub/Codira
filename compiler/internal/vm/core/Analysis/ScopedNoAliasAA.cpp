//===- ScopedNoAliasAA.cpp - Scoped No-Alias Alias Analysis ---------------===//
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
// This file defines the ScopedNoAlias alias-analysis pass, which implements
// metadata-based scoped no-alias support.
//
// Alias-analysis scopes are defined by an id (which can be a string or some
// other metadata node), a domain node, and an optional descriptive string.
// A domain is defined by an id (which can be a string or some other metadata
// node), and an optional descriptive string.
//
// !dom0 =   metadata !{ metadata !"domain of foo()" }
// !scope1 = metadata !{ metadata !scope1, metadata !dom0, metadata !"scope 1" }
// !scope2 = metadata !{ metadata !scope2, metadata !dom0, metadata !"scope 2" }
//
// Loads and stores can be tagged with an alias-analysis scope, and also, with
// a noalias tag for a specific scope:
//
// ... = load %ptr1, !alias.scope !{ !scope1 }
// ... = load %ptr2, !alias.scope !{ !scope1, !scope2 }, !noalias !{ !scope1 }
//
// When evaluating an aliasing query, if one of the instructions is associated
// has a set of noalias scopes in some domain that is a superset of the alias
// scopes in that domain of some other instruction, then the two memory
// accesses are assumed not to alias.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/ScopedNoAliasAA.h"
#include "vm/core/ADT/SetOperations.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/Analysis/MemoryLocation.h"
#include "vm/core/IR/InstrTypes.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/CommandLine.h"

using namespace vm::core;

// A handy option for disabling scoped no-alias functionality. The same effect
// can also be achieved by stripping the associated metadata tags from IR, but
// this option is sometimes more convenient.
static cl::opt<bool> EnableScopedNoAlias("enable-scoped-noalias",
                                         cl::init(true), cl::Hidden);

AliasResult ScopedNoAliasAAResult::alias(const MemoryLocation &LocA,
                                         const MemoryLocation &LocB,
                                         AAQueryInfo &AAQI,
                                         const Instruction *) {
  if (!EnableScopedNoAlias)
    return AliasResult::MayAlias;

  // Get the attached MDNodes.
  const MDNode *AScopes = LocA.AATags.Scope, *BScopes = LocB.AATags.Scope;

  const MDNode *ANoAlias = LocA.AATags.NoAlias, *BNoAlias = LocB.AATags.NoAlias;

  if (!mayAliasInScopes(AScopes, BNoAlias))
    return AliasResult::NoAlias;

  if (!mayAliasInScopes(BScopes, ANoAlias))
    return AliasResult::NoAlias;

  return AliasResult::MayAlias;
}

ModRefInfo ScopedNoAliasAAResult::getModRefInfo(const CallBase *Call,
                                                const MemoryLocation &Loc,
                                                AAQueryInfo &AAQI) {
  if (!EnableScopedNoAlias)
    return ModRefInfo::ModRef;

  if (!mayAliasInScopes(Loc.AATags.Scope,
                        Call->getMetadata(LLVMContext::MD_noalias)))
    return ModRefInfo::NoModRef;

  if (!mayAliasInScopes(Call->getMetadata(LLVMContext::MD_alias_scope),
                        Loc.AATags.NoAlias))
    return ModRefInfo::NoModRef;

  return ModRefInfo::ModRef;
}

ModRefInfo ScopedNoAliasAAResult::getModRefInfo(const CallBase *Call1,
                                                const CallBase *Call2,
                                                AAQueryInfo &AAQI) {
  if (!EnableScopedNoAlias)
    return ModRefInfo::ModRef;

  if (!mayAliasInScopes(Call1->getMetadata(LLVMContext::MD_alias_scope),
                        Call2->getMetadata(LLVMContext::MD_noalias)))
    return ModRefInfo::NoModRef;

  if (!mayAliasInScopes(Call2->getMetadata(LLVMContext::MD_alias_scope),
                        Call1->getMetadata(LLVMContext::MD_noalias)))
    return ModRefInfo::NoModRef;

  return ModRefInfo::ModRef;
}

static void collectMDInDomain(const MDNode *List, const MDNode *Domain,
                              SmallPtrSetImpl<const MDNode *> &Nodes) {
  for (const MDOperand &MDOp : List->operands())
    if (const MDNode *MD = dyn_cast<MDNode>(MDOp))
      if (AliasScopeNode(MD).getDomain() == Domain)
        Nodes.insert(MD);
}

/// Collect the set of scoped domains relevant to the noalias scopes.
void ScopedNoAliasAAResult::collectScopedDomains(
    const MDNode *NoAlias, SmallPtrSetImpl<const MDNode *> &Domains) {
  if (!NoAlias)
    return;
  assert(Domains.empty() && "Domains should be empty");
  for (const MDOperand &MDOp : NoAlias->operands())
    if (const MDNode *NAMD = dyn_cast<MDNode>(MDOp))
      if (const MDNode *Domain = AliasScopeNode(NAMD).getDomain())
        Domains.insert(Domain);
}

bool ScopedNoAliasAAResult::mayAliasInScopes(const MDNode *Scopes,
                                             const MDNode *NoAlias) {
  if (!Scopes || !NoAlias)
    return true;

  // Collect the set of scope domains relevant to the noalias scopes.
  SmallPtrSet<const MDNode *, 16> Domains;
  collectScopedDomains(NoAlias, Domains);

  // We alias unless, for some domain, the set of noalias scopes in that domain
  // is a superset of the set of alias scopes in that domain.
  for (const MDNode *Domain : Domains) {
    SmallPtrSet<const MDNode *, 16> ScopeNodes;
    collectMDInDomain(Scopes, Domain, ScopeNodes);
    if (ScopeNodes.empty())
      continue;

    SmallPtrSet<const MDNode *, 16> NANodes;
    collectMDInDomain(NoAlias, Domain, NANodes);

    // To not alias, all of the nodes in ScopeNodes must be in NANodes.
    if (toolchain::set_is_subset(ScopeNodes, NANodes))
      return false;
  }

  return true;
}

AnalysisKey ScopedNoAliasAA::Key;

ScopedNoAliasAAResult ScopedNoAliasAA::run(Function &F,
                                           FunctionAnalysisManager &AM) {
  return ScopedNoAliasAAResult();
}

char ScopedNoAliasAAWrapperPass::ID = 0;

INITIALIZE_PASS(ScopedNoAliasAAWrapperPass, "scoped-noalias-aa",
                "Scoped NoAlias Alias Analysis", false, true)

ImmutablePass *toolchain::createScopedNoAliasAAWrapperPass() {
  return new ScopedNoAliasAAWrapperPass();
}

ScopedNoAliasAAWrapperPass::ScopedNoAliasAAWrapperPass() : ImmutablePass(ID) {}

bool ScopedNoAliasAAWrapperPass::doInitialization(Module &M) {
  Result.reset(new ScopedNoAliasAAResult());
  return false;
}

bool ScopedNoAliasAAWrapperPass::doFinalization(Module &M) {
  Result.reset();
  return false;
}

void ScopedNoAliasAAWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}
