//===--- sil_toolchain_gen_main.cpp --------------------------------------------===//
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
///
/// \file
///
/// This is a tool for reading sil files and running IRGen passes upon them. It
/// is not meant to be used to run toolchain optimizations on toolchain-ir.
///
//===----------------------------------------------------------------------===//

#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/IRGenRequests.h"
#include "language/AST/SILOptions.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Frontend/DiagnosticVerifier.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/IRGen/IRGenPublic.h"
#include "language/IRGen/IRGenSILPasses.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/PassManager/PassManager.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/Serialization/SerializationOptions.h"
#include "language/Serialization/SerializedModuleLoader.h"
#include "language/Serialization/SerializedSILLoader.h"
#include "language/Strings.h"
#include "language/Subsystems.h"
#include "toolchain/ADT/ScopeExit.h"
#include "toolchain/ADT/Statistic.h"
#include "toolchain/IR/Module.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/ManagedStatic.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Signals.h"
#include "toolchain/Support/TargetSelect.h"
#include "toolchain/Support/VirtualOutputBackends.h"
#include <cstdio>
using namespace language;

struct SILLLVMGenOptions {
  toolchain::cl::opt<std::string>
    InputFilename = toolchain::cl::opt<std::string>(toolchain::cl::desc("input file"),
                                                  toolchain::cl::init("-"),
                                                  toolchain::cl::Positional);

  toolchain::cl::opt<std::string>
    OutputFilename = toolchain::cl::opt<std::string>("o", toolchain::cl::init("-"), toolchain::cl::desc("output filename"));

  toolchain::cl::list<std::string>
    ImportPaths = toolchain::cl::list<std::string>("I",
                  toolchain::cl::desc("add a directory to the import search path"));

  toolchain::cl::list<std::string>
    FrameworkPaths = toolchain::cl::list<std::string>(
      "F", toolchain::cl::desc("add a directory to the framework search path"));

  toolchain::cl::opt<std::string>
    ModuleName = toolchain::cl::opt<std::string>("module-name",
                 toolchain::cl::desc("The name of the module if processing"
                                " a module. Necessary for processing "
                                "stdin."));

  toolchain::cl::opt<std::string>
    ResourceDir = toolchain::cl::opt<std::string>(
      "resource-dir",
      toolchain::cl::desc("The directory that holds the compiler resource files"));

  toolchain::cl::opt<std::string>
    SDKPath = toolchain::cl::opt<std::string>("sdk", toolchain::cl::desc("The path to the SDK for use with the clang "
                                                               "importer."),
              toolchain::cl::init(""));

  toolchain::cl::opt<std::string>
    Target = toolchain::cl::opt<std::string>("target",
                                           toolchain::cl::desc("target triple"));

  toolchain::cl::opt<bool>
    PrintStats = toolchain::cl::opt<bool>("print-stats", toolchain::cl::desc("Print various statistics"));

  toolchain::cl::opt<std::string>
    ModuleCachePath = toolchain::cl::opt<std::string>("module-cache-path",
                      toolchain::cl::desc("Clang module cache path"));

  toolchain::cl::opt<bool>
    PerformWMO = toolchain::cl::opt<bool>("wmo", toolchain::cl::desc("Enable whole-module optimizations"));

  toolchain::cl::opt<IRGenOutputKind>
    OutputKind = toolchain::cl::opt<IRGenOutputKind>(
      "output-kind", toolchain::cl::desc("Type of output to produce"),
      toolchain::cl::values(clEnumValN(IRGenOutputKind::LLVMAssemblyAfterOptimization,
                                  "toolchain-as", "Emit toolchain assembly"),
                       clEnumValN(IRGenOutputKind::LLVMBitcode, "toolchain-bc",
                                  "Emit toolchain bitcode"),
                       clEnumValN(IRGenOutputKind::NativeAssembly, "as",
                                  "Emit native assembly"),
                       clEnumValN(IRGenOutputKind::ObjectFile, "object",
                                  "Emit an object file")),
      toolchain::cl::init(IRGenOutputKind::ObjectFile));

  toolchain::cl::opt<bool>
    DisableLegacyTypeInfo = toolchain::cl::opt<bool>("disable-legacy-type-info",
          toolchain::cl::desc("Don't try to load backward deployment layouts"));
};

int sil_toolchain_gen_main(ArrayRef<const char *> argv, void *MainAddr) {
  INITIALIZE_LLVM();

  SILLLVMGenOptions options;

  toolchain::cl::ParseCommandLineOptions(argv.size(), argv.data(), "Codira LLVM IR Generator\n");

  if (options.PrintStats)
    toolchain::EnableStatistics();

  CompilerInvocation Invocation;

  Invocation.setMainExecutablePath(toolchain::sys::fs::getMainExecutable(argv[0], MainAddr));

  // Give the context the list of search paths to use for modules.
  std::vector<SearchPathOptions::SearchPath> ImportPaths;
  for (const auto &path : options.ImportPaths) {
    ImportPaths.push_back({path, /*isSystem=*/false});
  }
  Invocation.setImportSearchPaths(ImportPaths);
  std::vector<SearchPathOptions::SearchPath> FramePaths;
  for (const auto &path : options.FrameworkPaths) {
    FramePaths.push_back({path, /*isSystem=*/false});
  }
  Invocation.setFrameworkSearchPaths(FramePaths);
  // Set the SDK path and target if given.
  if (options.SDKPath.getNumOccurrences() == 0) {
    const char *SDKROOT = getenv("SDKROOT");
    if (SDKROOT)
      options.SDKPath = SDKROOT;
  }
  if (!options.SDKPath.empty())
    Invocation.setSDKPath(options.SDKPath);
  if (!options.Target.empty())
    Invocation.setTargetTriple(options.Target);
  if (!options.ResourceDir.empty())
    Invocation.setRuntimeResourcePath(options.ResourceDir);
  // Set the module cache path. If not passed in we use the default language module
  // cache.
  Invocation.getClangImporterOptions().ModuleCachePath = options.ModuleCachePath;
  Invocation.setParseStdlib();

  // Setup the language options
  auto &LangOpts = Invocation.getLangOptions();
  LangOpts.DisableAvailabilityChecking = true;
  LangOpts.EnableAccessControl = false;
  LangOpts.EnableObjCAttrRequiresFoundation = false;
  LangOpts.EnableObjCInterop = LangOpts.Target.isOSDarwin();

  // Setup the IRGen Options.
  IRGenOptions &Opts = Invocation.getIRGenOptions();
  Opts.OutputKind = options.OutputKind;
  Opts.DisableLegacyTypeInfo = options.DisableLegacyTypeInfo;

  serialization::ExtendedValidationInfo extendedInfo;
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> FileBufOrErr =
      Invocation.setUpInputForSILTool(options.InputFilename, options.ModuleName,
                                      /*alwaysSetModuleToMain*/ false,
                                      /*bePrimary*/ !options.PerformWMO, extendedInfo);
  if (!FileBufOrErr) {
    fprintf(stderr, "Error! Failed to open file: %s\n", options.InputFilename.c_str());
    exit(-1);
  }

  CompilerInstance CI;
  PrintingDiagnosticConsumer PrintDiags;
  CI.addDiagnosticConsumer(&PrintDiags);

  std::string InstanceSetupError;
  if (CI.setup(Invocation, InstanceSetupError)) {
    toolchain::errs() << InstanceSetupError << '\n';
    return 1;
  }

  toolchain::vfs::OnDiskOutputBackend Backend;
  auto outFile = Backend.createFile(options.OutputFilename);
  if (!outFile) {
    CI.getDiags().diagnose(SourceLoc(), diag::error_opening_output,
                           options.OutputFilename, toString(outFile.takeError()));
    return 1;
  }
  auto closeFile = toolchain::make_scope_exit([&]() {
    if (auto E = outFile->keep()) {
      CI.getDiags().diagnose(SourceLoc(), diag::error_closing_output,
                             options.OutputFilename, toString(std::move(E)));
    }
  });

  auto *mod = CI.getMainModule();
  assert(mod->getFiles().size() == 1);

  const auto &TBDOpts = Invocation.getTBDGenOptions();
  const auto &SILOpts = Invocation.getSILOptions();
  auto &SILTypes = CI.getSILTypes();
  auto moduleName = CI.getMainModule()->getName().str();
  const PrimarySpecificPaths PSPs(options.OutputFilename, options.InputFilename);

  auto getDescriptor = [&]() -> IRGenDescriptor {
    if (options.PerformWMO) {
      return IRGenDescriptor::forWholeModule(
          mod, Opts, TBDOpts, SILOpts, SILTypes,
          /*SILMod*/ nullptr, moduleName, PSPs);
    } else {
      return IRGenDescriptor::forFile(
          mod->getFiles()[0], Opts, TBDOpts, SILOpts, SILTypes,
          /*SILMod*/ nullptr, moduleName, PSPs, /*discriminator*/ "");
    }
  };

  auto &eval = CI.getASTContext().evaluator;
  auto desc = getDescriptor();
  desc.out = &outFile->getOS();
  auto generatedMod = evaluateOrFatal(eval, OptimizedIRRequest{desc});
  if (!generatedMod)
    return 1;

  return compileAndWriteLLVM(generatedMod.getModule(),
                             generatedMod.getTargetMachine(), Opts,
                             CI.getStatsReporter(), CI.getDiags(), *outFile);
}
