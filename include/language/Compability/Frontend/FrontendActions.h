//===- FrontendActions.h -----------------------------------------*- C++-*-===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_FRONTEND_FRONTENDACTIONS_H
#define LANGUAGE_COMPABILITY_FRONTEND_FRONTENDACTIONS_H

#include "language/Compability/Frontend/FrontendAction.h"
#include "language/Compability/Frontend/ParserActions.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/OwningOpRef.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/IR/Module.h"
#include <memory>

namespace language::Compability::frontend {

//===----------------------------------------------------------------------===//
// Custom Consumer Actions
//===----------------------------------------------------------------------===//

class InputOutputTestAction : public FrontendAction {
  void executeAction() override;
};

class InitOnlyAction : public FrontendAction {
  void executeAction() override;
};

//===----------------------------------------------------------------------===//
// Prescan Actions
//===----------------------------------------------------------------------===//
class PrescanAction : public FrontendAction {
  void executeAction() override = 0;
  bool beginSourceFileAction() override;
};

class PrintPreprocessedAction : public PrescanAction {
  void executeAction() override;
};

class DebugDumpProvenanceAction : public PrescanAction {
  void executeAction() override;
};

class DebugDumpParsingLogAction : public PrescanAction {
  void executeAction() override;
};

class DebugMeasureParseTreeAction : public PrescanAction {
  void executeAction() override;
};

//===----------------------------------------------------------------------===//
// PrescanAndParse Actions
//===----------------------------------------------------------------------===//
class PrescanAndParseAction : public FrontendAction {
  void executeAction() override = 0;
  bool beginSourceFileAction() override;
};

class DebugUnparseNoSemaAction : public PrescanAndParseAction {
  void executeAction() override;
};

class DebugDumpParseTreeNoSemaAction : public PrescanAndParseAction {
  void executeAction() override;
};

//===----------------------------------------------------------------------===//
// PrescanAndSema Actions
//
// These actions will parse the input, run the semantic checks and execute
// their actions provided that no parsing or semantic errors were found.
//===----------------------------------------------------------------------===//
class PrescanAndSemaAction : public FrontendAction {

  void executeAction() override = 0;
  bool beginSourceFileAction() override;
};

class DebugUnparseWithSymbolsAction : public PrescanAndSemaAction {
  void executeAction() override;
};

class DebugUnparseWithModulesAction : public PrescanAndSemaAction {
  void executeAction() override;
};

class DebugUnparseAction : public PrescanAndSemaAction {
  void executeAction() override;
};

class DebugDumpSymbolsAction : public PrescanAndSemaAction {
  void executeAction() override;
};

class DebugDumpParseTreeAction : public PrescanAndSemaAction {
  void executeAction() override;
};

class DebugDumpPFTAction : public PrescanAndSemaAction {
  void executeAction() override;
};

class DebugPreFIRTreeAction : public PrescanAndSemaAction {
  void executeAction() override;
};

class GetDefinitionAction : public PrescanAndSemaAction {
  void executeAction() override;
};

class GetSymbolsSourcesAction : public PrescanAndSemaAction {
  void executeAction() override;
};

class ParseSyntaxOnlyAction : public PrescanAndSemaAction {
  void executeAction() override;
};

class PluginParseTreeAction : public PrescanAndSemaAction {
  void executeAction() override = 0;

public:
  language::Compability::parser::Parsing &getParsing();
  /// Creates an output file. This is just a wrapper for calling
  /// CreateDefaultOutputFile from CompilerInstance. Use it to make sure that
  /// your plugin respects driver's `-o` flag.
  /// \param extension  The extension to use for the output file (ignored when
  ///                   the user decides to print to stdout via `-o -`)
  /// \return           Null on error, ostream for the output file otherwise
  std::unique_ptr<toolchain::raw_pwrite_stream>
  createOutputFile(toolchain::StringRef extension);
};

//===----------------------------------------------------------------------===//
// PrescanAndSemaDebug Actions
//
// These actions will parse the input, run the semantic checks and execute
// their actions _regardless of_ whether any semantic errors have been found.
// This can be useful when adding new languge feature and when you wish to
// investigate compiler output (e.g. the parse tree) despite any semantic
// errors.
//
// NOTE: Use with care and for development only!
//===----------------------------------------------------------------------===//
class PrescanAndSemaDebugAction : public FrontendAction {

  void executeAction() override = 0;
  bool beginSourceFileAction() override;
};

class DebugDumpAllAction : public PrescanAndSemaDebugAction {
  void executeAction() override;
};

//===----------------------------------------------------------------------===//
// CodeGen Actions
//===----------------------------------------------------------------------===//
/// Represents the type of "backend" action to perform by the corresponding
/// CodeGenAction. Note that from Flang's perspective, both LLVM and MLIR are
/// "backends" that are used for generating LLVM IR/BC, assembly files or
/// machine code. This enum captures "what" exactly one of these backends is to
/// do. The names are similar to what is used in Clang - this allows us to
/// maintain some level of consistency/similarity between the drivers.
enum class BackendActionTy {
  Backend_EmitAssembly, ///< Emit native assembly files
  Backend_EmitObj,      ///< Emit native object files
  Backend_EmitBC,       ///< Emit LLVM bitcode files
  Backend_EmitLL,       ///< Emit human-readable LLVM assembly
  Backend_EmitFIR,      ///< Emit FIR files, possibly lowering via HLFIR
  Backend_EmitHLFIR,    ///< Emit HLFIR files before any passes run
};

/// Abstract base class for actions that generate code (MLIR, LLVM IR, assembly
/// and machine code). Every action that inherits from this class will at
/// least run the prescanning, parsing, semantic checks and lower the parse
/// tree to an MLIR module.
class CodeGenAction : public FrontendAction {

  void executeAction() override;
  /// Runs prescan, parsing, sema and lowers to MLIR.
  bool beginSourceFileAction() override;
  /// Runs the optimization (aka middle-end) pipeline on the LLVM module
  /// associated with this action.
  void runOptimizationPipeline(toolchain::raw_pwrite_stream &os);

protected:
  CodeGenAction(BackendActionTy act) : action{act} {};
  /// @name MLIR
  /// {
  std::unique_ptr<mlir::MLIRContext> mlirCtx;
  mlir::OwningOpRef<mlir::ModuleOp> mlirModule;
  /// }

  /// @name LLVM IR
  std::unique_ptr<toolchain::LLVMContext> toolchainCtx;
  std::unique_ptr<toolchain::Module> toolchainModule;

  /// Embeds offload objects specified with -fembed-offload-object
  void embedOffloadObjects();

  /// Links in BC libraries spefified with -mlink-builtin-bitcode
  void linkBuiltinBCLibs();

  /// Runs pass pipeline to lower HLFIR into FIR
  void lowerHLFIRToFIR();

  /// Generates an LLVM IR module from CodeGenAction::mlirModule and saves it
  /// in CodeGenAction::toolchainModule.
  void generateLLVMIR();

  BackendActionTy action;

  /// }
public:
  ~CodeGenAction() override;
};

class EmitFIRAction : public CodeGenAction {
public:
  EmitFIRAction() : CodeGenAction(BackendActionTy::Backend_EmitFIR) {}
};

class EmitHLFIRAction : public CodeGenAction {
public:
  EmitHLFIRAction() : CodeGenAction(BackendActionTy::Backend_EmitHLFIR) {}
};

class EmitLLVMAction : public CodeGenAction {
public:
  EmitLLVMAction() : CodeGenAction(BackendActionTy::Backend_EmitLL) {}
};

class EmitLLVMBitcodeAction : public CodeGenAction {
public:
  EmitLLVMBitcodeAction() : CodeGenAction(BackendActionTy::Backend_EmitBC) {}
};

class EmitObjAction : public CodeGenAction {
public:
  EmitObjAction() : CodeGenAction(BackendActionTy::Backend_EmitObj) {}
};

class EmitAssemblyAction : public CodeGenAction {
public:
  EmitAssemblyAction() : CodeGenAction(BackendActionTy::Backend_EmitAssembly) {}
};

} // namespace language::Compability::frontend

#endif // FORTRAN_FRONTEND_FRONTENDACTIONS_H
