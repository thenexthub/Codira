//===- CIRGenerator.h - CIR Generation from Clang AST ---------------------===//
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
// This file declares a simple interface to perform CIR generation from Clang
// AST
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_CIR_CIRGENERATOR_H
#define LANGUAGE_CORE_CIR_CIRGENERATOR_H

#include "language/Core/AST/ASTConsumer.h"
#include "language/Core/Basic/CodeGenOptions.h"

#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/Support/VirtualFileSystem.h"

#include <memory>

namespace language::Core {
class DeclGroupRef;
class DiagnosticsEngine;
namespace CIRGen {
class CIRGenModule;
} // namespace CIRGen
} // namespace language::Core

namespace mlir {
class MLIRContext;
} // namespace mlir
namespace cir {
class CIRGenerator : public language::Core::ASTConsumer {
  virtual void anchor();
  language::Core::DiagnosticsEngine &diags;
  language::Core::ASTContext *astContext;
  // Only used for debug info.
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> fs;

  const language::Core::CodeGenOptions &codeGenOpts;

  unsigned handlingTopLevelDecls;

  /// Use this when emitting decls to block re-entrant decl emission. It will
  /// emit all deferred decls on scope exit. Set EmitDeferred to false if decl
  /// emission must be deferred longer, like at the end of a tag definition.
  struct HandlingTopLevelDeclRAII {
    CIRGenerator &self;
    bool emitDeferred;
    HandlingTopLevelDeclRAII(CIRGenerator &self, bool emitDeferred = true)
        : self{self}, emitDeferred{emitDeferred} {
      ++self.handlingTopLevelDecls;
    }
    ~HandlingTopLevelDeclRAII() {
      unsigned Level = --self.handlingTopLevelDecls;
      if (Level == 0 && emitDeferred)
        self.emitDeferredDecls();
    }
  };

protected:
  std::unique_ptr<mlir::MLIRContext> mlirContext;
  std::unique_ptr<language::Core::CIRGen::CIRGenModule> cgm;

private:
  toolchain::SmallVector<language::Core::FunctionDecl *, 8> deferredInlineMemberFuncDefs;

public:
  CIRGenerator(language::Core::DiagnosticsEngine &diags,
               toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> fs,
               const language::Core::CodeGenOptions &cgo);
  ~CIRGenerator() override;
  void Initialize(language::Core::ASTContext &astContext) override;
  bool HandleTopLevelDecl(language::Core::DeclGroupRef group) override;
  void HandleTranslationUnit(language::Core::ASTContext &astContext) override;
  void HandleInlineFunctionDefinition(language::Core::FunctionDecl *d) override;
  void HandleTagDeclDefinition(language::Core::TagDecl *d) override;
  void HandleTagDeclRequiredDefinition(const language::Core::TagDecl *D) override;
  void HandleCXXStaticMemberVarInstantiation(language::Core::VarDecl *D) override;
  void CompleteTentativeDefinition(language::Core::VarDecl *d) override;
  void HandleVTable(language::Core::CXXRecordDecl *rd) override;

  mlir::ModuleOp getModule() const;
  mlir::MLIRContext &getMLIRContext() { return *mlirContext; };
  const mlir::MLIRContext &getMLIRContext() const { return *mlirContext; };

  bool verifyModule() const;

  void emitDeferredDecls();
};

} // namespace cir

#endif // LANGUAGE_CORE_CIR_CIRGENERATOR_H
