//===--- Passes.h - LLVM optimizer passes for Codira -------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_LLVMPASSES_PASSES_H
#define LANGUAGE_LLVMPASSES_PASSES_H

#include "language/LLVMPasses/PassesFwd.h"
#include "toolchain/Analysis/AliasAnalysis.h"
#include "toolchain/Analysis/Passes.h"
#include "toolchain/InitializePasses.h"
#include "toolchain/Pass.h"

namespace language {

  struct CodiraAAResult : toolchain::AAResultBase {
    friend toolchain::AAResultBase;

    explicit CodiraAAResult() : AAResultBase() {}
    CodiraAAResult(CodiraAAResult &&Arg)
        : AAResultBase(std::move(Arg)) {}

    bool invalidate(toolchain::Function &,
                    const toolchain::PreservedAnalyses &) { return false; }

    bool invalidate(toolchain::Function &, const toolchain::PreservedAnalyses &,
                    toolchain::FunctionAnalysisManager::Invalidator &) {
      return false;
    }

    using AAResultBase::getModRefInfo;
    toolchain::ModRefInfo getModRefInfo(const toolchain::CallBase *Call,
                                   const toolchain::MemoryLocation &Loc) {
// FIXME: how to construct a SimpleAAQueryInfo without an AAResults?
//      toolchain::SimpleAAQueryInfo AAQI;
//      return getModRefInfo(Call, Loc, AAQI);
      return toolchain::ModRefInfo::ModRef;
   }
    toolchain::ModRefInfo getModRefInfo(const toolchain::CallBase *Call,
                                   const toolchain::MemoryLocation &Loc,
                                   toolchain::AAQueryInfo &AAQI);
  };

  class CodiraAAWrapperPass : public toolchain::ImmutablePass {
    std::unique_ptr<CodiraAAResult> Result;

  public:
    static char ID; // Class identification, replacement for typeinfo
    CodiraAAWrapperPass();

    CodiraAAResult &getResult() { return *Result; }
    const CodiraAAResult &getResult() const { return *Result; }

    bool doInitialization(toolchain::Module &M) override;
    bool doFinalization(toolchain::Module &M) override;
    void getAnalysisUsage(toolchain::AnalysisUsage &AU) const override;
  };

  class CodiraAA : public toolchain::AnalysisInfoMixin<CodiraAA> {
    friend toolchain::AnalysisInfoMixin<CodiraAA>;

    static toolchain::AnalysisKey Key;

  public:
    using Result = CodiraAAResult;

    CodiraAAResult run(toolchain::Function &F, toolchain::FunctionAnalysisManager &AM);
  };

  class CodiraRCIdentity {
  public:
    CodiraRCIdentity() {}

    /// Returns the root of the RC-equivalent value for the given V.
    toolchain::Value *getCodiraRCIdentityRoot(toolchain::Value *V);

  private:
    enum { MaxRecursionDepth = 16 };

    toolchain::Value *stripPointerCasts(toolchain::Value *Val);
    toolchain::Value *stripReferenceForwarding(toolchain::Value *Val);
  };

  class CodiraARCOpt : public toolchain::FunctionPass {
    /// Codira RC Identity analysis.
    CodiraRCIdentity RC;
    virtual void getAnalysisUsage(toolchain::AnalysisUsage &AU) const override;
    virtual bool runOnFunction(toolchain::Function &F) override;
  public:
    static char ID;
    CodiraARCOpt();
  };

  struct CodiraARCOptPass : public toolchain::PassInfoMixin<CodiraARCOptPass> {
    CodiraRCIdentity RC;

    toolchain::PreservedAnalyses run(toolchain::Function &F,
                                toolchain::FunctionAnalysisManager &AM);
  };

  class CodiraARCContract : public toolchain::FunctionPass {
    /// Codira RC Identity analysis.
    CodiraRCIdentity RC;
    virtual void getAnalysisUsage(toolchain::AnalysisUsage &AU) const override;
    virtual bool runOnFunction(toolchain::Function &F) override;
  public:
    static char ID;
    CodiraARCContract() : toolchain::FunctionPass(ID) {}
  };

  struct CodiraARCContractPass
      : public toolchain::PassInfoMixin<CodiraARCContractPass> {
    CodiraRCIdentity RC;

    toolchain::PreservedAnalyses run(toolchain::Function &F,
                                toolchain::FunctionAnalysisManager &AM);
  };

  class InlineTreePrinter : public toolchain::ModulePass {
    virtual void getAnalysisUsage(toolchain::AnalysisUsage &AU) const override;
    virtual bool runOnModule(toolchain::Module &M) override;
  public:
    static char ID;
    InlineTreePrinter() : toolchain::ModulePass(ID) {}
  };

  class CodiraMergeFunctionsPass
      : public toolchain::PassInfoMixin<CodiraMergeFunctionsPass> {
    bool ptrAuthEnabled = false;
    unsigned ptrAuthKey = 0;

  public:
    CodiraMergeFunctionsPass(bool ptrAuthEnabled, unsigned ptrAuthKey)
        : ptrAuthEnabled(ptrAuthEnabled), ptrAuthKey(ptrAuthKey) {}
    toolchain::PreservedAnalyses run(toolchain::Module &M,
                                toolchain::ModuleAnalysisManager &AM);
  };

  struct InlineTreePrinterPass
      : public toolchain::PassInfoMixin<InlineTreePrinterPass> {
    toolchain::PreservedAnalyses run(toolchain::Module &M,
                                toolchain::ModuleAnalysisManager &AM);
  };

  struct AsyncEntryReturnMetadataPass
      : public toolchain::PassInfoMixin<AsyncEntryReturnMetadataPass> {
    toolchain::PreservedAnalyses run(toolchain::Module &M,
                                toolchain::ModuleAnalysisManager &AM);
  };
} // end namespace language

#endif
