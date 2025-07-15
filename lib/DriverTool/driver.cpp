//===--- driver.cpp - Codira Compiler Driver -------------------------------===//
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
// This is the entry point to the language compiler driver.
//
//===----------------------------------------------------------------------===//

#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsDriver.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Basic/InitializeCodiraModules.h"
#include "language/Basic/PrettyStackTrace.h"
#include "language/Basic/Program.h"
#include "language/Basic/TaskQueue.h"
#include "language/Basic/SourceManager.h"
#include "language/Driver/Compilation.h"
#include "language/Driver/Driver.h"
#include "language/Driver/FrontendUtil.h"
#include "language/Driver/Job.h"
#include "language/Driver/ToolChain.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/FrontendTool/FrontendTool.h"
#include "language/DriverTool/DriverTool.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/ConvertUTF.h"
#include "toolchain/Support/Errno.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/TargetParser/Host.h"
#include "toolchain/Support/ManagedStatic.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/PrettyStackTrace.h"
#include "toolchain/Support/Process.h"
#include "toolchain/Support/Program.h"
#include "toolchain/Support/Signals.h"
#include "toolchain/Support/StringSaver.h"
#include "toolchain/Support/TargetSelect.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/TargetParser/Host.h"
#include "toolchain/TargetParser/Triple.h"

#include <csignal>
#include <memory>
#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#endif

using namespace language;
using namespace language::driver;

std::string getExecutablePath(const char *FirstArg) {
  void *P = (void *)(intptr_t)getExecutablePath;
  return toolchain::sys::fs::getMainExecutable(FirstArg, P);
}

/// Run 'sil-opt'
extern int sil_opt_main(ArrayRef<const char *> argv, void *MainAddr);

/// Run 'sil-fn-extractor'
extern int sil_func_extractor_main(ArrayRef<const char *> argv, void *MainAddr);

/// Run 'sil-nm'
extern int sil_nm_main(ArrayRef<const char *> argv, void *MainAddr);

/// Run 'sil-toolchain-gen'
extern int sil_toolchain_gen_main(ArrayRef<const char *> argv, void *MainAddr);

/// Run 'sil-passpipeline-dumper'
extern int sil_passpipeline_dumper_main(ArrayRef<const char *> argv, void *MainAddr);

/// Run 'language-dependency-tool'
extern int language_dependency_tool_main(ArrayRef<const char *> argv, void *MainAddr);

/// Run 'language-toolchain-opt'
extern int language_toolchain_opt_main(ArrayRef<const char *> argv, void *MainAddr);

/// Run 'language-autolink-extract'.
extern int autolink_extract_main(ArrayRef<const char *> Args, const char *Argv0,
                                 void *MainAddr);

extern int modulewrap_main(ArrayRef<const char *> Args, const char *Argv0,
                           void *MainAddr);

/// Run 'language-symbolgraph-extract'
extern int language_symbolgraph_extract_main(ArrayRef<const char *> Args, const char *Argv0,
void *MainAddr);

/// Run 'language-synthesize-interface'
extern int language_synthesize_interface_main(ArrayRef<const char *> Args,
                                           const char *Argv0, void *MainAddr);

/// Run 'language-api-digester'
extern int language_api_digester_main(ArrayRef<const char *> Args,
                                   const char *Argv0, void *MainAddr);

/// Run 'language-cache-tool'
extern int language_cache_tool_main(ArrayRef<const char *> Args, const char *Argv0,
                                 void *MainAddr);

/// Run 'language-parse-test'
extern int language_parse_test_main(ArrayRef<const char *> Args, const char *Argv0,
                                 void *MainAddr);

/// Determine if the given invocation should run as a "subcommand".
///
/// Examples of "subcommands" are 'language build' or 'language test', which are
/// usually used to invoke the Codira package manager executables 'language-build'
/// and 'language-test', respectively.
///
/// \param ExecName The name of the argv[0] we were invoked as.
/// \param SubcommandName On success, the full name of the subcommand to invoke.
/// \param Args On return, the adjusted program arguments to use.
/// \returns True if running as a subcommand.
static bool shouldRunAsSubcommand(StringRef ExecName,
                                  SmallString<256> &SubcommandName,
                                  const ArrayRef<const char *> Args) {
  assert(!Args.empty());

  // If we are not run as 'language', don't do anything special. This doesn't work
  // with symlinks with alternate names, but we can't detect 'language' vs 'languagec'
  // if we try and resolve using the actual executable path.
  if (ExecName != "language" && ExecName != "language-legacy-driver")
    return false;

  // If there are no program arguments, always invoke as normal.
  if (Args.size() == 1)
    return false;

  // Otherwise, we have a program argument. If it looks like an option or a
  // path, then invoke in interactive mode with the arguments as given.
  StringRef FirstArg(Args[1]);
  if (FirstArg.starts_with("-") || FirstArg.contains('.') ||
      FirstArg.contains('/'))
    return false;

  // Otherwise, we should have some sort of subcommand. Get the subcommand name
  // and remove it from the program arguments.
  StringRef Subcommand = Args[1];

  // If the subcommand is the "built-in" 'repl', then use the
  // normal driver.
  if (Subcommand == "repl") {
    return false;
  }

  // Form the subcommand name.
  SubcommandName.assign("language-");
  SubcommandName.append(Subcommand);

  return true;
}

static bool shouldDisallowNewDriver(DiagnosticEngine &diags,
                                    StringRef ExecName,
                                    const ArrayRef<const char *> argv) {
  // We are expected to use the legacy driver to `exec` an overload
  // for testing purposes.
  if (toolchain::sys::Process::GetEnv("LANGUAGE_OVERLOAD_DRIVER").has_value()) {
    return false;
  }
  StringRef disableArg = "-disallow-use-new-driver";
  StringRef disableEnv = "LANGUAGE_USE_OLD_DRIVER";
  auto shouldWarn = !toolchain::sys::Process::
    GetEnv("LANGUAGE_AVOID_WARNING_USING_OLD_DRIVER").has_value();

  // We explicitly are on the fallback to the legacy driver from the new driver.
  // Do not forward.
  if (ExecName == "language-legacy-driver" ||
      ExecName == "languagec-legacy-driver") {
    if (shouldWarn)
      diags.diagnose(SourceLoc(), diag::old_driver_deprecated, disableArg);
    return true;
  }

  // We are not invoking the driver, so don't forward.
  if (ExecName != "language" && ExecName != "languagec") {
    return true;
  }

  // If user specified using the old driver, don't forward.
  if (toolchain::find_if(argv, [&](const char* arg) {
    return StringRef(arg) == disableArg;
  }) != argv.end()) {
    if (shouldWarn)
      diags.diagnose(SourceLoc(), diag::old_driver_deprecated, disableArg);
    return true;
  }
  if (toolchain::sys::Process::GetEnv(disableEnv).has_value()) {
    if (shouldWarn)
      diags.diagnose(SourceLoc(), diag::old_driver_deprecated, disableEnv);
    return true;
  }
  return false;
}

static bool appendCodiraDriverName(SmallString<256> &buffer) {
  assert(toolchain::sys::fs::exists(buffer));
  if (auto driverNameOp = toolchain::sys::Process::GetEnv("LANGUAGE_OVERLOAD_DRIVER")) {
    toolchain::sys::path::append(buffer, *driverNameOp);
    return true;
  }

  StringRef execSuffix(toolchain::Triple(toolchain::sys::getProcessTriple()).isOSWindows() ? ".exe" : "");
  toolchain::sys::path::append(buffer, "language-driver" + execSuffix);
  if (toolchain::sys::fs::exists(buffer)) {
    return true;
  }
  toolchain::sys::path::remove_filename(buffer);
  toolchain::sys::path::append(buffer, "language-driver-new" + execSuffix);
  if (toolchain::sys::fs::exists(buffer)) {
    return true;
  }

  return false;
}

static toolchain::SmallVector<const char *, 32> eraseFirstArg(ArrayRef<const char *> argv){
  toolchain::SmallVector<const char *, 32> newArgv;
  newArgv.push_back(argv[0]);
  for (const char *arg : argv.slice(2)) {
    newArgv.push_back(arg);
  }
  return newArgv;
}

static int run_driver(StringRef ExecName,
                       ArrayRef<const char *> argv,
                       const ArrayRef<const char *> originalArgv) {
  // This is done here and not done in FrontendTool.cpp, because
  // FrontendTool.cpp is linked to tools, which don't use language modules.
  initializeCodiraModules();

  bool isRepl = false;

  // Handle integrated tools.
  StringRef DriverModeArg;
  if (argv.size() > 1) {
    StringRef FirstArg(argv[1]);

    if (FirstArg == "-frontend") {
      return performFrontend(
          toolchain::ArrayRef(argv.data() + 2, argv.data() + argv.size()), argv[0],
          (void *)(intptr_t)getExecutablePath);
    }
    if (FirstArg == "-modulewrap") {
      return modulewrap_main(
          toolchain::ArrayRef(argv.data() + 2, argv.data() + argv.size()), argv[0],
          (void *)(intptr_t)getExecutablePath);
    }
    if (FirstArg == "-sil-opt") {
      return sil_opt_main(eraseFirstArg(argv),
                          (void *)(intptr_t)getExecutablePath);
    }
    if (FirstArg == "-sil-fn-extractor") {
      return sil_func_extractor_main(eraseFirstArg(argv),
                                     (void *)(intptr_t)getExecutablePath);
    }
    if (FirstArg == "-sil-nm") {
      return sil_nm_main(eraseFirstArg(argv),
                         (void *)(intptr_t)getExecutablePath);
    }
    if (FirstArg == "-sil-toolchain-gen") {
      return sil_toolchain_gen_main(eraseFirstArg(argv),
                               (void *)(intptr_t)getExecutablePath);
    }
    if (FirstArg == "-sil-passpipeline-dumper") {
      return sil_passpipeline_dumper_main(eraseFirstArg(argv),
                                          (void *)(intptr_t)getExecutablePath);
    }
    if (FirstArg == "-language-dependency-tool") {
      return language_dependency_tool_main(eraseFirstArg(argv),
                                        (void *)(intptr_t)getExecutablePath);
    }
    if (FirstArg == "-language-toolchain-opt") {
      return language_toolchain_opt_main(eraseFirstArg(argv),
                                 (void *)(intptr_t)getExecutablePath);
    }

    // Run the integrated Codira frontend when called as "language-frontend" but
    // without a leading "-frontend".
    if (!FirstArg.starts_with("--driver-mode=")
        && ExecName == "language-frontend") {
      return performFrontend(
          toolchain::ArrayRef(argv.data() + 1, argv.data() + argv.size()), argv[0],
          (void *)(intptr_t)getExecutablePath);
    }

    if (FirstArg == "repl") {
      isRepl = true;
      argv = argv.drop_front();
    } else if (FirstArg.starts_with("--driver-mode=")) {
      DriverModeArg = FirstArg;
    }
  }

  std::string Path = getExecutablePath(argv[0]);

  PrintingDiagnosticConsumer PDC;

  SourceManager SM;
  DiagnosticEngine Diags(SM);
  Diags.addConsumer(PDC);

  // Forwarding calls to the language driver if the C++ driver is invoked as `language`
  // or `languagec`, and an environment variable LANGUAGE_USE_NEW_DRIVER is defined.
  if (!shouldDisallowNewDriver(Diags, ExecName, argv)) {
    SmallString<256> NewDriverPath(toolchain::sys::path::parent_path(Path));
    if (appendCodiraDriverName(NewDriverPath) &&
        toolchain::sys::fs::exists(NewDriverPath)) {
      std::vector<const char *> subCommandArgs;
      // Rewrite the program argument.
      subCommandArgs.push_back(NewDriverPath.c_str());
      if (!DriverModeArg.empty()) {
        subCommandArgs.push_back(DriverModeArg.data());
      } else if (ExecName == "languagec") {
        subCommandArgs.push_back("--driver-mode=languagec");
      } else if (ExecName == "language") {
        subCommandArgs.push_back("--driver-mode=language");
      }
      // Push these non-op frontend arguments so the build log can indicate
      // the new driver is used.
      subCommandArgs.push_back("-Xfrontend");
      subCommandArgs.push_back("-new-driver-path");
      subCommandArgs.push_back("-Xfrontend");
      subCommandArgs.push_back(NewDriverPath.c_str());

      // Push on the source program arguments
      if (isRepl) {
        subCommandArgs.push_back("-repl");
        subCommandArgs.insert(subCommandArgs.end(),
                              originalArgv.begin() + 2, originalArgv.end());
      } else {
        subCommandArgs.insert(subCommandArgs.end(),
                              originalArgv.begin() + 1, originalArgv.end());
      }

      // Execute the subcommand.
      subCommandArgs.push_back(nullptr);
      ExecuteInPlace(NewDriverPath.c_str(), subCommandArgs.data());

      // If we reach here then an error occurred (typically a missing path).
      std::string ErrorString = toolchain::sys::StrError();
      toolchain::errs() << "error: unable to invoke subcommand: " << subCommandArgs[0]
                   << " (" << ErrorString << ")\n";
      return 2;
    } else
      Diags.diagnose(SourceLoc(), diag::new_driver_not_found, NewDriverPath);
  }
  
  // We are in the fallback to legacy driver mode.
  // Now that we have determined above that we are not going to
  // forward the invocation to the new driver, ensure the rest of the
  // downstream driver execution refers to itself by the appropriate name.
  if (ExecName == "language-legacy-driver")
    ExecName = "language";
  else if (ExecName == "languagec-legacy-driver")
    ExecName = "languagec";
  
  Driver TheDriver(Path, ExecName, argv, Diags);
  switch (TheDriver.getDriverKind()) {
  case Driver::DriverKind::SILOpt:
    return sil_opt_main(argv, (void *)(intptr_t)getExecutablePath);
  case Driver::DriverKind::SILFuncExtractor:
    return sil_func_extractor_main(argv, (void *)(intptr_t)getExecutablePath);
  case Driver::DriverKind::SILNM:
    return sil_nm_main(argv, (void *)(intptr_t)getExecutablePath);
  case Driver::DriverKind::SILLLVMGen:
    return sil_toolchain_gen_main(argv, (void *)(intptr_t)getExecutablePath);
  case Driver::DriverKind::SILPassPipelineDumper:
    return sil_passpipeline_dumper_main(argv, (void *)(intptr_t)getExecutablePath);
  case Driver::DriverKind::CodiraDependencyTool:
    return language_dependency_tool_main(argv, (void *)(intptr_t)getExecutablePath);
  case Driver::DriverKind::CodiraLLVMOpt:
    return language_toolchain_opt_main(argv, (void *)(intptr_t)getExecutablePath);
  case Driver::DriverKind::AutolinkExtract:
    return autolink_extract_main(
      TheDriver.getArgsWithoutProgramNameAndDriverMode(argv),
      argv[0], (void *)(intptr_t)getExecutablePath);
  case Driver::DriverKind::SymbolGraph:
      return language_symbolgraph_extract_main(TheDriver.getArgsWithoutProgramNameAndDriverMode(argv), argv[0], (void *)(intptr_t)getExecutablePath);
  case Driver::DriverKind::SynthesizeInterface:
    return language_synthesize_interface_main(
        TheDriver.getArgsWithoutProgramNameAndDriverMode(argv), argv[0],
        (void *)(intptr_t)getExecutablePath);
  case Driver::DriverKind::APIDigester:
    return language_api_digester_main(
        TheDriver.getArgsWithoutProgramNameAndDriverMode(argv), argv[0],
        (void *)(intptr_t)getExecutablePath);
  case Driver::DriverKind::CacheTool:
    return language_cache_tool_main(
        TheDriver.getArgsWithoutProgramNameAndDriverMode(argv), argv[0],
        (void *)(intptr_t)getExecutablePath);
  case Driver::DriverKind::ParseTest:
    return language_parse_test_main(argv, argv[0],
                                 (void *)(intptr_t)getExecutablePath);
  default:
    break;
  }

  std::unique_ptr<toolchain::opt::InputArgList> ArgList =
    TheDriver.parseArgStrings(ArrayRef<const char*>(argv).slice(1));
  if (Diags.hadAnyError())
    return 1;

  std::unique_ptr<ToolChain> TC = TheDriver.buildToolChain(*ArgList);
  if (Diags.hadAnyError())
    return 1;

  for (auto arg: ArgList->getArgs()) {
    if (arg->getOption().hasFlag(options::NewDriverOnlyOption)) {
      Diags.diagnose(SourceLoc(), diag::warning_unsupported_driver_option,
                     arg->getSpelling());
    }
  }

  std::unique_ptr<Compilation> C =
      TheDriver.buildCompilation(*TC, std::move(ArgList));

  if (Diags.hadAnyError())
    return 1;

  if (C) {
    std::unique_ptr<sys::TaskQueue> TQ = TheDriver.buildTaskQueue(*C);
    if (!TQ)
        return 1;
    return C->performJobs(std::move(TQ)).exitCode;
  }

  return 0;
}

int language::mainEntry(int argc_, const char **argv_) {
#if defined(_WIN32)
  LPWSTR *wargv_ = CommandLineToArgvW(GetCommandLineW(), &argc_);
  std::vector<std::string> utf8Args;
  // We use UTF-8 as the internal character encoding. On Windows,
  // arguments passed to wmain are encoded in UTF-16
  for (int i = 0; i < argc_; i++) {
    const wchar_t *wideArg = wargv_[i];
    int wideArgLen = std::wcslen(wideArg);
    utf8Args.push_back("");
    toolchain::ArrayRef<char> uRef((const char *)wideArg,
                              (const char *)(wideArg + wideArgLen));
    toolchain::convertUTF16ToUTF8String(uRef, utf8Args[i]);
  }

  std::vector<const char *> utf8CStrs;
  toolchain::transform(utf8Args, std::back_inserter(utf8CStrs),
                  std::mem_fn(&std::string::c_str));
  argv_ = utf8CStrs.data();
#else
  // Set SIGINT to the default handler, ensuring we exit. This needs to be set
  // before PROGRAM_START/INITIALIZE_LLVM since LLVM will set its own signal
  // handler that does some cleanup before delegating to the original handler.
  std::signal(SIGINT, SIG_DFL);
#endif
  // Expand any response files in the command line argument vector - arguments
  // may be passed through response files in the event of command line length
  // restrictions.
  SmallVector<const char *, 256> ExpandedArgs(&argv_[0], &argv_[argc_]);
  toolchain::BumpPtrAllocator Allocator;
  toolchain::StringSaver Saver(Allocator);
  language::driver::ExpandResponseFilesWithRetry(Saver, ExpandedArgs);

  // Initialize the stack trace using the parsed argument vector with expanded
  // response files.

  // PROGRAM_START/InitLLVM overwrites the passed in arguments with UTF-8
  // versions of them on Windows. This also has the effect of overwriting the
  // response file expansion. Since we handle the UTF-8 conversion above, we
  // pass in a copy and throw away the modifications.
  int ThrowawayExpandedArgc = ExpandedArgs.size();
  const char **ThrowawayExpandedArgv = ExpandedArgs.data();
  PROGRAM_START(ThrowawayExpandedArgc, ThrowawayExpandedArgv);
  ArrayRef<const char *> argv(ExpandedArgs);

  PrettyStackTraceCodiraVersion versionStackTrace;

  // Check if this invocation should execute a subcommand.
  StringRef ExecName = toolchain::sys::path::stem(argv[0]);
  SmallString<256> SubcommandName;
  if (shouldRunAsSubcommand(ExecName, SubcommandName, argv)) {
    // Preserve argv for the stack trace.
    SmallVector<const char *, 256> subCommandArgs(argv.begin(), argv.end());
    subCommandArgs.erase(&subCommandArgs[1]);
    // We are running as a subcommand, try to find the subcommand adjacent to
    // the executable we are running as.
    SmallString<256> SubcommandPath(SubcommandName);
    auto result = toolchain::sys::findProgramByName(SubcommandName,
      { toolchain::sys::path::parent_path(getExecutablePath(argv[0])) });
    if (!result.getError()) {
      SubcommandPath = *result;
    } else {
      // If we didn't find the tool there, let the OS search for it.
      result = toolchain::sys::findProgramByName(SubcommandName);
      // Search for the program and use the path if found. If there was an
      // error, ignore it and just let the exec fail.
      if (!result.getError())
        SubcommandPath = *result;
    }

    // Rewrite the program argument.
    subCommandArgs[0] = SubcommandPath.c_str();

    // Execute the subcommand.
    subCommandArgs.push_back(nullptr);
    ExecuteInPlace(SubcommandPath.c_str(), subCommandArgs.data());

    // If we reach here then an error occurred (typically a missing path).
    std::string ErrorString = toolchain::sys::StrError();
    toolchain::errs() << "error: unable to invoke subcommand: " << subCommandArgs[0]
                 << " (" << ErrorString << ")\n";
    return 2;
  }

  ArrayRef<const char *> originalArgv(argv_, &argv_[argc_]);
  return run_driver(ExecName, argv, originalArgv);
}
