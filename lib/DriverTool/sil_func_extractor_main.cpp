//===--- sil_func_extractor_main.cpp --------------------------------------===//
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
/// This utility is meant to help simplify the extraction of test cases from sil
/// files by removing (currently only) functions that do not match a list of
/// string. It also allows for the inverse to be selected. Eventually this
/// should have additional capabilities like stripping globals, vtables, etc.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-fn-extractor"
#include "language/Basic/Assertions.h"
#include "language/Basic/FileTypes.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Demangling/Demangle.h"
#include "language/Demangling/ManglingMacros.h"
#include "language/Frontend/DiagnosticVerifier.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILUndef.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/PassManager/PassManager.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/Serialization/SerializedModuleLoader.h"
#include "language/Serialization/SerializationOptions.h"
#include "language/Serialization/SerializedSILLoader.h"
#include "language/SymbolGraphGen/SymbolGraphOptions.h"
#include "language/Subsystems.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/ManagedStatic.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Signals.h"
#include <cstdio>

using namespace language;

struct SILFuncExtractorOptions {
  toolchain::cl::opt<std::string>
    InputFilename = toolchain::cl::opt<std::string>(toolchain::cl::desc("input file"),
                                                toolchain::cl::init("-"),
                                                toolchain::cl::Positional);

  toolchain::cl::opt<std::string>
    OutputFilename = toolchain::cl::opt<std::string>("o", toolchain::cl::desc("output filename"), toolchain::cl::init("-"));

  toolchain::cl::opt<bool>
    EmitVerboseSIL = toolchain::cl::opt<bool>("emit-verbose-sil",
                   toolchain::cl::desc("Emit locations during sil emission."));

  toolchain::cl::list<std::string>
    CommandLineFunctionNames = toolchain::cl::list<std::string>("fn",
                             toolchain::cl::desc("Function names to extract."));
  toolchain::cl::opt<std::string>
    FunctionNameFile = toolchain::cl::opt<std::string>(
    "fn-file", toolchain::cl::desc("File to load additional function names from"));

  toolchain::cl::opt<bool>
    EmitSIB = toolchain::cl::opt<bool>("emit-sib",
        toolchain::cl::desc("Emit a sib file as output instead of a sil file"));

  toolchain::cl::opt<bool>
    InvertMatch = toolchain::cl::opt<bool>(
                  "invert",
                  toolchain::cl::desc("Match functions whose name do not "
                  "match the names of the functions to be extracted"));

  toolchain::cl::list<std::string>
    ImportPaths = toolchain::cl::list<std::string>("I",
                toolchain::cl::desc("add a directory to the import search path"));

  toolchain::cl::opt<std::string>
    ModuleName = toolchain::cl::opt<std::string>("module-name",
               toolchain::cl::desc("The name of the module if processing"
                              " a module. Necessary for processing "
                              "stdin."));

  toolchain::cl::opt<std::string>
    ModuleCachePath = toolchain::cl::opt<std::string>("module-cache-path",
                    toolchain::cl::desc("Clang module cache path"));

  toolchain::cl::opt<std::string>
    ResourceDir = toolchain::cl::opt<std::string>(
                "resource-dir",
                toolchain::cl::desc("The directory that holds the compiler resource files"));

  toolchain::cl::opt<std::string>
    SDKPath = toolchain::cl::opt<std::string>("sdk", toolchain::cl::desc("The path to the SDK for use with the clang "
                                  "importer."),
              toolchain::cl::init(""));

  toolchain::cl::opt<std::string>
    Triple = toolchain::cl::opt<std::string>("target",
                                        toolchain::cl::desc("target triple"));

  toolchain::cl::opt<bool>
    EmitSortedSIL = toolchain::cl::opt<bool>("emit-sorted-sil", toolchain::cl::Hidden, toolchain::cl::init(false),
                  toolchain::cl::desc("Sort Functions, VTables, Globals, "
                                 "WitnessTables by name to ease diffing."));

  toolchain::cl::opt<bool>
    DisableASTDump = toolchain::cl::opt<bool>("sil-disable-ast-dump", toolchain::cl::Hidden,
                 toolchain::cl::init(false),
                 toolchain::cl::desc("Do not dump AST."));

  toolchain::cl::opt<bool> EnableOSSAModules = toolchain::cl::opt<bool>(
      "enable-ossa-modules", toolchain::cl::init(true),
      toolchain::cl::desc("Do we always serialize SIL in OSSA form? If "
                     "this is disabled we do not serialize in OSSA "
                     "form when optimizing."));

  toolchain::cl::opt<toolchain::cl::boolOrDefault>
    EnableObjCInterop = toolchain::cl::opt<toolchain::cl::boolOrDefault>(
      "enable-objc-interop",
      toolchain::cl::desc("Whether the Objective-C interop should be enabled. "
                     "The value is `true` by default on Darwin platforms."));
};

static void getFunctionNames(std::vector<std::string> &Names,
                             const SILFuncExtractorOptions &options) {
  std::copy(options.CommandLineFunctionNames.begin(), options.CommandLineFunctionNames.end(),
            std::back_inserter(Names));

  if (!options.FunctionNameFile.empty()) {
    toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> FileBufOrErr =
        toolchain::MemoryBuffer::getFileOrSTDIN(options.FunctionNameFile);
    if (!FileBufOrErr) {
      fprintf(stderr, "Error! Failed to open file: %s\n",
              options.InputFilename.c_str());
      exit(-1);
    }
    StringRef Buffer = FileBufOrErr.get()->getBuffer();
    while (!Buffer.empty()) {
      StringRef Token, NewBuffer;
      std::tie(Token, NewBuffer) = toolchain::getToken(Buffer, "\n");
      if (Token.empty()) {
        break;
      }
      Names.push_back(Token.str());
      Buffer = NewBuffer;
    }
  }
}

static bool stringInSortedArray(
    StringRef str, ArrayRef<std::string> list,
    toolchain::function_ref<bool(const std::string &, const std::string &)> &&cmp) {
  if (list.empty())
    return false;
  auto iter = std::lower_bound(list.begin(), list.end(), str.str(), cmp);
  // If we didn't find str, return false.
  if (list.end() == iter)
    return false;

  return !cmp(str.str(), *iter);
}

void removeUnwantedFunctions(SILModule *M, ArrayRef<std::string> MangledNames,
                             ArrayRef<std::string> DemangledNames,
                             const SILFuncExtractorOptions &options) {
  assert((!MangledNames.empty() || !DemangledNames.empty()) &&
         "Expected names of function we want to retain!");
  assert(M && "Expected a SIL module to extract from.");

  std::vector<SILFunction *> DeadFunctions;
  for (auto &F : M->getFunctionList()) {
    StringRef MangledName = F.getName();
    std::string DemangledName =
        language::Demangle::demangleSymbolAsString(MangledName);
    DemangledName = DemangledName.substr(0, DemangledName.find_first_of(" <("));
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Visiting New Func:\n"
                            << "    Mangled: " << MangledName
                            << "\n    Demangled: " << DemangledName << "\n");

    bool FoundMangledName = stringInSortedArray(MangledName, MangledNames,
                                                std::less<std::string>());
    bool FoundDemangledName = stringInSortedArray(
        DemangledName, DemangledNames,
        [](const std::string &str1, const std::string &str2) -> bool {
          return str1.substr(0, str1.find(' ')) <
                 str2.substr(0, str2.find(' '));
        });
    if ((FoundMangledName || FoundDemangledName) ^ options.InvertMatch) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Not removing!\n");
      continue;
    }

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Removing!\n");

    // If F has no body, there is nothing further to do.
    if (!F.size())
      continue;

    SILBasicBlock &BB = F.front();
    SILLocation Loc = BB.back().getLoc();
    BB.split(BB.begin());
    // Make terminator unreachable.
    SILBuilder(&BB).createUnreachable(Loc);
    DeadFunctions.push_back(&F);
  }

  // After running this pass all of the functions we will remove
  // should consist only of one basic block terminated by
  // UnreachableInst.
  performSILDiagnoseUnreachable(M);

  // Now mark all of these functions as public and remove their bodies.
  for (auto &F : DeadFunctions) {
    F->setLinkage(SILLinkage::PublicExternal);
    F->clear();
  }

  // Remove dead functions.
  performSILDeadFunctionElimination(M);
}

int sil_func_extractor_main(ArrayRef<const char *> argv, void *MainAddr) {
  INITIALIZE_LLVM();

  SILFuncExtractorOptions options;

  toolchain::cl::ParseCommandLineOptions(argv.size(), argv.data(), "Codira SIL Extractor\n");

  CompilerInvocation Invocation;

  Invocation.setMainExecutablePath(toolchain::sys::fs::getMainExecutable(argv[0], MainAddr));

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

  if (options.EnableObjCInterop == toolchain::cl::BOU_UNSET) {
    Invocation.getLangOptions().EnableObjCInterop =
        Invocation.getLangOptions().Target.isOSDarwin();
  } else {
    Invocation.getLangOptions().EnableObjCInterop =
    options.EnableObjCInterop == toolchain::cl::BOU_TRUE;
  }

  SILOptions &Opts = Invocation.getSILOptions();
  Opts.EmitVerboseSIL = options.EmitVerboseSIL;
  Opts.EmitSortedSIL = options.EmitSortedSIL;
  Opts.EnableOSSAModules = options.EnableOSSAModules;
  Opts.StopOptimizationAfterSerialization |= options.EmitSIB;

  serialization::ExtendedValidationInfo extendedInfo;
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> FileBufOrErr =
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
    toolchain::errs() << InstanceSetupError << '\n';
    return 1;
  }
  CI.performSema();

  // If parsing produced an error, don't run any passes.
  if (CI.getASTContext().hadError())
    return 1;

  auto SILMod = performASTLowering(CI.getMainModule(), CI.getSILTypes(),
                                   CI.getSILOptions());

  if (options.CommandLineFunctionNames.empty() && options.FunctionNameFile.empty())
    return CI.getASTContext().hadError();

  // For efficient usage, we separate our names into two separate sorted
  // lists, one of managled names, and one of unmangled names.
  std::vector<std::string> Names;
  getFunctionNames(Names, options);

  // First partition our function names into mangled/demangled arrays.
  auto FirstDemangledName = std::partition(
      Names.begin(), Names.end(), [](const std::string &Name) -> bool {
        StringRef NameRef(Name);
        return NameRef.starts_with("_T") ||
               NameRef.starts_with(MANGLING_PREFIX_STR);
      });

  // Then grab offsets to avoid any issues with iterator invalidation when we
  // sort.
  unsigned NumMangled = std::distance(Names.begin(), FirstDemangledName);
  unsigned NumNames = Names.size();

  // Then sort the two partitioned arrays.
  std::sort(Names.begin(), FirstDemangledName);
  std::sort(FirstDemangledName, Names.end());

  // Finally construct our ArrayRefs into the sorted std::vector for our
  // mangled and demangled names.
  ArrayRef<std::string> MangledNames(&*Names.begin(), NumMangled);
  ArrayRef<std::string> DemangledNames(&*std::next(Names.begin(), NumMangled),
                                       NumNames - NumMangled);

  TOOLCHAIN_DEBUG(toolchain::errs() << "MangledNames to keep:\n";
             std::for_each(MangledNames.begin(), MangledNames.end(),
                           [](const std::string &str) {
                             toolchain::errs() << "    " << str << '\n';
                           }));
  TOOLCHAIN_DEBUG(toolchain::errs() << "DemangledNames to keep:\n";
             std::for_each(DemangledNames.begin(), DemangledNames.end(),
                           [](const std::string &str) {
                             toolchain::errs() << "    " << str << '\n';
                           }));

  removeUnwantedFunctions(SILMod.get(), MangledNames, DemangledNames, options);

  if (options.EmitSIB) {
    toolchain::SmallString<128> OutputFile;
    if (options.OutputFilename.size()) {
      OutputFile = options.OutputFilename;
    } else if (options.ModuleName.size()) {
      OutputFile = options.ModuleName;
      toolchain::sys::path::replace_extension(
          OutputFile, file_types::getExtension(file_types::TY_SIB));
    } else {
      OutputFile = CI.getMainModule()->getName().str();
      toolchain::sys::path::replace_extension(
          OutputFile, file_types::getExtension(file_types::TY_SIB));
    }

    SerializationOptions serializationOpts;
    serializationOpts.OutputPath = OutputFile;
    serializationOpts.SerializeAllSIL = true;
    serializationOpts.IsSIB = true;
    serializationOpts.IsOSSA = options.EnableOSSAModules;

    symbolgraphgen::SymbolGraphOptions symbolGraphOpts;

    serialize(CI.getMainModule(), serializationOpts, symbolGraphOpts, SILMod.get());
  } else {
    const StringRef OutputFile =
    options.OutputFilename.size() ? StringRef(options.OutputFilename) : "-";

    auto SILOpts = SILOptions();
    SILOpts.EmitVerboseSIL = options.EmitVerboseSIL;
    SILOpts.EmitSortedSIL = options.EmitSortedSIL;

    if (OutputFile == "-") {
      SILMod->print(toolchain::outs(), CI.getMainModule(), SILOpts, !options.DisableASTDump);
    } else {
      std::error_code EC;
      toolchain::raw_fd_ostream OS(OutputFile, EC, toolchain::sys::fs::OF_None);
      if (EC) {
        toolchain::errs() << "while opening '" << OutputFile << "': " << EC.message()
                     << '\n';
        return 1;
      }
      SILMod->print(OS, CI.getMainModule(), SILOpts, !options.DisableASTDump);
    }
  }
  return 0;
}
