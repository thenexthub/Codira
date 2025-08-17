//===--- Interpreter.h - Incremental Compilation and Execution---*- C++ -*-===//
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
// This file defines the component which performs incremental code
// compilation and execution.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_INTERPRETER_INTERPRETER_H
#define LANGUAGE_CORE_INTERPRETER_INTERPRETER_H

#include "language/Core/AST/GlobalDecl.h"
#include "language/Core/Interpreter/PartialTranslationUnit.h"
#include "language/Core/Interpreter/Value.h"

#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ExecutionEngine/JITSymbol.h"
#include "toolchain/ExecutionEngine/Orc/ExecutorProcessControl.h"
#include "toolchain/ExecutionEngine/Orc/Shared/ExecutorAddress.h"
#include "toolchain/Support/Error.h"
#include <memory>
#include <vector>

namespace toolchain {
namespace orc {
class LLJIT;
class LLJITBuilder;
class ThreadSafeContext;
} // namespace orc
} // namespace toolchain

namespace language::Core {

class CompilerInstance;
class CodeGenerator;
class CXXRecordDecl;
class Decl;
class IncrementalExecutor;
class IncrementalParser;
class IncrementalCUDADeviceParser;

/// Create a pre-configured \c CompilerInstance for incremental processing.
class IncrementalCompilerBuilder {
public:
  IncrementalCompilerBuilder() {}

  void SetCompilerArgs(const std::vector<const char *> &Args) {
    UserArgs = Args;
  }

  void SetTargetTriple(std::string TT) { TargetTriple = TT; }

  // General C++
  toolchain::Expected<std::unique_ptr<CompilerInstance>> CreateCpp();

  // Offload options
  void SetOffloadArch(toolchain::StringRef Arch) { OffloadArch = Arch; };

  // CUDA specific
  void SetCudaSDK(toolchain::StringRef path) { CudaSDKPath = path; };

  toolchain::Expected<std::unique_ptr<CompilerInstance>> CreateCudaHost();
  toolchain::Expected<std::unique_ptr<CompilerInstance>> CreateCudaDevice();

private:
  static toolchain::Expected<std::unique_ptr<CompilerInstance>>
  create(std::string TT, std::vector<const char *> &ClangArgv);

  toolchain::Expected<std::unique_ptr<CompilerInstance>> createCuda(bool device);

  std::vector<const char *> UserArgs;
  std::optional<std::string> TargetTriple;

  toolchain::StringRef OffloadArch;
  toolchain::StringRef CudaSDKPath;
};

class IncrementalAction;
class InProcessPrintingASTConsumer;

/// Provides top-level interfaces for incremental compilation and execution.
class Interpreter {
  friend class Value;
  friend InProcessPrintingASTConsumer;

  std::unique_ptr<toolchain::orc::ThreadSafeContext> TSCtx;
  /// Long-lived, incremental parsing action.
  std::unique_ptr<IncrementalAction> Act;
  std::unique_ptr<IncrementalParser> IncrParser;
  std::unique_ptr<IncrementalExecutor> IncrExecutor;

  // An optional parser for CUDA offloading
  std::unique_ptr<IncrementalCUDADeviceParser> DeviceParser;

  // An optional action for CUDA offloading
  std::unique_ptr<IncrementalAction> DeviceAct;

  /// List containing information about each incrementally parsed piece of code.
  std::list<PartialTranslationUnit> PTUs;

  unsigned InitPTUSize = 0;

  // This member holds the last result of the value printing. It's a class
  // member because we might want to access it after more inputs. If no value
  // printing happens, it's in an invalid state.
  Value LastValue;

  /// When CodeGen is created the first toolchain::Module gets cached in many places
  /// and we must keep it alive.
  std::unique_ptr<toolchain::Module> CachedInCodeGenModule;

  /// Compiler instance performing the incremental compilation.
  std::unique_ptr<CompilerInstance> CI;

  /// An optional compiler instance for CUDA offloading
  std::unique_ptr<CompilerInstance> DeviceCI;

protected:
  // Derived classes can use an extended interface of the Interpreter.
  Interpreter(std::unique_ptr<CompilerInstance> Instance, toolchain::Error &Err,
              std::unique_ptr<toolchain::orc::LLJITBuilder> JITBuilder = nullptr,
              std::unique_ptr<language::Core::ASTConsumer> Consumer = nullptr);

  // Create the internal IncrementalExecutor, or re-create it after calling
  // ResetExecutor().
  toolchain::Error CreateExecutor();

  // Delete the internal IncrementalExecutor. This causes a hard shutdown of the
  // JIT engine. In particular, it doesn't run cleanup or destructors.
  void ResetExecutor();

public:
  virtual ~Interpreter();
  static toolchain::Expected<std::unique_ptr<Interpreter>>
  create(std::unique_ptr<CompilerInstance> CI,
         std::unique_ptr<toolchain::orc::LLJITBuilder> JITBuilder = nullptr);
  static toolchain::Expected<std::unique_ptr<Interpreter>>
  createWithCUDA(std::unique_ptr<CompilerInstance> CI,
                 std::unique_ptr<CompilerInstance> DCI);
  static toolchain::Expected<std::unique_ptr<toolchain::orc::LLJITBuilder>>
  createLLJITBuilder(std::unique_ptr<toolchain::orc::ExecutorProcessControl> EPC,
                     toolchain::StringRef OrcRuntimePath);
  const ASTContext &getASTContext() const;
  ASTContext &getASTContext();
  const CompilerInstance *getCompilerInstance() const;
  CompilerInstance *getCompilerInstance();
  toolchain::Expected<toolchain::orc::LLJIT &> getExecutionEngine();

  toolchain::Expected<PartialTranslationUnit &> Parse(toolchain::StringRef Code);
  toolchain::Error Execute(PartialTranslationUnit &T);
  toolchain::Error ParseAndExecute(toolchain::StringRef Code, Value *V = nullptr);

  /// Undo N previous incremental inputs.
  toolchain::Error Undo(unsigned N = 1);

  /// Link a dynamic library
  toolchain::Error LoadDynamicLibrary(const char *name);

  /// \returns the \c ExecutorAddr of a \c GlobalDecl. This interface uses
  /// the CodeGenModule's internal mangling cache to avoid recomputing the
  /// mangled name.
  toolchain::Expected<toolchain::orc::ExecutorAddr> getSymbolAddress(GlobalDecl GD) const;

  /// \returns the \c ExecutorAddr of a given name as written in the IR.
  toolchain::Expected<toolchain::orc::ExecutorAddr>
  getSymbolAddress(toolchain::StringRef IRName) const;

  /// \returns the \c ExecutorAddr of a given name as written in the object
  /// file.
  toolchain::Expected<toolchain::orc::ExecutorAddr>
  getSymbolAddressFromLinkerName(toolchain::StringRef LinkerName) const;

  std::unique_ptr<toolchain::Module> GenModule(IncrementalAction *Action = nullptr);
  PartialTranslationUnit &RegisterPTU(TranslationUnitDecl *TU,
                                      std::unique_ptr<toolchain::Module> M = {},
                                      IncrementalAction *Action = nullptr);

private:
  size_t getEffectivePTUSize() const;
  void markUserCodeStart();
  toolchain::Expected<Expr *> ExtractValueFromExpr(Expr *E);

  // A cache for the compiled destructors used to for de-allocation of managed
  // language::Core::Values.
  mutable toolchain::DenseMap<CXXRecordDecl *, toolchain::orc::ExecutorAddr> Dtors;

  std::array<Expr *, 4> ValuePrintingInfo = {0};

  std::unique_ptr<toolchain::orc::LLJITBuilder> JITBuilder;

  /// @}
  /// @name Value and pretty printing support
  /// @{

  std::string ValueDataToString(const Value &V) const;
  std::string ValueTypeToString(const Value &V) const;

  toolchain::Expected<Expr *> convertExprToValue(Expr *E);

  // When we deallocate language::Core::Value we need to run the destructor of the type.
  // This function forces emission of the needed dtor.
  toolchain::Expected<toolchain::orc::ExecutorAddr>
  CompileDtorCall(CXXRecordDecl *CXXRD) const;

  /// @}
  /// @name Code generation
  /// @{
  CodeGenerator *getCodeGen(IncrementalAction *Action = nullptr) const;
};
} // namespace language::Core

#endif // LANGUAGE_CORE_INTERPRETER_INTERPRETER_H
