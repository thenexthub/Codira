//===--- language_synthesize_interface_main.cpp - Codira interface synthesis --===//
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
//
//  Prints the synthesized Codira interface for a Clang module.
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTPrinter.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Basic/Version.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/IDE/ModuleInterfacePrinting.h"
#include "language/Option/Options.h"
#include "language/Parse/ParseVersion.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;
using namespace options;

int language_synthesize_interface_main(ArrayRef<const char *> Args,
                                    const char *Argv0, void *MainAddr) {
  INITIALIZE_LLVM();

  CompilerInvocation Invocation;
  CompilerInstance CI;
  PrintingDiagnosticConsumer DiagPrinter;
  auto &Diags = CI.getDiags();
  Diags.addConsumer(DiagPrinter);

  std::unique_ptr<toolchain::opt::OptTable> Table = createCodiraOptTable();
  unsigned MissingIndex;
  unsigned MissingCount;
  toolchain::opt::InputArgList ParsedArgs = Table->ParseArgs(
      Args, MissingIndex, MissingCount, CodiraSynthesizeInterfaceOption);
  if (MissingCount) {
    Diags.diagnose(SourceLoc(), diag::error_missing_arg_value,
                   ParsedArgs.getArgString(MissingIndex), MissingCount);
    return EXIT_FAILURE;
  }

  if (ParsedArgs.hasArg(OPT_UNKNOWN)) {
    for (const auto *A : ParsedArgs.filtered(OPT_UNKNOWN)) {
      Diags.diagnose(SourceLoc(), diag::error_unknown_arg,
                     A->getAsString(ParsedArgs));
    }
    return EXIT_FAILURE;
  }

  auto MainExecutablePath = toolchain::sys::fs::getMainExecutable(Argv0, MainAddr);

  if (ParsedArgs.getLastArg(OPT_help) || Args.empty()) {
    std::string ExecutableName =
        toolchain::sys::path::stem(MainExecutablePath).str();
    Table->printHelp(toolchain::outs(), ExecutableName.c_str(),
                     "Codira Interface Synthesizer",
                     CodiraSynthesizeInterfaceOption, 0,
                     /*ShowAllAliases*/ false);
    return EXIT_FAILURE;
  }

  std::string ModuleName;
  if (auto *A = ParsedArgs.getLastArg(OPT_module_name)) {
    ModuleName = A->getValue();
  } else {
    Diags.diagnose(SourceLoc(), diag::error_option_required, "-module-name");
    return EXIT_FAILURE;
  }

  toolchain::Triple Target;
  if (auto *A = ParsedArgs.getLastArg(OPT_target)) {
    Target = toolchain::Triple(A->getValue());
  } else {
    Diags.diagnose(SourceLoc(), diag::error_option_required, "-target");
    return EXIT_FAILURE;
  }

  std::string OutputFile;
  if (auto *A = ParsedArgs.getLastArg(OPT_o)) {
    OutputFile = A->getValue();
  } else {
    OutputFile = "-";
  }

  Invocation.setMainExecutablePath(MainExecutablePath);
  Invocation.setModuleName("language_synthesize_interface");

  if (auto *A = ParsedArgs.getLastArg(OPT_resource_dir)) {
    Invocation.setRuntimeResourcePath(A->getValue());
  }

  std::string SDK = "";
  if (auto *A = ParsedArgs.getLastArg(OPT_sdk)) {
    SDK = A->getValue();
  }
  Invocation.setSDKPath(SDK);
  Invocation.setTargetTriple(Target);

  for (const auto *A : ParsedArgs.filtered(OPT_Xcc)) {
    Invocation.getClangImporterOptions().ExtraArgs.push_back(A->getValue());
  }

  std::vector<SearchPathOptions::SearchPath> FrameworkSearchPaths;
  for (const auto *A : ParsedArgs.filtered(OPT_F)) {
    FrameworkSearchPaths.push_back({A->getValue(), /*isSystem*/ false});
  }
  for (const auto *A : ParsedArgs.filtered(OPT_Fsystem)) {
    FrameworkSearchPaths.push_back({A->getValue(), /*isSystem*/ true});
  }
  Invocation.setFrameworkSearchPaths(FrameworkSearchPaths);
  Invocation.getSearchPathOptions().LibrarySearchPaths =
      ParsedArgs.getAllArgValues(OPT_L);
  std::vector<SearchPathOptions::SearchPath> ImportSearchPaths;
  for (const auto *A : ParsedArgs.filtered(OPT_I)) {
    ImportSearchPaths.push_back({A->getValue(), /*isSystem*/ false});
  }
  for (const auto *A : ParsedArgs.filtered(OPT_Isystem)) {
    ImportSearchPaths.push_back({A->getValue(), /*isSystem*/ true});
  }
  Invocation.setImportSearchPaths(ImportSearchPaths);

  Invocation.getLangOptions().EnableObjCInterop = Target.isOSDarwin();
  Invocation.getLangOptions().setCxxInteropFromArgs(ParsedArgs, Diags,
                                                    Invocation.getFrontendOptions());

  std::string ModuleCachePath = "";
  if (auto *A = ParsedArgs.getLastArg(OPT_module_cache_path)) {
    ModuleCachePath = A->getValue();
  }
  Invocation.setClangModuleCachePath(ModuleCachePath);
  Invocation.getClangImporterOptions().ModuleCachePath = ModuleCachePath;
  Invocation.getClangImporterOptions().ImportForwardDeclarations = true;
  Invocation.setDefaultPrebuiltCacheIfNecessary();

  if (auto *A = ParsedArgs.getLastArg(OPT_language_version)) {
    using version::Version;
    auto CodiraVersion = A->getValue();
    bool isValid = false;
    if (auto Version = VersionParser::parseVersionString(
            CodiraVersion, SourceLoc(), nullptr)) {
      if (auto Effective = Version.value().getEffectiveLanguageVersion()) {
        Invocation.getLangOptions().EffectiveLanguageVersion = *Effective;
        isValid = true;
      }
    }
    if (!isValid) {
      Diags.diagnose(SourceLoc(), diag::error_invalid_arg_value,
                     "-language-version", CodiraVersion);
      return EXIT_FAILURE;
    }
  }

  std::string InstanceSetupError;
  if (CI.setup(Invocation, InstanceSetupError)) {
    toolchain::outs() << InstanceSetupError << '\n';
    return EXIT_FAILURE;
  }

  auto M = CI.getASTContext().getModuleByName(ModuleName);
  if (!M) {
    toolchain::errs() << "Couldn't load module '" << ModuleName << '\''
                 << " in the current SDK and search paths.\n";

    SmallVector<Identifier, 32> VisibleModuleNames;
    CI.getASTContext().getVisibleTopLevelModuleNames(VisibleModuleNames);

    if (VisibleModuleNames.empty()) {
      toolchain::errs() << "Could not find any modules.\n";
    } else {
      std::sort(VisibleModuleNames.begin(), VisibleModuleNames.end(),
                [](const Identifier &A, const Identifier &B) -> bool {
                  return A.str() < B.str();
                });
      toolchain::errs() << "Current visible modules:\n";
      for (const auto &ModuleName : VisibleModuleNames) {
        toolchain::errs() << ModuleName.str() << "\n";
      }
    }
    return EXIT_FAILURE;
  }

  if (M->failedToLoad()) {
    toolchain::errs() << "Error: Failed to load the module '" << ModuleName
                 << "'. Are you missing build dependencies or "
                    "include/framework directories?\n"
                 << "See the previous error messages for details. Aborting.\n";

    return EXIT_FAILURE;
  }

  std::error_code EC;
  toolchain::raw_fd_ostream fs(OutputFile, EC);
  if (EC) {
    toolchain::errs() << "Cannot open output file: " << OutputFile << "\n";
    return EXIT_FAILURE;
  }

  PrintOptions printOpts =
      PrintOptions::printModuleInterface(/*printFullConvention=*/true);
  if (ParsedArgs.hasArg(OPT_print_fully_qualified_types)) {
    printOpts.FullyQualifiedTypes = true;
  }

  language::OptionSet<language::ide::ModuleTraversal> traversalOpts = std::nullopt;
  if (ParsedArgs.hasArg(OPT_include_submodules)) {
    traversalOpts = language::ide::ModuleTraversal::VisitSubmodules;
  }

  StreamPrinter printer(fs);
  ide::printModuleInterface(M, /*GroupNames=*/{}, traversalOpts, printer,
                            printOpts, /*PrintSynthesizedExtensions=*/false);

  return EXIT_SUCCESS;
}
