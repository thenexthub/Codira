//===--- ModuleBuilder.cpp - Emit LLVM Code from ASTs ---------------------===//
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
// This builds an AST and converts it to LLVM Code.
//
//===----------------------------------------------------------------------===//

#include "language/Core/CodeGen/ModuleBuilder.h"
#include "CGDebugInfo.h"
#include "CodeGenModule.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/Basic/CodeGenOptions.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/TargetInfo.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/IR/DataLayout.h"
#include "toolchain/IR/LLVMContext.h"
#include "toolchain/IR/Module.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include <memory>

using namespace language::Core;
using namespace CodeGen;

namespace {
  class CodeGeneratorImpl : public CodeGenerator {
    DiagnosticsEngine &Diags;
    ASTContext *Ctx;
    IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS; // Only used for debug info.
    const HeaderSearchOptions &HeaderSearchOpts; // Only used for debug info.
    const PreprocessorOptions &PreprocessorOpts; // Only used for debug info.
    const CodeGenOptions &CodeGenOpts;

    unsigned HandlingTopLevelDecls;

    /// Use this when emitting decls to block re-entrant decl emission. It will
    /// emit all deferred decls on scope exit. Set EmitDeferred to false if decl
    /// emission must be deferred longer, like at the end of a tag definition.
    struct HandlingTopLevelDeclRAII {
      CodeGeneratorImpl &Self;
      bool EmitDeferred;
      HandlingTopLevelDeclRAII(CodeGeneratorImpl &Self,
                               bool EmitDeferred = true)
          : Self(Self), EmitDeferred(EmitDeferred) {
        ++Self.HandlingTopLevelDecls;
      }
      ~HandlingTopLevelDeclRAII() {
        unsigned Level = --Self.HandlingTopLevelDecls;
        if (Level == 0 && EmitDeferred)
          Self.EmitDeferredDecls();
      }
    };

    CoverageSourceInfo *CoverageInfo;

  protected:
    std::unique_ptr<toolchain::Module> M;
    std::unique_ptr<CodeGen::CodeGenModule> Builder;

  private:
    SmallVector<FunctionDecl *, 8> DeferredInlineMemberFuncDefs;

    static toolchain::StringRef ExpandModuleName(toolchain::StringRef ModuleName,
                                            const CodeGenOptions &CGO) {
      if (ModuleName == "-" && !CGO.MainFileName.empty())
        return CGO.MainFileName;
      return ModuleName;
    }

  public:
    CodeGeneratorImpl(DiagnosticsEngine &diags, toolchain::StringRef ModuleName,
                      IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS,
                      const HeaderSearchOptions &HSO,
                      const PreprocessorOptions &PPO, const CodeGenOptions &CGO,
                      toolchain::LLVMContext &C,
                      CoverageSourceInfo *CoverageInfo = nullptr)
        : Diags(diags), Ctx(nullptr), FS(std::move(FS)), HeaderSearchOpts(HSO),
          PreprocessorOpts(PPO), CodeGenOpts(CGO), HandlingTopLevelDecls(0),
          CoverageInfo(CoverageInfo),
          M(new toolchain::Module(ExpandModuleName(ModuleName, CGO), C)) {
      C.setDiscardValueNames(CGO.DiscardValueNames);
    }

    ~CodeGeneratorImpl() override {
      // There should normally not be any leftover inline method definitions.
      assert(DeferredInlineMemberFuncDefs.empty() ||
             Diags.hasErrorOccurred());
    }

    CodeGenModule &CGM() {
      return *Builder;
    }

    toolchain::Module *GetModule() {
      return M.get();
    }

    CGDebugInfo *getCGDebugInfo() {
      return Builder->getModuleDebugInfo();
    }

    toolchain::Module *ReleaseModule() {
      return M.release();
    }

    const Decl *GetDeclForMangledName(StringRef MangledName) {
      GlobalDecl Result;
      if (!Builder->lookupRepresentativeDecl(MangledName, Result))
        return nullptr;
      const Decl *D = Result.getCanonicalDecl().getDecl();
      if (auto FD = dyn_cast<FunctionDecl>(D)) {
        if (FD->hasBody(FD))
          return FD;
      } else if (auto TD = dyn_cast<TagDecl>(D)) {
        if (auto Def = TD->getDefinition())
          return Def;
      }
      return D;
    }

    toolchain::StringRef GetMangledName(GlobalDecl GD) {
      return Builder->getMangledName(GD);
    }

    toolchain::Constant *GetAddrOfGlobal(GlobalDecl global, bool isForDefinition) {
      return Builder->GetAddrOfGlobal(global, ForDefinition_t(isForDefinition));
    }

    toolchain::Module *StartModule(toolchain::StringRef ModuleName,
                              toolchain::LLVMContext &C) {
      assert(!M && "Replacing existing Module?");
      M.reset(new toolchain::Module(ExpandModuleName(ModuleName, CodeGenOpts), C));

      std::unique_ptr<CodeGenModule> OldBuilder = std::move(Builder);

      Initialize(*Ctx);

      if (OldBuilder)
        OldBuilder->moveLazyEmissionStates(Builder.get());

      return M.get();
    }

    void Initialize(ASTContext &Context) override {
      Ctx = &Context;

      M->setTargetTriple(Ctx->getTargetInfo().getTriple());
      M->setDataLayout(Ctx->getTargetInfo().getDataLayoutString());
      const auto &SDKVersion = Ctx->getTargetInfo().getSDKVersion();
      if (!SDKVersion.empty())
        M->setSDKVersion(SDKVersion);
      if (const auto *TVT = Ctx->getTargetInfo().getDarwinTargetVariantTriple())
        M->setDarwinTargetVariantTriple(TVT->getTriple());
      if (auto TVSDKVersion =
              Ctx->getTargetInfo().getDarwinTargetVariantSDKVersion())
        M->setDarwinTargetVariantSDKVersion(*TVSDKVersion);
      Builder.reset(new CodeGen::CodeGenModule(Context, FS, HeaderSearchOpts,
                                               PreprocessorOpts, CodeGenOpts,
                                               *M, Diags, CoverageInfo));

      for (auto &&Lib : CodeGenOpts.DependentLibraries)
        Builder->AddDependentLib(Lib);
      for (auto &&Opt : CodeGenOpts.LinkerOptions)
        Builder->AppendLinkerOptions(Opt);
    }

    void HandleCXXStaticMemberVarInstantiation(VarDecl *VD) override {
      if (Diags.hasErrorOccurred())
        return;

      Builder->HandleCXXStaticMemberVarInstantiation(VD);
    }

    bool HandleTopLevelDecl(DeclGroupRef DG) override {
      // FIXME: Why not return false and abort parsing?
      if (Diags.hasUnrecoverableErrorOccurred())
        return true;

      HandlingTopLevelDeclRAII HandlingDecl(*this);

      // Make sure to emit all elements of a Decl.
      for (auto &I : DG)
        Builder->EmitTopLevelDecl(I);

      return true;
    }

    void EmitDeferredDecls() {
      if (DeferredInlineMemberFuncDefs.empty())
        return;

      // Emit any deferred inline method definitions. Note that more deferred
      // methods may be added during this loop, since ASTConsumer callbacks
      // can be invoked if AST inspection results in declarations being added.
      HandlingTopLevelDeclRAII HandlingDecl(*this);
      for (unsigned I = 0; I != DeferredInlineMemberFuncDefs.size(); ++I)
        Builder->EmitTopLevelDecl(DeferredInlineMemberFuncDefs[I]);
      DeferredInlineMemberFuncDefs.clear();
    }

    void HandleInlineFunctionDefinition(FunctionDecl *D) override {
      if (Diags.hasUnrecoverableErrorOccurred())
        return;

      assert(D->doesThisDeclarationHaveABody());

      // We may want to emit this definition. However, that decision might be
      // based on computing the linkage, and we have to defer that in case we
      // are inside of something that will change the method's final linkage,
      // e.g.
      //   typedef struct {
      //     void bar();
      //     void foo() { bar(); }
      //   } A;
      DeferredInlineMemberFuncDefs.push_back(D);

      // Provide some coverage mapping even for methods that aren't emitted.
      // Don't do this for templated classes though, as they may not be
      // instantiable.
      if (!D->getLexicalDeclContext()->isDependentContext())
        Builder->AddDeferredUnusedCoverageMapping(D);
    }

    /// HandleTagDeclDefinition - This callback is invoked each time a TagDecl
    /// to (e.g. struct, union, enum, class) is completed. This allows the
    /// client hack on the type, which can occur at any point in the file
    /// (because these can be defined in declspecs).
    void HandleTagDeclDefinition(TagDecl *D) override {
      if (Diags.hasUnrecoverableErrorOccurred())
        return;

      // Don't allow re-entrant calls to CodeGen triggered by PCH
      // deserialization to emit deferred decls.
      HandlingTopLevelDeclRAII HandlingDecl(*this, /*EmitDeferred=*/false);

      Builder->UpdateCompletedType(D);

      // For MSVC compatibility, treat declarations of static data members with
      // inline initializers as definitions.
      if (Ctx->getTargetInfo().getCXXABI().isMicrosoft()) {
        for (Decl *Member : D->decls()) {
          if (VarDecl *VD = dyn_cast<VarDecl>(Member)) {
            if (Ctx->isMSStaticDataMemberInlineDefinition(VD) &&
                Ctx->DeclMustBeEmitted(VD)) {
              Builder->EmitGlobal(VD);
            }
          }
        }
      }
      // For OpenMP emit declare reduction functions, if required.
      if (Ctx->getLangOpts().OpenMP) {
        for (Decl *Member : D->decls()) {
          if (auto *DRD = dyn_cast<OMPDeclareReductionDecl>(Member)) {
            if (Ctx->DeclMustBeEmitted(DRD))
              Builder->EmitGlobal(DRD);
          } else if (auto *DMD = dyn_cast<OMPDeclareMapperDecl>(Member)) {
            if (Ctx->DeclMustBeEmitted(DMD))
              Builder->EmitGlobal(DMD);
          }
        }
      }
    }

    void HandleTagDeclRequiredDefinition(const TagDecl *D) override {
      if (Diags.hasUnrecoverableErrorOccurred())
        return;

      // Don't allow re-entrant calls to CodeGen triggered by PCH
      // deserialization to emit deferred decls.
      HandlingTopLevelDeclRAII HandlingDecl(*this, /*EmitDeferred=*/false);

      if (CodeGen::CGDebugInfo *DI = Builder->getModuleDebugInfo())
        if (const RecordDecl *RD = dyn_cast<RecordDecl>(D))
          DI->completeRequiredType(RD);
    }

    void HandleTranslationUnit(ASTContext &Ctx) override {
      // Release the Builder when there is no error.
      if (!Diags.hasUnrecoverableErrorOccurred() && Builder)
        Builder->Release();

      // If there are errors before or when releasing the Builder, reset
      // the module to stop here before invoking the backend.
      if (Diags.hasErrorOccurred()) {
        if (Builder)
          Builder->clear();
        M.reset();
        return;
      }
    }

    void AssignInheritanceModel(CXXRecordDecl *RD) override {
      if (Diags.hasUnrecoverableErrorOccurred())
        return;

      Builder->RefreshTypeCacheForClass(RD);
    }

    void CompleteTentativeDefinition(VarDecl *D) override {
      if (Diags.hasUnrecoverableErrorOccurred())
        return;

      Builder->EmitTentativeDefinition(D);
    }

    void CompleteExternalDeclaration(DeclaratorDecl *D) override {
      Builder->EmitExternalDeclaration(D);
    }

    void HandleVTable(CXXRecordDecl *RD) override {
      if (Diags.hasUnrecoverableErrorOccurred())
        return;

      Builder->EmitVTable(RD);
    }
  };
}

void CodeGenerator::anchor() { }

CodeGenModule &CodeGenerator::CGM() {
  return static_cast<CodeGeneratorImpl*>(this)->CGM();
}

toolchain::Module *CodeGenerator::GetModule() {
  return static_cast<CodeGeneratorImpl*>(this)->GetModule();
}

toolchain::Module *CodeGenerator::ReleaseModule() {
  return static_cast<CodeGeneratorImpl*>(this)->ReleaseModule();
}

CGDebugInfo *CodeGenerator::getCGDebugInfo() {
  return static_cast<CodeGeneratorImpl*>(this)->getCGDebugInfo();
}

const Decl *CodeGenerator::GetDeclForMangledName(toolchain::StringRef name) {
  return static_cast<CodeGeneratorImpl*>(this)->GetDeclForMangledName(name);
}

toolchain::StringRef CodeGenerator::GetMangledName(GlobalDecl GD) {
  return static_cast<CodeGeneratorImpl *>(this)->GetMangledName(GD);
}

toolchain::Constant *CodeGenerator::GetAddrOfGlobal(GlobalDecl global,
                                               bool isForDefinition) {
  return static_cast<CodeGeneratorImpl*>(this)
           ->GetAddrOfGlobal(global, isForDefinition);
}

toolchain::Module *CodeGenerator::StartModule(toolchain::StringRef ModuleName,
                                         toolchain::LLVMContext &C) {
  return static_cast<CodeGeneratorImpl*>(this)->StartModule(ModuleName, C);
}

CodeGenerator *
language::Core::CreateLLVMCodeGen(DiagnosticsEngine &Diags, toolchain::StringRef ModuleName,
                         IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS,
                         const HeaderSearchOptions &HeaderSearchOpts,
                         const PreprocessorOptions &PreprocessorOpts,
                         const CodeGenOptions &CGO, toolchain::LLVMContext &C,
                         CoverageSourceInfo *CoverageInfo) {
  return new CodeGeneratorImpl(Diags, ModuleName, std::move(FS),
                               HeaderSearchOpts, PreprocessorOpts, CGO, C,
                               CoverageInfo);
}
