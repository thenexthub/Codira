//===---- CIRGenAction.h - CIR Code Generation Frontend Action -*- C++ -*--===//
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

#ifndef LANGUAGE_CORE_CIR_CIRGENACTION_H
#define LANGUAGE_CORE_CIR_CIRGENACTION_H

#include "language/Core/Frontend/FrontendAction.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/OwningOpRef.h"

namespace mlir {
class MLIRContext;
class ModuleOp;
} // namespace mlir

namespace cir {
class CIRGenConsumer;

class CIRGenAction : public language::Core::ASTFrontendAction {
public:
  enum class OutputType {
    EmitAssembly,
    EmitCIR,
    EmitLLVM,
    EmitBC,
    EmitObj,
  };

private:
  friend class CIRGenConsumer;

  mlir::OwningOpRef<mlir::ModuleOp> MLIRMod;

  mlir::MLIRContext *MLIRCtx;

protected:
  CIRGenAction(OutputType Action, mlir::MLIRContext *MLIRCtx = nullptr);

  std::unique_ptr<language::Core::ASTConsumer>
  CreateASTConsumer(language::Core::CompilerInstance &CI,
                    toolchain::StringRef InFile) override;

public:
  ~CIRGenAction() override;

  OutputType Action;
};

class EmitCIRAction : public CIRGenAction {
  virtual void anchor();

public:
  EmitCIRAction(mlir::MLIRContext *MLIRCtx = nullptr);
};

class EmitLLVMAction : public CIRGenAction {
  virtual void anchor();

public:
  EmitLLVMAction(mlir::MLIRContext *MLIRCtx = nullptr);
};

class EmitBCAction : public CIRGenAction {
  virtual void anchor();

public:
  EmitBCAction(mlir::MLIRContext *MLIRCtx = nullptr);
};

class EmitAssemblyAction : public CIRGenAction {
  virtual void anchor();

public:
  EmitAssemblyAction(mlir::MLIRContext *MLIRCtx = nullptr);
};

class EmitObjAction : public CIRGenAction {
  virtual void anchor();

public:
  EmitObjAction(mlir::MLIRContext *MLIRCtx = nullptr);
};

} // namespace cir

#endif
