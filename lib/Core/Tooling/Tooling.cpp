//===- Tooling.cpp - Running clang standalone tools -----------------------===//
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
//  This file implements functions to run clang tools standalone instead
//  of running them as a plugin.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Tooling/Tooling.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/DiagnosticIDs.h"
#include "language/Core/Basic/DiagnosticOptions.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/Basic/FileSystemOptions.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Job.h"
#include "language/Core/Driver/Options.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/ToolChain.h"
#include "language/Core/Frontend/ASTUnit.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Frontend/CompilerInvocation.h"
#include "language/Core/Frontend/FrontendDiagnostic.h"
#include "language/Core/Frontend/FrontendOptions.h"
#include "language/Core/Frontend/TextDiagnosticPrinter.h"
#include "language/Core/Lex/HeaderSearchOptions.h"
#include "language/Core/Lex/PreprocessorOptions.h"
#include "language/Core/Tooling/ArgumentsAdjusters.h"
#include "language/Core/Tooling/CompilationDatabase.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/Twine.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Option/OptTable.h"
#include "toolchain/Option/Option.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/TargetParser/Host.h"
#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#define DEBUG_TYPE "clang-tooling"

using namespace language::Core;
using namespace tooling;

ToolAction::~ToolAction() = default;

FrontendActionFactory::~FrontendActionFactory() = default;

// FIXME: This file contains structural duplication with other parts of the
// code that sets up a compiler to run tools on it, and we should refactor
// it to be based on the same framework.

/// Builds a clang driver initialized for running clang tools.
static driver::Driver *
newDriver(DiagnosticsEngine *Diagnostics, const char *BinaryName,
          IntrusiveRefCntPtr<toolchain::vfs::FileSystem> VFS) {
  driver::Driver *CompilerDriver =
      new driver::Driver(BinaryName, toolchain::sys::getDefaultTargetTriple(),
                         *Diagnostics, "clang LLVM compiler", std::move(VFS));
  CompilerDriver->setTitle("clang_based_tool");
  return CompilerDriver;
}

/// Decide whether extra compiler frontend commands can be ignored.
static bool ignoreExtraCC1Commands(const driver::Compilation *Compilation) {
  const driver::JobList &Jobs = Compilation->getJobs();
  const driver::ActionList &Actions = Compilation->getActions();

  bool OffloadCompilation = false;

  // Jobs and Actions look very different depending on whether the Clang tool
  // injected -fsyntax-only or not. Try to handle both cases here.

  for (const auto &Job : Jobs)
    if (StringRef(Job.getExecutable()) == "clang-offload-bundler")
      OffloadCompilation = true;

  if (Jobs.size() > 1) {
    for (auto *A : Actions){
      // On MacOSX real actions may end up being wrapped in BindArchAction
      if (isa<driver::BindArchAction>(A))
        A = *A->input_begin();
      if (isa<driver::OffloadAction>(A)) {
        // Offload compilation has 2 top-level actions, one (at the front) is
        // the original host compilation and the other is offload action
        // composed of at least one device compilation. For such case, general
        // tooling will consider host-compilation only. For tooling on device
        // compilation, device compilation only option, such as
        // `--cuda-device-only`, needs specifying.
        assert(Actions.size() > 1);
        assert(
            isa<driver::CompileJobAction>(Actions.front()) ||
            // On MacOSX real actions may end up being wrapped in
            // BindArchAction.
            (isa<driver::BindArchAction>(Actions.front()) &&
             isa<driver::CompileJobAction>(*Actions.front()->input_begin())));
        OffloadCompilation = true;
        break;
      }
    }
  }

  return OffloadCompilation;
}

namespace language::Core {
namespace tooling {

const toolchain::opt::ArgStringList *
getCC1Arguments(DiagnosticsEngine *Diagnostics,
                driver::Compilation *Compilation) {
  const driver::JobList &Jobs = Compilation->getJobs();

  auto IsCC1Command = [](const driver::Command &Cmd) {
    return StringRef(Cmd.getCreator().getName()) == "clang";
  };

  auto IsSrcFile = [](const driver::InputInfo &II) {
    return isSrcFile(II.getType());
  };

  toolchain::SmallVector<const driver::Command *, 1> CC1Jobs;
  for (const driver::Command &Job : Jobs)
    if (IsCC1Command(Job) && toolchain::all_of(Job.getInputInfos(), IsSrcFile))
      CC1Jobs.push_back(&Job);

  // If there are no jobs for source files, try checking again for a single job
  // with any file type. This accepts a preprocessed file as input.
  if (CC1Jobs.empty())
    for (const driver::Command &Job : Jobs)
      if (IsCC1Command(Job))
        CC1Jobs.push_back(&Job);

  if (CC1Jobs.empty() ||
      (CC1Jobs.size() > 1 && !ignoreExtraCC1Commands(Compilation))) {
    SmallString<256> error_msg;
    toolchain::raw_svector_ostream error_stream(error_msg);
    Jobs.Print(error_stream, "; ", true);
    Diagnostics->Report(diag::err_fe_expected_compiler_job)
        << error_stream.str();
    return nullptr;
  }

  return &CC1Jobs[0]->getArguments();
}

/// Returns a clang build invocation initialized from the CC1 flags.
CompilerInvocation *newInvocation(DiagnosticsEngine *Diagnostics,
                                  ArrayRef<const char *> CC1Args,
                                  const char *const BinaryName) {
  assert(!CC1Args.empty() && "Must at least contain the program name!");
  CompilerInvocation *Invocation = new CompilerInvocation;
  CompilerInvocation::CreateFromArgs(*Invocation, CC1Args, *Diagnostics,
                                     BinaryName);
  Invocation->getFrontendOpts().DisableFree = false;
  Invocation->getCodeGenOpts().DisableFree = false;
  return Invocation;
}

bool runToolOnCode(std::unique_ptr<FrontendAction> ToolAction,
                   const Twine &Code, const Twine &FileName,
                   std::shared_ptr<PCHContainerOperations> PCHContainerOps) {
  return runToolOnCodeWithArgs(std::move(ToolAction), Code,
                               std::vector<std::string>(), FileName,
                               "clang-tool", std::move(PCHContainerOps));
}

} // namespace tooling
} // namespace language::Core

static std::vector<std::string>
getSyntaxOnlyToolArgs(const Twine &ToolName,
                      const std::vector<std::string> &ExtraArgs,
                      StringRef FileName) {
  std::vector<std::string> Args;
  Args.push_back(ToolName.str());
  Args.push_back("-fsyntax-only");
  toolchain::append_range(Args, ExtraArgs);
  Args.push_back(FileName.str());
  return Args;
}

namespace language::Core {
namespace tooling {

bool runToolOnCodeWithArgs(
    std::unique_ptr<FrontendAction> ToolAction, const Twine &Code,
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> VFS,
    const std::vector<std::string> &Args, const Twine &FileName,
    const Twine &ToolName,
    std::shared_ptr<PCHContainerOperations> PCHContainerOps) {
  SmallString<16> FileNameStorage;
  StringRef FileNameRef = FileName.toNullTerminatedStringRef(FileNameStorage);

  toolchain::IntrusiveRefCntPtr<FileManager> Files =
      toolchain::makeIntrusiveRefCnt<FileManager>(FileSystemOptions(), VFS);
  ArgumentsAdjuster Adjuster = getClangStripDependencyFileAdjuster();
  ToolInvocation Invocation(
      getSyntaxOnlyToolArgs(ToolName, Adjuster(Args, FileNameRef), FileNameRef),
      std::move(ToolAction), Files.get(), std::move(PCHContainerOps));
  return Invocation.run();
}

bool runToolOnCodeWithArgs(
    std::unique_ptr<FrontendAction> ToolAction, const Twine &Code,
    const std::vector<std::string> &Args, const Twine &FileName,
    const Twine &ToolName,
    std::shared_ptr<PCHContainerOperations> PCHContainerOps,
    const FileContentMappings &VirtualMappedFiles) {
  auto OverlayFileSystem =
      toolchain::makeIntrusiveRefCnt<toolchain::vfs::OverlayFileSystem>(
          toolchain::vfs::getRealFileSystem());
  auto InMemoryFileSystem =
      toolchain::makeIntrusiveRefCnt<toolchain::vfs::InMemoryFileSystem>();
  OverlayFileSystem->pushOverlay(InMemoryFileSystem);

  SmallString<1024> CodeStorage;
  InMemoryFileSystem->addFile(FileName, 0,
                              toolchain::MemoryBuffer::getMemBuffer(
                                  Code.toNullTerminatedStringRef(CodeStorage)));

  for (auto &FilenameWithContent : VirtualMappedFiles) {
    InMemoryFileSystem->addFile(
        FilenameWithContent.first, 0,
        toolchain::MemoryBuffer::getMemBuffer(FilenameWithContent.second));
  }

  return runToolOnCodeWithArgs(std::move(ToolAction), Code, OverlayFileSystem,
                               Args, FileName, ToolName);
}

toolchain::Expected<std::string> getAbsolutePath(toolchain::vfs::FileSystem &FS,
                                            StringRef File) {
  StringRef RelativePath(File);
  // FIXME: Should '.\\' be accepted on Win32?
  RelativePath.consume_front("./");

  SmallString<1024> AbsolutePath = RelativePath;
  if (auto EC = FS.makeAbsolute(AbsolutePath))
    return toolchain::errorCodeToError(EC);
  toolchain::sys::path::native(AbsolutePath);
  return std::string(AbsolutePath);
}

std::string getAbsolutePath(StringRef File) {
  return toolchain::cantFail(getAbsolutePath(*toolchain::vfs::getRealFileSystem(), File));
}

void addTargetAndModeForProgramName(std::vector<std::string> &CommandLine,
                                    StringRef InvokedAs) {
  if (CommandLine.empty() || InvokedAs.empty())
    return;
  const auto &Table = driver::getDriverOptTable();
  // --target=X
  StringRef TargetOPT =
      Table.getOption(driver::options::OPT_target).getPrefixedName();
  // -target X
  StringRef TargetOPTLegacy =
      Table.getOption(driver::options::OPT_target_legacy_spelling)
          .getPrefixedName();
  // --driver-mode=X
  StringRef DriverModeOPT =
      Table.getOption(driver::options::OPT_driver_mode).getPrefixedName();
  auto TargetMode =
      driver::ToolChain::getTargetAndModeFromProgramName(InvokedAs);
  // No need to search for target args if we don't have a target/mode to insert.
  bool ShouldAddTarget = TargetMode.TargetIsValid;
  bool ShouldAddMode = TargetMode.DriverMode != nullptr;
  // Skip CommandLine[0].
  for (auto Token = ++CommandLine.begin(); Token != CommandLine.end();
       ++Token) {
    StringRef TokenRef(*Token);
    ShouldAddTarget = ShouldAddTarget && !TokenRef.starts_with(TargetOPT) &&
                      TokenRef != TargetOPTLegacy;
    ShouldAddMode = ShouldAddMode && !TokenRef.starts_with(DriverModeOPT);
  }
  if (ShouldAddMode) {
    CommandLine.insert(++CommandLine.begin(), TargetMode.DriverMode);
  }
  if (ShouldAddTarget) {
    CommandLine.insert(++CommandLine.begin(),
                       (TargetOPT + TargetMode.TargetPrefix).str());
  }
}

void addExpandedResponseFiles(std::vector<std::string> &CommandLine,
                              toolchain::StringRef WorkingDir,
                              toolchain::cl::TokenizerCallback Tokenizer,
                              toolchain::vfs::FileSystem &FS) {
  bool SeenRSPFile = false;
  toolchain::SmallVector<const char *, 20> Argv;
  Argv.reserve(CommandLine.size());
  for (auto &Arg : CommandLine) {
    Argv.push_back(Arg.c_str());
    if (!Arg.empty())
      SeenRSPFile |= Arg.front() == '@';
  }
  if (!SeenRSPFile)
    return;
  toolchain::BumpPtrAllocator Alloc;
  toolchain::cl::ExpansionContext ECtx(Alloc, Tokenizer);
  toolchain::Error Err =
      ECtx.setVFS(&FS).setCurrentDir(WorkingDir).expandResponseFiles(Argv);
  if (Err)
    toolchain::errs() << Err;
  // Don't assign directly, Argv aliases CommandLine.
  std::vector<std::string> ExpandedArgv(Argv.begin(), Argv.end());
  CommandLine = std::move(ExpandedArgv);
}

} // namespace tooling
} // namespace language::Core

namespace {

class SingleFrontendActionFactory : public FrontendActionFactory {
  std::unique_ptr<FrontendAction> Action;

public:
  SingleFrontendActionFactory(std::unique_ptr<FrontendAction> Action)
      : Action(std::move(Action)) {}

  std::unique_ptr<FrontendAction> create() override {
    return std::move(Action);
  }
};

} // namespace

ToolInvocation::ToolInvocation(
    std::vector<std::string> CommandLine, ToolAction *Action,
    FileManager *Files, std::shared_ptr<PCHContainerOperations> PCHContainerOps)
    : CommandLine(std::move(CommandLine)), Action(Action), OwnsAction(false),
      Files(Files), PCHContainerOps(std::move(PCHContainerOps)) {}

ToolInvocation::ToolInvocation(
    std::vector<std::string> CommandLine,
    std::unique_ptr<FrontendAction> FAction, FileManager *Files,
    std::shared_ptr<PCHContainerOperations> PCHContainerOps)
    : CommandLine(std::move(CommandLine)),
      Action(new SingleFrontendActionFactory(std::move(FAction))),
      OwnsAction(true), Files(Files),
      PCHContainerOps(std::move(PCHContainerOps)) {}

ToolInvocation::~ToolInvocation() {
  if (OwnsAction)
    delete Action;
}

bool ToolInvocation::run() {
  toolchain::opt::ArgStringList Argv;
  for (const std::string &Str : CommandLine)
    Argv.push_back(Str.c_str());
  const char *const BinaryName = Argv[0];

  // Parse diagnostic options from the driver command-line only if none were
  // explicitly set.
  std::unique_ptr<DiagnosticOptions> ParsedDiagOpts;
  DiagnosticOptions *DiagOpts = this->DiagOpts;
  if (!DiagOpts) {
    ParsedDiagOpts = CreateAndPopulateDiagOpts(Argv);
    DiagOpts = &*ParsedDiagOpts;
  }

  TextDiagnosticPrinter DiagnosticPrinter(toolchain::errs(), *DiagOpts);
  IntrusiveRefCntPtr<DiagnosticsEngine> Diagnostics =
      CompilerInstance::createDiagnostics(
          Files->getVirtualFileSystem(), *DiagOpts,
          DiagConsumer ? DiagConsumer : &DiagnosticPrinter, false);
  // Although `Diagnostics` are used only for command-line parsing, the custom
  // `DiagConsumer` might expect a `SourceManager` to be present.
  SourceManager SrcMgr(*Diagnostics, *Files);
  Diagnostics->setSourceManager(&SrcMgr);

  // We already have a cc1, just create an invocation.
  if (CommandLine.size() >= 2 && CommandLine[1] == "-cc1") {
    ArrayRef<const char *> CC1Args = ArrayRef(Argv).drop_front();
    std::unique_ptr<CompilerInvocation> Invocation(
        newInvocation(&*Diagnostics, CC1Args, BinaryName));
    if (Diagnostics->hasErrorOccurred())
      return false;
    return Action->runInvocation(std::move(Invocation), Files,
                                 std::move(PCHContainerOps), DiagConsumer);
  }

  const std::unique_ptr<driver::Driver> Driver(
      newDriver(&*Diagnostics, BinaryName, Files->getVirtualFileSystemPtr()));
  // The "input file not found" diagnostics from the driver are useful.
  // The driver is only aware of the VFS working directory, but some clients
  // change this at the FileManager level instead.
  // In this case the checks have false positives, so skip them.
  if (!Files->getFileSystemOpts().WorkingDir.empty())
    Driver->setCheckInputsExist(false);
  const std::unique_ptr<driver::Compilation> Compilation(
      Driver->BuildCompilation(toolchain::ArrayRef(Argv)));
  if (!Compilation)
    return false;
  const toolchain::opt::ArgStringList *const CC1Args = getCC1Arguments(
      &*Diagnostics, Compilation.get());
  if (!CC1Args)
    return false;
  std::unique_ptr<CompilerInvocation> Invocation(
      newInvocation(&*Diagnostics, *CC1Args, BinaryName));
  return runInvocation(BinaryName, Compilation.get(), std::move(Invocation),
                       std::move(PCHContainerOps));
}

bool ToolInvocation::runInvocation(
    const char *BinaryName, driver::Compilation *Compilation,
    std::shared_ptr<CompilerInvocation> Invocation,
    std::shared_ptr<PCHContainerOperations> PCHContainerOps) {
  // Show the invocation, with -v.
  if (Invocation->getHeaderSearchOpts().Verbose) {
    toolchain::errs() << "clang Invocation:\n";
    Compilation->getJobs().Print(toolchain::errs(), "\n", true);
    toolchain::errs() << "\n";
  }

  return Action->runInvocation(std::move(Invocation), Files,
                               std::move(PCHContainerOps), DiagConsumer);
}

bool FrontendActionFactory::runInvocation(
    std::shared_ptr<CompilerInvocation> Invocation, FileManager *Files,
    std::shared_ptr<PCHContainerOperations> PCHContainerOps,
    DiagnosticConsumer *DiagConsumer) {
  // Create a compiler instance to handle the actual work.
  CompilerInstance Compiler(std::move(Invocation), std::move(PCHContainerOps));
  Compiler.setFileManager(Files);

  // The FrontendAction can have lifetime requirements for Compiler or its
  // members, and we need to ensure it's deleted earlier than Compiler. So we
  // pass it to an std::unique_ptr declared after the Compiler variable.
  std::unique_ptr<FrontendAction> ScopedToolAction(create());

  // Create the compiler's actual diagnostics engine.
  Compiler.createDiagnostics(Files->getVirtualFileSystem(), DiagConsumer,
                             /*ShouldOwnClient=*/false);
  if (!Compiler.hasDiagnostics())
    return false;

  Compiler.createSourceManager(*Files);

  const bool Success = Compiler.ExecuteAction(*ScopedToolAction);

  Files->clearStatCache();
  return Success;
}

ClangTool::ClangTool(const CompilationDatabase &Compilations,
                     ArrayRef<std::string> SourcePaths,
                     std::shared_ptr<PCHContainerOperations> PCHContainerOps,
                     IntrusiveRefCntPtr<toolchain::vfs::FileSystem> BaseFS,
                     IntrusiveRefCntPtr<FileManager> Files)
    : Compilations(Compilations), SourcePaths(SourcePaths),
      PCHContainerOps(std::move(PCHContainerOps)),
      OverlayFileSystem(toolchain::makeIntrusiveRefCnt<toolchain::vfs::OverlayFileSystem>(
          std::move(BaseFS))),
      InMemoryFileSystem(
          toolchain::makeIntrusiveRefCnt<toolchain::vfs::InMemoryFileSystem>()),
      Files(Files ? Files
                  : toolchain::makeIntrusiveRefCnt<FileManager>(FileSystemOptions(),
                                                           OverlayFileSystem)) {
  OverlayFileSystem->pushOverlay(InMemoryFileSystem);
  appendArgumentsAdjuster(getClangStripOutputAdjuster());
  appendArgumentsAdjuster(getClangSyntaxOnlyAdjuster());
  appendArgumentsAdjuster(getClangStripDependencyFileAdjuster());
  if (Files)
    Files->setVirtualFileSystem(OverlayFileSystem);
}

ClangTool::~ClangTool() = default;

void ClangTool::mapVirtualFile(StringRef FilePath, StringRef Content) {
  MappedFileContents.push_back(std::make_pair(FilePath, Content));
}

void ClangTool::appendArgumentsAdjuster(ArgumentsAdjuster Adjuster) {
  ArgsAdjuster = combineAdjusters(std::move(ArgsAdjuster), std::move(Adjuster));
}

void ClangTool::clearArgumentsAdjusters() {
  ArgsAdjuster = nullptr;
}

static void injectResourceDir(CommandLineArguments &Args, const char *Argv0,
                              void *MainAddr) {
  // Allow users to override the resource dir.
  for (StringRef Arg : Args)
    if (Arg.starts_with("-resource-dir"))
      return;

  // If there's no override in place add our resource dir.
  Args = getInsertArgumentAdjuster(
      ("-resource-dir=" + CompilerInvocation::GetResourcesPath(Argv0, MainAddr))
          .c_str())(Args, "");
}

int ClangTool::run(ToolAction *Action) {
  // Exists solely for the purpose of lookup of the resource path.
  // This just needs to be some symbol in the binary.
  static int StaticSymbol;

  // First insert all absolute paths into the in-memory VFS. These are global
  // for all compile commands.
  if (SeenWorkingDirectories.insert("/").second)
    for (const auto &MappedFile : MappedFileContents)
      if (toolchain::sys::path::is_absolute(MappedFile.first))
        InMemoryFileSystem->addFile(
            MappedFile.first, 0,
            toolchain::MemoryBuffer::getMemBuffer(MappedFile.second));

  bool ProcessingFailed = false;
  bool FileSkipped = false;
  // Compute all absolute paths before we run any actions, as those will change
  // the working directory.
  std::vector<std::string> AbsolutePaths;
  AbsolutePaths.reserve(SourcePaths.size());
  for (const auto &SourcePath : SourcePaths) {
    auto AbsPath = getAbsolutePath(*OverlayFileSystem, SourcePath);
    if (!AbsPath) {
      toolchain::errs() << "Skipping " << SourcePath
                   << ". Error while getting an absolute path: "
                   << toolchain::toString(AbsPath.takeError()) << "\n";
      continue;
    }
    AbsolutePaths.push_back(std::move(*AbsPath));
  }

  // Remember the working directory in case we need to restore it.
  std::string InitialWorkingDir;
  if (auto CWD = OverlayFileSystem->getCurrentWorkingDirectory()) {
    InitialWorkingDir = std::move(*CWD);
  } else {
    toolchain::errs() << "Could not get working directory: "
                 << CWD.getError().message() << "\n";
  }

  size_t NumOfTotalFiles = AbsolutePaths.size();
  unsigned ProcessedFileCounter = 0;
  for (toolchain::StringRef File : AbsolutePaths) {
    // Currently implementations of CompilationDatabase::getCompileCommands can
    // change the state of the file system (e.g.  prepare generated headers), so
    // this method needs to run right before we invoke the tool, as the next
    // file may require a different (incompatible) state of the file system.
    //
    // FIXME: Make the compilation database interface more explicit about the
    // requirements to the order of invocation of its members.
    std::vector<CompileCommand> CompileCommandsForFile =
        Compilations.getCompileCommands(File);
    if (CompileCommandsForFile.empty()) {
      toolchain::errs() << "Skipping " << File << ". Compile command not found.\n";
      FileSkipped = true;
      continue;
    }
    for (CompileCommand &CompileCommand : CompileCommandsForFile) {
      // FIXME: chdir is thread hostile; on the other hand, creating the same
      // behavior as chdir is complex: chdir resolves the path once, thus
      // guaranteeing that all subsequent relative path operations work
      // on the same path the original chdir resulted in. This makes a
      // difference for example on network filesystems, where symlinks might be
      // switched during runtime of the tool. Fixing this depends on having a
      // file system abstraction that allows openat() style interactions.
      if (OverlayFileSystem->setCurrentWorkingDirectory(
              CompileCommand.Directory))
        toolchain::report_fatal_error("Cannot chdir into \"" +
                                 Twine(CompileCommand.Directory) + "\"!");

      // Now fill the in-memory VFS with the relative file mappings so it will
      // have the correct relative paths. We never remove mappings but that
      // should be fine.
      if (SeenWorkingDirectories.insert(CompileCommand.Directory).second)
        for (const auto &MappedFile : MappedFileContents)
          if (!toolchain::sys::path::is_absolute(MappedFile.first))
            InMemoryFileSystem->addFile(
                MappedFile.first, 0,
                toolchain::MemoryBuffer::getMemBuffer(MappedFile.second));

      std::vector<std::string> CommandLine = CompileCommand.CommandLine;
      if (ArgsAdjuster)
        CommandLine = ArgsAdjuster(CommandLine, CompileCommand.Filename);
      assert(!CommandLine.empty());

      // Add the resource dir based on the binary of this tool. argv[0] in the
      // compilation database may refer to a different compiler and we want to
      // pick up the very same standard library that compiler is using. The
      // builtin headers in the resource dir need to match the exact clang
      // version the tool is using.
      // FIXME: On linux, GetMainExecutable is independent of the value of the
      // first argument, thus allowing ClangTool and runToolOnCode to just
      // pass in made-up names here. Make sure this works on other platforms.
      injectResourceDir(CommandLine, "clang_tool", &StaticSymbol);

      // FIXME: We need a callback mechanism for the tool writer to output a
      // customized message for each file.
      if (NumOfTotalFiles > 1)
        toolchain::errs() << "[" + std::to_string(++ProcessedFileCounter) + "/" +
                            std::to_string(NumOfTotalFiles) +
                            "] Processing file " + File
                     << ".\n";
      ToolInvocation Invocation(std::move(CommandLine), Action, Files.get(),
                                PCHContainerOps);
      Invocation.setDiagnosticConsumer(DiagConsumer);

      if (!Invocation.run()) {
        // FIXME: Diagnostics should be used instead.
        if (PrintErrorMessage)
          toolchain::errs() << "Error while processing " << File << ".\n";
        ProcessingFailed = true;
      }
    }
  }

  if (!InitialWorkingDir.empty()) {
    if (auto EC =
            OverlayFileSystem->setCurrentWorkingDirectory(InitialWorkingDir))
      toolchain::errs() << "Error when trying to restore working dir: "
                   << EC.message() << "\n";
  }
  return ProcessingFailed ? 1 : (FileSkipped ? 2 : 0);
}

namespace {

class ASTBuilderAction : public ToolAction {
  std::vector<std::unique_ptr<ASTUnit>> &ASTs;

public:
  ASTBuilderAction(std::vector<std::unique_ptr<ASTUnit>> &ASTs) : ASTs(ASTs) {}

  bool runInvocation(std::shared_ptr<CompilerInvocation> Invocation,
                     FileManager *Files,
                     std::shared_ptr<PCHContainerOperations> PCHContainerOps,
                     DiagnosticConsumer *DiagConsumer) override {
    std::unique_ptr<ASTUnit> AST = ASTUnit::LoadFromCompilerInvocation(
        Invocation, std::move(PCHContainerOps), nullptr,
        CompilerInstance::createDiagnostics(Files->getVirtualFileSystem(),
                                            Invocation->getDiagnosticOpts(),
                                            DiagConsumer,
                                            /*ShouldOwnClient=*/false),
        Files);
    if (!AST)
      return false;

    ASTs.push_back(std::move(AST));
    return true;
  }
};

} // namespace

int ClangTool::buildASTs(std::vector<std::unique_ptr<ASTUnit>> &ASTs) {
  ASTBuilderAction Action(ASTs);
  return run(&Action);
}

void ClangTool::setPrintErrorMessage(bool PrintErrorMessage) {
  this->PrintErrorMessage = PrintErrorMessage;
}

namespace language::Core {
namespace tooling {

std::unique_ptr<ASTUnit>
buildASTFromCode(StringRef Code, StringRef FileName,
                 std::shared_ptr<PCHContainerOperations> PCHContainerOps) {
  return buildASTFromCodeWithArgs(Code, std::vector<std::string>(), FileName,
                                  "clang-tool", std::move(PCHContainerOps));
}

std::unique_ptr<ASTUnit> buildASTFromCodeWithArgs(
    StringRef Code, const std::vector<std::string> &Args, StringRef FileName,
    StringRef ToolName, std::shared_ptr<PCHContainerOperations> PCHContainerOps,
    ArgumentsAdjuster Adjuster, const FileContentMappings &VirtualMappedFiles,
    DiagnosticConsumer *DiagConsumer,
    IntrusiveRefCntPtr<toolchain::vfs::FileSystem> BaseFS) {
  std::vector<std::unique_ptr<ASTUnit>> ASTs;
  ASTBuilderAction Action(ASTs);
  auto OverlayFileSystem =
      toolchain::makeIntrusiveRefCnt<toolchain::vfs::OverlayFileSystem>(
          std::move(BaseFS));
  auto InMemoryFileSystem =
      toolchain::makeIntrusiveRefCnt<toolchain::vfs::InMemoryFileSystem>();
  OverlayFileSystem->pushOverlay(InMemoryFileSystem);
  toolchain::IntrusiveRefCntPtr<FileManager> Files =
      toolchain::makeIntrusiveRefCnt<FileManager>(FileSystemOptions(),
                                             OverlayFileSystem);

  ToolInvocation Invocation(
      getSyntaxOnlyToolArgs(ToolName, Adjuster(Args, FileName), FileName),
      &Action, Files.get(), std::move(PCHContainerOps));
  Invocation.setDiagnosticConsumer(DiagConsumer);

  InMemoryFileSystem->addFile(FileName, 0,
                              toolchain::MemoryBuffer::getMemBufferCopy(Code));
  for (auto &FilenameWithContent : VirtualMappedFiles) {
    InMemoryFileSystem->addFile(
        FilenameWithContent.first, 0,
        toolchain::MemoryBuffer::getMemBuffer(FilenameWithContent.second));
  }

  if (!Invocation.run())
    return nullptr;

  assert(ASTs.size() == 1);
  return std::move(ASTs[0]);
}

} // namespace tooling
} // namespace language::Core
