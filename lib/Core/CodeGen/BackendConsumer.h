//===--- BackendConsumer.h - LLVM BackendConsumer Header File -------------===//
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

#ifndef LANGUAGE_CORE_LIB_CODEGEN_BACKENDCONSUMER_H
#define LANGUAGE_CORE_LIB_CODEGEN_BACKENDCONSUMER_H

#include "language/Core/CodeGen/BackendUtil.h"
#include "language/Core/CodeGen/CodeGenAction.h"

#include "toolchain/IR/DiagnosticInfo.h"
#include "toolchain/Support/Timer.h"

namespace toolchain {
  class DiagnosticInfoDontCall;
}

namespace language::Core {
class ASTContext;
class CodeGenAction;
class CoverageSourceInfo;

class BackendConsumer : public ASTConsumer {
  using LinkModule = CodeGenAction::LinkModule;

  virtual void anchor();
  CompilerInstance &CI;
  DiagnosticsEngine &Diags;
  const CodeGenOptions &CodeGenOpts;
  const TargetOptions &TargetOpts;
  const LangOptions &LangOpts;
  std::unique_ptr<raw_pwrite_stream> AsmOutStream;
  ASTContext *Context = nullptr;
  IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS;

  toolchain::Timer LLVMIRGeneration;
  unsigned LLVMIRGenerationRefCount = 0;

  /// True if we've finished generating IR. This prevents us from generating
  /// additional LLVM IR after emitting output in HandleTranslationUnit. This
  /// can happen when Clang plugins trigger additional AST deserialization.
  bool IRGenFinished = false;

  bool TimerIsEnabled = false;

  BackendAction Action;

  std::unique_ptr<CodeGenerator> Gen;

  SmallVector<LinkModule, 4> LinkModules;

  // A map from mangled names to their function's source location, used for
  // backend diagnostics as the Clang AST may be unavailable. We actually use
  // the mangled name's hash as the key because mangled names can be very
  // long and take up lots of space. Using a hash can cause name collision,
  // but that is rare and the consequences are pointing to a wrong source
  // location which is not severe. This is a vector instead of an actual map
  // because we optimize for time building this map rather than time
  // retrieving an entry, as backend diagnostics are uncommon.
  std::vector<std::pair<toolchain::hash_code, FullSourceLoc>>
    ManglingFullSourceLocs;


  // This is here so that the diagnostic printer knows the module a diagnostic
  // refers to.
  toolchain::Module *CurLinkModule = nullptr;

public:
  BackendConsumer(CompilerInstance &CI, BackendAction Action,
                  IntrusiveRefCntPtr<toolchain::vfs::FileSystem> VFS,
                  toolchain::LLVMContext &C, SmallVector<LinkModule, 4> LinkModules,
                  StringRef InFile, std::unique_ptr<raw_pwrite_stream> OS,
                  CoverageSourceInfo *CoverageInfo,
                  toolchain::Module *CurLinkModule = nullptr);

  toolchain::Module *getModule() const;
  std::unique_ptr<toolchain::Module> takeModule();

  CodeGenerator *getCodeGenerator();

  void HandleCXXStaticMemberVarInstantiation(VarDecl *VD) override;
  void Initialize(ASTContext &Ctx) override;
  bool HandleTopLevelDecl(DeclGroupRef D) override;
  void HandleInlineFunctionDefinition(FunctionDecl *D) override;
  void HandleInterestingDecl(DeclGroupRef D) override;
  void HandleTranslationUnit(ASTContext &C) override;
  void HandleTagDeclDefinition(TagDecl *D) override;
  void HandleTagDeclRequiredDefinition(const TagDecl *D) override;
  void CompleteTentativeDefinition(VarDecl *D) override;
  void CompleteExternalDeclaration(DeclaratorDecl *D) override;
  void AssignInheritanceModel(CXXRecordDecl *RD) override;
  void HandleVTable(CXXRecordDecl *RD) override;

  // Links each entry in LinkModules into our module.  Returns true on error.
  bool LinkInModules(toolchain::Module *M);

  /// Get the best possible source location to represent a diagnostic that
  /// may have associated debug info.
  const FullSourceLoc getBestLocationFromDebugLoc(
    const toolchain::DiagnosticInfoWithLocationBase &D,
    bool &BadDebugInfo, StringRef &Filename,
    unsigned &Line, unsigned &Column) const;

  std::optional<FullSourceLoc> getFunctionSourceLocation(
    const toolchain::Function &F) const;

  void DiagnosticHandlerImpl(const toolchain::DiagnosticInfo &DI);
  /// Specialized handler for InlineAsm diagnostic.
  /// \return True if the diagnostic has been successfully reported, false
  /// otherwise.
  bool InlineAsmDiagHandler(const toolchain::DiagnosticInfoInlineAsm &D);
  /// Specialized handler for diagnostics reported using SMDiagnostic.
  void SrcMgrDiagHandler(const toolchain::DiagnosticInfoSrcMgr &D);
  /// Specialized handler for StackSize diagnostic.
  /// \return True if the diagnostic has been successfully reported, false
  /// otherwise.
  bool StackSizeDiagHandler(const toolchain::DiagnosticInfoStackSize &D);
  /// Specialized handler for ResourceLimit diagnostic.
  /// \return True if the diagnostic has been successfully reported, false
  /// otherwise.
  bool ResourceLimitDiagHandler(const toolchain::DiagnosticInfoResourceLimit &D);

  /// Specialized handler for unsupported backend feature diagnostic.
  void UnsupportedDiagHandler(const toolchain::DiagnosticInfoUnsupported &D);
  /// Specialized handlers for optimization remarks.
  /// Note that these handlers only accept remarks and they always handle
  /// them.
  void EmitOptimizationMessage(const toolchain::DiagnosticInfoOptimizationBase &D,
                               unsigned DiagID);
  void
    OptimizationRemarkHandler(const toolchain::DiagnosticInfoOptimizationBase &D);
  void OptimizationRemarkHandler(
    const toolchain::OptimizationRemarkAnalysisFPCommute &D);
  void OptimizationRemarkHandler(
    const toolchain::OptimizationRemarkAnalysisAliasing &D);
  void OptimizationFailureHandler(
    const toolchain::DiagnosticInfoOptimizationFailure &D);
  void DontCallDiagHandler(const toolchain::DiagnosticInfoDontCall &D);
  /// Specialized handler for misexpect warnings.
  /// Note that misexpect remarks are emitted through ORE
  void MisExpectDiagHandler(const toolchain::DiagnosticInfoMisExpect &D);
};

} // namespace language::Core
#endif
