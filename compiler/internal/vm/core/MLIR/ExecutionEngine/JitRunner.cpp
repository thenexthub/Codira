//===- jit-runner.cpp - MLIR CPU Execution Driver Library -----------------===//
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
// This is a library that provides a shared implementation for command line
// utilities that execute an MLIR file on the CPU by translating MLIR to LLVM
// IR before JIT-compiling and executing the latter.
//
// The translation can be customized by providing an MLIR to MLIR
// transformation.
//===----------------------------------------------------------------------===//

#include "mlir/ExecutionEngine/JitRunner.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/ExecutionEngine/ExecutionEngine.h"
#include "mlir/ExecutionEngine/OptUtils.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Tools/ParseUtilities.h"

#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "vm/core/ExecutionEngine/Orc/LLJIT.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/LegacyPassNameParser.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/FileUtilities.h"
#include "vm/core/Support/SourceMgr.h"
#include "vm/core/Support/StringSaver.h"
#include "vm/core/Support/ToolOutputFile.h"
#include <cstdint>
#include <numeric>
#include <optional>
#include <utility>

#define DEBUG_TYPE "jit-runner"

using namespace mlir;
using toolchain::Error;

namespace {
/// This options struct prevents the need for global static initializers, and
/// is only initialized if the JITRunner is invoked.
struct Options {
  toolchain::cl::opt<std::string> inputFilename{toolchain::cl::Positional,
                                           toolchain::cl::desc("<input file>"),
                                           toolchain::cl::init("-")};
  toolchain::cl::opt<std::string> mainFuncName{
      "e", toolchain::cl::desc("The function to be called"),
      toolchain::cl::value_desc("<function name>"), toolchain::cl::init("main")};
  toolchain::cl::opt<std::string> mainFuncType{
      "entry-point-result",
      toolchain::cl::desc("Textual description of the function type to be called"),
      toolchain::cl::value_desc("f32 | i32 | i64 | void"), toolchain::cl::init("f32")};

  toolchain::cl::OptionCategory optFlags{"opt-like flags"};

  // CLI variables for -On options.
  toolchain::cl::opt<bool> optO0{"O0",
                            toolchain::cl::desc("Run opt passes and codegen at O0"),
                            toolchain::cl::cat(optFlags)};
  toolchain::cl::opt<bool> optO1{"O1",
                            toolchain::cl::desc("Run opt passes and codegen at O1"),
                            toolchain::cl::cat(optFlags)};
  toolchain::cl::opt<bool> optO2{"O2",
                            toolchain::cl::desc("Run opt passes and codegen at O2"),
                            toolchain::cl::cat(optFlags)};
  toolchain::cl::opt<bool> optO3{"O3",
                            toolchain::cl::desc("Run opt passes and codegen at O3"),
                            toolchain::cl::cat(optFlags)};

  toolchain::cl::list<std::string> mAttrs{
      "mattr", toolchain::cl::MiscFlags::CommaSeparated,
      toolchain::cl::desc("Target specific attributes (-mattr=help for details)"),
      toolchain::cl::value_desc("a1,+a2,-a3,..."), toolchain::cl::cat(optFlags)};

  toolchain::cl::opt<std::string> mArch{
      "march",
      toolchain::cl::desc("Architecture to generate code for (see --version)")};

  toolchain::cl::OptionCategory clOptionsCategory{"linking options"};
  toolchain::cl::list<std::string> clSharedLibs{
      "shared-libs", toolchain::cl::desc("Libraries to link dynamically"),
      toolchain::cl::MiscFlags::CommaSeparated, toolchain::cl::cat(clOptionsCategory)};

  /// CLI variables for debugging.
  toolchain::cl::opt<bool> dumpObjectFile{
      "dump-object-file",
      toolchain::cl::desc("Dump JITted-compiled object to file specified with "
                     "-object-filename (<input file>.o by default).")};

  toolchain::cl::opt<std::string> objectFilename{
      "object-filename",
      toolchain::cl::desc("Dump JITted-compiled object to file <input file>.o")};

  toolchain::cl::opt<bool> hostSupportsJit{"host-supports-jit",
                                      toolchain::cl::desc("Report host JIT support"),
                                      toolchain::cl::Hidden};

  toolchain::cl::opt<bool> noImplicitModule{
      "no-implicit-module",
      toolchain::cl::desc(
          "Disable implicit addition of a top-level module op during parsing"),
      toolchain::cl::init(false)};
};

struct CompileAndExecuteConfig {
  /// LLVM module transformer that is passed to ExecutionEngine.
  std::function<toolchain::Error(toolchain::Module *)> transformer;

  /// A custom function that is passed to ExecutionEngine. It processes MLIR
  /// module and creates LLVM IR module.
  toolchain::function_ref<std::unique_ptr<toolchain::Module>(Operation *,
                                                   toolchain::LLVMContext &)>
      llvmModuleBuilder;

  /// A custom function that is passed to ExecutinEngine to register symbols at
  /// runtime.
  toolchain::function_ref<toolchain::orc::SymbolMap(toolchain::orc::MangleAndInterner)>
      runtimeSymbolMap;
};

} // namespace

static OwningOpRef<Operation *> parseMLIRInput(StringRef inputFilename,
                                               bool insertImplicitModule,
                                               MLIRContext *context) {
  // Set up the input file.
  std::string errorMessage;
  auto file = openInputFile(inputFilename, &errorMessage);
  if (!file) {
    toolchain::errs() << errorMessage << "\n";
    return nullptr;
  }

  auto sourceMgr = std::make_shared<toolchain::SourceMgr>();
  sourceMgr->AddNewSourceBuffer(std::move(file), SMLoc());
  OwningOpRef<Operation *> module =
      parseSourceFileForTool(sourceMgr, context, insertImplicitModule);
  if (!module)
    return nullptr;
  if (!module.get()->hasTrait<OpTrait::SymbolTable>()) {
    toolchain::errs() << "Error: top-level op must be a symbol table.\n";
    return nullptr;
  }
  return module;
}

static inline Error makeStringError(const Twine &message) {
  return toolchain::make_error<toolchain::StringError>(message.str(),
                                             toolchain::inconvertibleErrorCode());
}

static std::optional<unsigned> getCommandLineOptLevel(Options &options) {
  std::optional<unsigned> optLevel;
  SmallVector<std::reference_wrapper<toolchain::cl::opt<bool>>, 4> optFlags{
      options.optO0, options.optO1, options.optO2, options.optO3};

  // Determine if there is an optimization flag present.
  for (unsigned j = 0; j < 4; ++j) {
    auto &flag = optFlags[j].get();
    if (flag) {
      optLevel = j;
      break;
    }
  }
  return optLevel;
}

// JIT-compile the given module and run "entryPoint" with "args" as arguments.
static Error
compileAndExecute(Options &options, Operation *module, StringRef entryPoint,
                  CompileAndExecuteConfig config, void **args,
                  std::unique_ptr<toolchain::TargetMachine> tm = nullptr) {
  std::optional<toolchain::CodeGenOptLevel> jitCodeGenOptLevel;
  if (auto clOptLevel = getCommandLineOptLevel(options))
    jitCodeGenOptLevel = static_cast<toolchain::CodeGenOptLevel>(*clOptLevel);

  SmallVector<StringRef, 4> sharedLibs(options.clSharedLibs.begin(),
                                       options.clSharedLibs.end());

  mlir::ExecutionEngineOptions engineOptions;
  engineOptions.llvmModuleBuilder = config.llvmModuleBuilder;
  if (config.transformer)
    engineOptions.transformer = config.transformer;
  engineOptions.jitCodeGenOptLevel = jitCodeGenOptLevel;
  engineOptions.sharedLibPaths = sharedLibs;
  engineOptions.enableObjectDump = true;
  auto expectedEngine =
      mlir::ExecutionEngine::create(module, engineOptions, std::move(tm));
  if (!expectedEngine)
    return expectedEngine.takeError();

  auto engine = std::move(*expectedEngine);

  engine->initialize();

  auto expectedFPtr = engine->lookupPacked(entryPoint);
  if (!expectedFPtr)
    return expectedFPtr.takeError();

  if (options.dumpObjectFile)
    engine->dumpToObjectFile(options.objectFilename.empty()
                                 ? options.inputFilename + ".o"
                                 : options.objectFilename);

  void (*fptr)(void **) = *expectedFPtr;
  (*fptr)(args);

  return Error::success();
}

static Error compileAndExecuteVoidFunction(
    Options &options, Operation *module, StringRef entryPoint,
    CompileAndExecuteConfig config, std::unique_ptr<toolchain::TargetMachine> tm) {
  auto mainFunction = dyn_cast_or_null<LLVM::LLVMFuncOp>(
      SymbolTable::lookupSymbolIn(module, entryPoint));
  if (!mainFunction || mainFunction.isExternal())
    return makeStringError("entry point not found");

  if (cast<LLVM::LLVMFunctionType>(mainFunction.getFunctionType())
          .getNumParams() != 0)
    return makeStringError(
        "JIT can't invoke a main function expecting arguments");

  auto resultType = dyn_cast<LLVM::LLVMVoidType>(
      mainFunction.getFunctionType().getReturnType());
  if (!resultType)
    return makeStringError("expected void function");

  void *empty = nullptr;
  return compileAndExecute(options, module, entryPoint, std::move(config),
                           &empty, std::move(tm));
}

template <typename Type>
Error checkCompatibleReturnType(LLVM::LLVMFuncOp mainFunction);
template <>
Error checkCompatibleReturnType<int32_t>(LLVM::LLVMFuncOp mainFunction) {
  auto resultType = dyn_cast<IntegerType>(
      cast<LLVM::LLVMFunctionType>(mainFunction.getFunctionType())
          .getReturnType());
  if (!resultType || resultType.getWidth() != 32)
    return makeStringError("only single i32 function result supported");
  return Error::success();
}
template <>
Error checkCompatibleReturnType<int64_t>(LLVM::LLVMFuncOp mainFunction) {
  auto resultType = dyn_cast<IntegerType>(
      cast<LLVM::LLVMFunctionType>(mainFunction.getFunctionType())
          .getReturnType());
  if (!resultType || resultType.getWidth() != 64)
    return makeStringError("only single i64 function result supported");
  return Error::success();
}
template <>
Error checkCompatibleReturnType<float>(LLVM::LLVMFuncOp mainFunction) {
  if (!isa<Float32Type>(
          cast<LLVM::LLVMFunctionType>(mainFunction.getFunctionType())
              .getReturnType()))
    return makeStringError("only single f32 function result supported");
  return Error::success();
}
template <typename Type>
static Error compileAndExecuteSingleReturnFunction(
    Options &options, Operation *module, StringRef entryPoint,
    CompileAndExecuteConfig config, std::unique_ptr<toolchain::TargetMachine> tm) {
  auto mainFunction = dyn_cast_or_null<LLVM::LLVMFuncOp>(
      SymbolTable::lookupSymbolIn(module, entryPoint));
  if (!mainFunction || mainFunction.isExternal())
    return makeStringError("entry point not found");

  if (cast<LLVM::LLVMFunctionType>(mainFunction.getFunctionType())
          .getNumParams() != 0)
    return makeStringError(
        "JIT can't invoke a main function expecting arguments");

  if (Error error = checkCompatibleReturnType<Type>(mainFunction))
    return error;

  Type res;
  struct {
    void *data;
  } data;
  data.data = &res;
  if (auto error =
          compileAndExecute(options, module, entryPoint, std::move(config),
                            (void **)&data, std::move(tm)))
    return error;

  // Intentional printing of the output so we can test.
  toolchain::outs() << res << '\n';

  return Error::success();
}

/// Entry point for all CPU runners. Expects the common argc/argv arguments for
/// standard C++ main functions.
int mlir::JitRunnerMain(int argc, char **argv, const DialectRegistry &registry,
                        JitRunnerConfig config) {
  toolchain::ExitOnError exitOnErr;

  // Create the options struct containing the command line options for the
  // runner. This must come before the command line options are parsed.
  Options options;
  toolchain::cl::ParseCommandLineOptions(argc, argv, "MLIR CPU execution driver\n");

  if (options.hostSupportsJit) {
    auto j = toolchain::orc::LLJITBuilder().create();
    if (j)
      toolchain::outs() << "true\n";
    else {
      toolchain::outs() << "false\n";
      exitOnErr(j.takeError());
    }
    return 0;
  }

  std::optional<unsigned> optLevel = getCommandLineOptLevel(options);
  SmallVector<std::reference_wrapper<toolchain::cl::opt<bool>>, 4> optFlags{
      options.optO0, options.optO1, options.optO2, options.optO3};

  MLIRContext context(registry);

  auto m = parseMLIRInput(options.inputFilename, !options.noImplicitModule,
                          &context);
  if (!m) {
    toolchain::errs() << "could not parse the input IR\n";
    return 1;
  }

  JitRunnerOptions runnerOptions{options.mainFuncName, options.mainFuncType};
  if (config.mlirTransformer)
    if (failed(config.mlirTransformer(m.get(), runnerOptions)))
      return EXIT_FAILURE;

  auto tmBuilderOrError = toolchain::orc::JITTargetMachineBuilder::detectHost();
  if (!tmBuilderOrError) {
    toolchain::errs() << "Failed to create a JITTargetMachineBuilder for the host\n";
    return EXIT_FAILURE;
  }

  // Configure TargetMachine builder based on the command line options
  toolchain::SubtargetFeatures features;
  if (!options.mAttrs.empty()) {
    for (StringRef attr : options.mAttrs)
      features.AddFeature(attr);
    tmBuilderOrError->addFeatures(features.getFeatures());
  }

  if (!options.mArch.empty()) {
    tmBuilderOrError->getTargetTriple().setArchName(options.mArch);
  }

  // Build TargetMachine
  auto tmOrError = tmBuilderOrError->createTargetMachine();

  if (!tmOrError) {
    toolchain::errs() << "Failed to create a TargetMachine for the host\n";
    exitOnErr(tmOrError.takeError());
  }

  LLVM_DEBUG({
    toolchain::dbgs() << "  JITTargetMachineBuilder is "
                 << toolchain::orc::JITTargetMachineBuilderPrinter(*tmBuilderOrError,
                                                              "\n");
  });

  CompileAndExecuteConfig compileAndExecuteConfig;
  if (optLevel) {
    compileAndExecuteConfig.transformer = mlir::makeOptimizingTransformer(
        *optLevel, /*sizeLevel=*/0, /*targetMachine=*/tmOrError->get());
  }
  compileAndExecuteConfig.llvmModuleBuilder = config.llvmModuleBuilder;
  compileAndExecuteConfig.runtimeSymbolMap = config.runtimesymbolMap;

  // Get the function used to compile and execute the module.
  using CompileAndExecuteFnT =
      Error (*)(Options &, Operation *, StringRef, CompileAndExecuteConfig,
                std::unique_ptr<toolchain::TargetMachine> tm);
  auto compileAndExecuteFn =
      StringSwitch<CompileAndExecuteFnT>(options.mainFuncType.getValue())
          .Case("i32", compileAndExecuteSingleReturnFunction<int32_t>)
          .Case("i64", compileAndExecuteSingleReturnFunction<int64_t>)
          .Case("f32", compileAndExecuteSingleReturnFunction<float>)
          .Case("void", compileAndExecuteVoidFunction)
          .Default(nullptr);

  Error error = compileAndExecuteFn
                    ? compileAndExecuteFn(
                          options, m.get(), options.mainFuncName.getValue(),
                          compileAndExecuteConfig, std::move(tmOrError.get()))
                    : makeStringError("unsupported function type");

  int exitCode = EXIT_SUCCESS;
  toolchain::handleAllErrors(std::move(error),
                        [&exitCode](const toolchain::ErrorInfoBase &info) {
                          toolchain::errs() << "Error: ";
                          info.log(toolchain::errs());
                          toolchain::errs() << '\n';
                          exitCode = EXIT_FAILURE;
                        });

  return exitCode;
}
