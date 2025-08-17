//===--- CodeGenAction.h - LLVM Code Generation Frontend Action -*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_CODEGEN_CODEGENACTION_H
#define LANGUAGE_CORE_CODEGEN_CODEGENACTION_H

#include "language/Core/Frontend/FrontendAction.h"
#include <memory>

namespace toolchain {
  class LLVMContext;
  class Module;
}

namespace language::Core {
class BackendConsumer;
class CodeGenerator;

class CodeGenAction : public ASTFrontendAction {
private:
  // Let BackendConsumer access LinkModule.
  friend class BackendConsumer;

  /// Info about module to link into a module we're generating.
  struct LinkModule {
    /// The module to link in.
    std::unique_ptr<toolchain::Module> Module;

    /// If true, we set attributes on Module's functions according to our
    /// CodeGenOptions and LangOptions, as though we were generating the
    /// function ourselves.
    bool PropagateAttrs;

    /// If true, we use LLVM module internalizer.
    bool Internalize;

    /// Bitwise combination of toolchain::LinkerFlags used when we link the module.
    unsigned LinkFlags;
  };

  unsigned Act;
  std::unique_ptr<toolchain::Module> TheModule;

  /// Bitcode modules to link in to our module.
  SmallVector<LinkModule, 4> LinkModules;
  toolchain::LLVMContext *VMContext;
  bool OwnsVMContext;

  std::unique_ptr<toolchain::Module> loadModule(toolchain::MemoryBufferRef MBRef);

  /// Load bitcode modules to link into our module from the options.
  bool loadLinkModules(CompilerInstance &CI);

protected:
  bool BeginSourceFileAction(CompilerInstance &CI) override;

  /// Create a new code generation action.  If the optional \p _VMContext
  /// parameter is supplied, the action uses it without taking ownership,
  /// otherwise it creates a fresh LLVM context and takes ownership.
  CodeGenAction(unsigned _Act, toolchain::LLVMContext *_VMContext = nullptr);

  bool hasIRSupport() const override;

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;

  void ExecuteAction() override;

  void EndSourceFileAction() override;

public:
  ~CodeGenAction() override;

  /// Take the generated LLVM module, for use after the action has been run.
  /// The result may be null on failure.
  std::unique_ptr<toolchain::Module> takeModule();

  /// Take the LLVM context used by this action.
  toolchain::LLVMContext *takeLLVMContext();

  CodeGenerator *getCodeGenerator() const;

  BackendConsumer *BEConsumer = nullptr;
};

class EmitAssemblyAction : public CodeGenAction {
  virtual void anchor();
public:
  EmitAssemblyAction(toolchain::LLVMContext *_VMContext = nullptr);
};

class EmitBCAction : public CodeGenAction {
  virtual void anchor();
public:
  EmitBCAction(toolchain::LLVMContext *_VMContext = nullptr);
};

class EmitLLVMAction : public CodeGenAction {
  virtual void anchor();
public:
  EmitLLVMAction(toolchain::LLVMContext *_VMContext = nullptr);
};

class EmitLLVMOnlyAction : public CodeGenAction {
  virtual void anchor();
public:
  EmitLLVMOnlyAction(toolchain::LLVMContext *_VMContext = nullptr);
};

class EmitCodeGenOnlyAction : public CodeGenAction {
  virtual void anchor();
public:
  EmitCodeGenOnlyAction(toolchain::LLVMContext *_VMContext = nullptr);
};

class EmitObjAction : public CodeGenAction {
  virtual void anchor();
public:
  EmitObjAction(toolchain::LLVMContext *_VMContext = nullptr);
};

}

#endif
