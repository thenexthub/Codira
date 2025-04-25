//===--- sil_nm_main.cpp --------------------------------------------------===//
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
/// This utility is a command line tool that given a sil or sib file dumps out
/// the names of the functions, globals, vtables, and witness tables in a
/// machine readable form. The intention is that it can be used with things like
/// sil-func-extractor to manipulate sil from the commandline.
///
//===----------------------------------------------------------------------===//

#include "language/Demangling/Demangle.h"
#include "language/Basic/LLVM.h"
#include "language/Basic/LLVMInitialize.h"
#include "language/Basic/Range.h"
#include "language/Frontend/DiagnosticVerifier.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILUndef.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/PassManager/PassManager.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/Serialization/SerializedModuleLoader.h"
#include "language/Serialization/SerializedSILLoader.h"
#include "language/Subsystems.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Signals.h"
#include <cstdio>
#include <functional>

using namespace language;

struct SILNMOptions {
  llvm::cl::opt<std::string>
    InputFilename = llvm::cl::opt<std::string>(llvm::cl::desc("input file"),
                                               llvm::cl::init("-"),
                                               llvm::cl::Positional);

  llvm::cl::list<std::string>
    ImportPaths = llvm::cl::list<std::string>("I",
                  llvm::cl::desc("add a directory to the import search path"));

  llvm::cl::opt<std::string>
    ModuleName = llvm::cl::opt<std::string>("module-name",
                 llvm::cl::desc("The name of the module if processing"
                                " a module. Necessary for processing "
                                "stdin."));

  llvm::cl::opt<bool>
    DemangleNames = llvm::cl::opt<bool>("demangle",
                    llvm::cl::desc("Demangle names of entities outputted"));

  llvm::cl::opt<std::string>
    ModuleCachePath = llvm::cl::opt<std::string>("module-cache-path",
                      llvm::cl::desc("Clang module cache path"));

  llvm::cl::opt<std::string>
    ResourceDir = llvm::cl::opt<std::string>(
      "resource-dir",
      llvm::cl::desc("The directory that holds the compiler resource files"));

  llvm::cl::opt<std::string>
    SDKPath = llvm::cl::opt<std::string>("sdk", llvm::cl::desc("The path to the SDK for use with the clang "
                                    "importer."),
              llvm::cl::init(""));

  llvm::cl::opt<std::string>
    Triple = llvm::cl::opt<std::string>("target", llvm::cl::desc("target triple"));
};

static void printAndSortNames(std::vector<StringRef> &Names, char Code,
                              const SILNMOptions &options) {
  std::sort(Names.begin(), Names.end());
  for (StringRef N : Names) {
    llvm::outs() << Code << " ";
    if (options.DemangleNames) {
      llvm::outs() << swift::Demangle::demangleSymbolAsString(N);
    } else {
      llvm::outs() << N;
    }
    llvm::outs() << '\n';
  }
}

static void nmModule(SILModule *M, const SILNMOptions &options) {
  {
    std::vector<StringRef> FuncNames;
    for (SILFunction &f : *M) {
      FuncNames.push_back(f.getName());
    }
    printAndSortNames(FuncNames, 'F', options);
  }

  {
    std::vector<StringRef> GlobalNames;
    for (SILGlobalVariable &g : M->getSILGlobals()) {
      GlobalNames.push_back(g.getName());
    }
    printAndSortNames(GlobalNames, 'G', options);
  }

  {
    std::vector<StringRef> WitnessTableNames;
    for (SILWitnessTable &wt : M->getWitnessTables()) {
      WitnessTableNames.push_back(wt.getName());
    }
    printAndSortNames(WitnessTableNames, 'W', options);
  }

  {
    std::vector<StringRef> VTableNames;
    for (SILVTable *vt : M->getVTables()) {
      VTableNames.push_back(vt->getClass()->getName().str());
    }
    printAndSortNames(VTableNames, 'V', options);
  }
}

int sil_nm_main(ArrayRef<const char *> argv, void *MainAddr) {
  INITIALIZE_LLVM();

  SILNMOptions options;

  llvm::cl::ParseCommandLineOptions(argv.size(), argv.data(), "SIL NM\n");

  CompilerInvocation Invocation;

  Invocation.setMainExecutablePath(llvm::sys::fs::getMainExecutable(argv[0], MainAddr));

  // Give the context the list of search paths to use for modules.
  std::vector<SearchPathOptions::SearchPath> ImportPaths;
  for (const auto &path : options.ImportPaths) {
    ImportPaths.push_back({path, /*isSystem=*/false});
  }
  Invocation.setImportSearchPaths(ImportPaths);
  // Set the SDK path and target if given.
  if (options.SDKPath.getNumOccurrences() == 0) {
    const char *SDKROOT = getenv("SDKROOT");
    if (SDKROOT)
      options.SDKPath = SDKROOT;
  }
  if (!options.SDKPath.empty())
    Invocation.setSDKPath(options.SDKPath);
  if (!options.Triple.empty())
    Invocation.setTargetTriple(options.Triple);
  if (!options.ResourceDir.empty())
    Invocation.setRuntimeResourcePath(options.ResourceDir);
  Invocation.getClangImporterOptions().ModuleCachePath = options.ModuleCachePath;
  Invocation.setParseStdlib();
  Invocation.getLangOptions().DisableAvailabilityChecking = true;
  Invocation.getLangOptions().EnableAccessControl = false;
  Invocation.getLangOptions().EnableObjCAttrRequiresFoundation = false;

  serialization::ExtendedValidationInfo extendedInfo;
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> FileBufOrErr =
      Invocation.setUpInputForSILTool(options.InputFilename, options.ModuleName,
                                      /*alwaysSetModuleToMain*/ true,
                                      /*bePrimary*/ false, extendedInfo);
  if (!FileBufOrErr) {
    fprintf(stderr, "Error! Failed to open file: %s\n", options.InputFilename.c_str());
    exit(-1);
  }

  CompilerInstance CI;
  PrintingDiagnosticConsumer PrintDiags;
  CI.addDiagnosticConsumer(&PrintDiags);

  std::string InstanceSetupError;
  if (CI.setup(Invocation, InstanceSetupError)) {
    llvm::errs() << InstanceSetupError << '\n';
    return 1;
  }
  CI.performSema();

  // If parsing produced an error, don't run any passes.
  if (CI.getASTContext().hadError())
    return 1;

  auto SILMod = performASTLowering(CI.getMainModule(), CI.getSILTypes(),
                                   CI.getSILOptions());
  nmModule(SILMod.get(), options);

  return CI.getASTContext().hadError();
}
