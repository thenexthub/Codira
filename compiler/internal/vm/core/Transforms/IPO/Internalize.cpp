//===-- Internalize.cpp - Mark functions internal -------------------------===//
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
// This pass loops over all of the functions and variables in the input module.
// If the function or variable does not need to be preserved according to the
// client supplied callback, it is marked as internal.
//
// This transformation would not be legal in a regular compilation, but it gets
// extra information from the linker about what is safe.
//
// For example: Internalizing a function with external linkage. Only if we are
// told it is only used from within this module, it is safe to do it.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/IPO/Internalize.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/ADT/StringSet.h"
#include "vm/core/Analysis/CallGraph.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/GlobPattern.h"
#include "vm/core/Support/LineIterator.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/TargetParser/Triple.h"
#include "vm/core/Transforms/IPO.h"
using namespace vm::core;

#define DEBUG_TYPE "internalize"

STATISTIC(NumAliases, "Number of aliases internalized");
STATISTIC(NumFunctions, "Number of functions internalized");
STATISTIC(NumGlobals, "Number of global vars internalized");

// APIFile - A file which contains a list of symbol glob patterns that should
// not be marked external.
static cl::opt<std::string>
    APIFile("internalize-public-api-file", cl::value_desc("filename"),
            cl::desc("A file containing list of symbol names to preserve"));

// APIList - A list of symbol glob patterns that should not be marked internal.
static cl::list<std::string>
    APIList("internalize-public-api-list", cl::value_desc("list"),
            cl::desc("A list of symbol names to preserve"), cl::CommaSeparated);

namespace {
// Helper to load an API list to preserve from file and expose it as a functor
// for internalization.
class PreserveAPIList {
public:
  PreserveAPIList() {
    if (!APIFile.empty())
      LoadFile(APIFile);
    for (StringRef Pattern : APIList)
      addGlob(Pattern);
  }

  bool operator()(const GlobalValue &GV) {
    return toolchain::any_of(
        ExternalNames, [&](GlobPattern &GP) { return GP.match(GV.getName()); });
  }

private:
  // Contains the set of symbols loaded from file
  SmallVector<GlobPattern> ExternalNames;

  void addGlob(StringRef Pattern) {
    auto GlobOrErr = GlobPattern::create(Pattern);
    if (!GlobOrErr) {
      errs() << "WARNING: when loading pattern: '"
             << toString(GlobOrErr.takeError()) << "' ignoring";
      return;
    }
    ExternalNames.emplace_back(std::move(*GlobOrErr));
  }

  void LoadFile(StringRef Filename) {
    // Load the APIFile...
    ErrorOr<std::unique_ptr<MemoryBuffer>> BufOrErr =
        MemoryBuffer::getFile(Filename);
    if (!BufOrErr) {
      errs() << "WARNING: Internalize couldn't load file '" << Filename
             << "'! Continuing as if it's empty.\n";
      return; // Just continue as if the file were empty
    }
    Buf = std::move(*BufOrErr);
    for (line_iterator I(*Buf, true), E; I != E; ++I)
      addGlob(*I);
  }

  std::shared_ptr<MemoryBuffer> Buf;
};
} // end anonymous namespace

bool InternalizePass::shouldPreserveGV(const GlobalValue &GV) {
  // Function must be defined here
  if (GV.isDeclaration())
    return true;

  // Available externally is really just a "declaration with a body".
  if (GV.hasAvailableExternallyLinkage())
    return true;

  // Assume that dllexported symbols are referenced elsewhere
  if (GV.hasDLLExportStorageClass())
    return true;

  // As the name suggests, externally initialized variables need preserving as
  // they would be initialized elsewhere externally.
  if (const auto *G = dyn_cast<GlobalVariable>(&GV))
    if (G->isExternallyInitialized())
      return true;

  // Already local, has nothing to do.
  if (GV.hasLocalLinkage())
    return false;

  // Check some special cases
  if (AlwaysPreserved.count(GV.getName()))
    return true;

  return MustPreserveGV(GV);
}

bool InternalizePass::maybeInternalize(
    GlobalValue &GV, DenseMap<const Comdat *, ComdatInfo> &ComdatMap) {
  if (Comdat *C = GV.getComdat()) {
    // For GlobalAlias, C is the aliasee object's comdat which may have been
    // redirected. So ComdatMap may not contain C.
    if (ComdatMap.lookup(C).External)
      return false;

    if (auto *GO = dyn_cast<GlobalObject>(&GV)) {
      // If a comdat with one member is not externally visible, we can drop it.
      // Otherwise, the comdat can be used to establish dependencies among the
      // group of sections. Thus we have to keep the comdat but switch it to
      // nodeduplicate.
      // Note: nodeduplicate is not necessary for COFF. wasm doesn't support
      // nodeduplicate.
      ComdatInfo &Info = ComdatMap.find(C)->second;
      if (Info.Size == 1)
        GO->setComdat(nullptr);
      else if (!IsWasm)
        C->setSelectionKind(Comdat::NoDeduplicate);
    }

    if (GV.hasLocalLinkage())
      return false;
  } else {
    if (GV.hasLocalLinkage())
      return false;

    if (shouldPreserveGV(GV))
      return false;
  }

  GV.setVisibility(GlobalValue::DefaultVisibility);
  GV.setLinkage(GlobalValue::InternalLinkage);
  return true;
}

// If GV is part of a comdat and is externally visible, update the comdat size
// and keep track of its comdat so that we don't internalize any of its members.
void InternalizePass::checkComdat(
    GlobalValue &GV, DenseMap<const Comdat *, ComdatInfo> &ComdatMap) {
  Comdat *C = GV.getComdat();
  if (!C)
    return;

  ComdatInfo &Info = ComdatMap[C];
  ++Info.Size;
  if (shouldPreserveGV(GV))
    Info.External = true;
}

bool InternalizePass::internalizeModule(Module &M) {
  bool Changed = false;

  SmallVector<GlobalValue *, 4> Used;
  collectUsedGlobalVariables(M, Used, false);

  // Collect comdat size and visiblity information for the module.
  DenseMap<const Comdat *, ComdatInfo> ComdatMap;
  if (!M.getComdatSymbolTable().empty()) {
    for (Function &F : M)
      checkComdat(F, ComdatMap);
    for (GlobalVariable &GV : M.globals())
      checkComdat(GV, ComdatMap);
    for (GlobalAlias &GA : M.aliases())
      checkComdat(GA, ComdatMap);
  }

  // We must assume that globals in toolchain.used have a reference that not even
  // the linker can see, so we don't internalize them.
  // For toolchain.compiler.used the situation is a bit fuzzy. The assembler and
  // linker can drop those symbols. If this pass is running as part of LTO,
  // one might think that it could just drop toolchain.compiler.used. The problem
  // is that even in LTO toolchain doesn't see every reference. For example,
  // we don't see references from function local inline assembly. To be
  // conservative, we internalize symbols in toolchain.compiler.used, but we
  // keep toolchain.compiler.used so that the symbol is not deleted by toolchain.
  for (GlobalValue *V : Used) {
    AlwaysPreserved.insert(V->getName());
  }

  // Never internalize the toolchain.used symbol.  It is used to implement
  // attribute((used)).
  // FIXME: Shouldn't this just filter on toolchain.metadata section??
  AlwaysPreserved.insert("toolchain.used");
  AlwaysPreserved.insert("toolchain.compiler.used");

  // Never internalize anchors used by the machine module info, else the info
  // won't find them.  (see MachineModuleInfo.)
  AlwaysPreserved.insert("toolchain.global_ctors");
  AlwaysPreserved.insert("toolchain.global_dtors");
  AlwaysPreserved.insert("toolchain.global.annotations");

  // Never internalize symbols code-gen inserts.
  // FIXME: We should probably add this (and the __stack_chk_guard) via some
  // type of call-back in CodeGen.
  AlwaysPreserved.insert("__stack_chk_fail");
  if (M.getTargetTriple().isOSAIX())
    AlwaysPreserved.insert("__ssp_canary_word");
  else
    AlwaysPreserved.insert("__stack_chk_guard");

  // Preserve the RPC interface for GPU host callbacks when internalizing.
  if (M.getTargetTriple().isNVPTX())
    AlwaysPreserved.insert("__llvm_rpc_client");

  // Mark all functions not in the api as internal.
  IsWasm = M.getTargetTriple().isOSBinFormatWasm();
  for (Function &I : M) {
    if (!maybeInternalize(I, ComdatMap))
      continue;
    Changed = true;

    ++NumFunctions;
    LLVM_DEBUG(dbgs() << "Internalizing func " << I.getName() << "\n");
  }

  // Mark all global variables with initializers that are not in the api as
  // internal as well.
  for (auto &GV : M.globals()) {
    if (!maybeInternalize(GV, ComdatMap))
      continue;
    Changed = true;

    ++NumGlobals;
    LLVM_DEBUG(dbgs() << "Internalized gvar " << GV.getName() << "\n");
  }

  // Mark all aliases that are not in the api as internal as well.
  for (auto &GA : M.aliases()) {
    if (!maybeInternalize(GA, ComdatMap))
      continue;
    Changed = true;

    ++NumAliases;
    LLVM_DEBUG(dbgs() << "Internalized alias " << GA.getName() << "\n");
  }

  return Changed;
}

InternalizePass::InternalizePass() : MustPreserveGV(PreserveAPIList()) {}

PreservedAnalyses InternalizePass::run(Module &M, ModuleAnalysisManager &AM) {
  if (!internalizeModule(M))
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}
