//===--- Immediate.cpp - the language immediate mode -------------------------===//
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
// This is the implementation of the language interpreter, which takes a
// source file and JITs it.
//
//===----------------------------------------------------------------------===//

#include "language/Immediate/Immediate.h"
#include "ImmediateImpl.h"
#include "language/Immediate/CodiraMaterializationUnit.h"

#include "language/AST/ASTContext.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/IRGenOptions.h"
#include "language/AST/IRGenRequests.h"
#include "language/AST/Module.h"
#include "language/AST/SILGenRequests.h"
#include "language/AST/TBDGenRequests.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Toolchain.h"
#include "language/Frontend/Frontend.h"
#include "language/IRGen/IRGenPublic.h"
#include "language/Runtime/Config.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/Subsystems.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Config/config.h"
#include "toolchain/ExecutionEngine/Orc/DebugUtils.h"
#include "toolchain/ExecutionEngine/Orc/EPCIndirectionUtils.h"
#include "toolchain/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "toolchain/ExecutionEngine/Orc/LLJIT.h"
#include "toolchain/ExecutionEngine/Orc/ObjectTransformLayer.h"
#include "toolchain/ExecutionEngine/Orc/TargetProcess/TargetExecutionUtils.h"
#include "toolchain/IR/LLVMContext.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Transforms/IPO.h"

// TODO: Replace pass manager
//       Removed in: d623b2f95fd559901f008a0588dddd0949a8db01
/* #include "toolchain/Transforms/IPO/PassManagerBuilder.h" */

#define DEBUG_TYPE "language-immediate"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using namespace language;
using namespace language::immediate;

static void *loadRuntimeLib(StringRef runtimeLibPathWithName) {
#if defined(_WIN32)
  return LoadLibraryA(runtimeLibPathWithName.str().c_str());
#else
  return dlopen(runtimeLibPathWithName.str().c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif
}

static void *loadRuntimeLibAtPath(StringRef sharedLibName,
                                  StringRef runtimeLibPath) {
  // FIXME: Need error-checking.
  toolchain::SmallString<128> Path = runtimeLibPath;
  toolchain::sys::path::append(Path, sharedLibName);
  return loadRuntimeLib(Path);
}

static void *loadRuntimeLib(StringRef sharedLibName,
                            ArrayRef<std::string> runtimeLibPaths) {
  for (auto &runtimeLibPath : runtimeLibPaths) {
    if (void *handle = loadRuntimeLibAtPath(sharedLibName, runtimeLibPath))
      return handle;
  }
  return nullptr;
}

void *language::immediate::loadCodiraRuntime(ArrayRef<std::string>
                                         runtimeLibPaths) {
#if defined(_WIN32)
  return loadRuntimeLib("languageCore" LTDL_SHLIB_EXT, runtimeLibPaths);
#elif (defined(__linux__) || defined(_WIN64) || defined(__FreeBSD__))
  return loadRuntimeLib("liblanguageCore" LTDL_SHLIB_EXT, runtimeLibPaths);
#else
  return loadRuntimeLib("liblanguageCore" LTDL_SHLIB_EXT, {"/usr/lib/language"});
#endif
}

static bool tryLoadLibrary(LinkLibrary linkLib,
                           SearchPathOptions searchPathOpts) {
  toolchain::SmallString<128> path = linkLib.getName();

  // If we have an absolute or relative path, just try to load it now.
  if (toolchain::sys::path::has_parent_path(path.str())) {
    return loadRuntimeLib(path);
  }

  bool success = false;
  switch (linkLib.getKind()) {
  case LibraryKind::Library: {
    toolchain::SmallString<32> stem;
    if (toolchain::sys::path::has_extension(path.str())) {
      stem = std::move(path);
    } else {
      // FIXME: Try the appropriate extension for the current platform?
      stem = "lib";
      stem += path;
      stem += LTDL_SHLIB_EXT;
    }

    // Try user-provided library search paths first.
    for (auto &libDir : searchPathOpts.LibrarySearchPaths) {
      path = libDir;
      toolchain::sys::path::append(path, stem.str());
      success = loadRuntimeLib(path);
      if (success)
        break;
    }

    // Let loadRuntimeLib determine the best search paths.
    if (!success)
      success = loadRuntimeLib(stem);

    // If that fails, try our runtime library paths.
    if (!success)
      success = loadRuntimeLib(stem, searchPathOpts.RuntimeLibraryPaths);
    break;
  }
  case LibraryKind::Framework: {
    // If we have a framework, mangle the name to point to the framework
    // binary.
    toolchain::SmallString<64> frameworkPart{std::move(path)};
    frameworkPart += ".framework";
    toolchain::sys::path::append(frameworkPart, linkLib.getName());

    // Try user-provided framework search paths first; frameworks contain
    // binaries as well as modules.
    for (const auto &frameworkDir : searchPathOpts.getFrameworkSearchPaths()) {
      path = frameworkDir.Path;
      toolchain::sys::path::append(path, frameworkPart.str());
      success = loadRuntimeLib(path);
      if (success)
        break;
    }

    // If that fails, let loadRuntimeLib search for system frameworks.
    if (!success)
      success = loadRuntimeLib(frameworkPart);
    break;
  }
  }

  return success;
}

bool language::immediate::tryLoadLibraries(ArrayRef<LinkLibrary> LinkLibraries,
                                        SearchPathOptions SearchPathOpts,
                                        DiagnosticEngine &Diags) {
  SmallVector<bool, 4> LoadedLibraries;
  LoadedLibraries.append(LinkLibraries.size(), false);

  // Libraries are not sorted in the topological order of dependencies, and we
  // don't know the dependencies in advance.  Try to load all libraries until
  // we stop making progress.
  bool HadProgress;
  do {
    HadProgress = false;
    for (unsigned i = 0; i != LinkLibraries.size(); ++i) {
      if (!LoadedLibraries[i] &&
          tryLoadLibrary(LinkLibraries[i], SearchPathOpts)) {
        LoadedLibraries[i] = true;
        HadProgress = true;
      }
    }
  } while (HadProgress);

  return std::all_of(LoadedLibraries.begin(), LoadedLibraries.end(),
                     [](bool Value) { return Value; });
}

/// Workaround for rdar://94645534.
///
/// The framework layout of some frameworks have changed over time, causing
/// unresolved symbol errors in immediate mode when running on older OS versions
/// with a newer SDK. This workaround scans through the list of dependencies and
/// manually adds the right libraries as necessary.
///
/// FIXME: JITLink should emulate the Darwin linker's handling of ld$previous
/// mappings so this is handled automatically.
static void addMergedLibraries(SmallVectorImpl<LinkLibrary> &AllLinkLibraries,
                               const toolchain::Triple &Target) {
  assert(Target.isMacOSX());

  struct MergedLibrary {
    StringRef OldLibrary;
    toolchain::VersionTuple MovedIn;
  };

  using VersionTuple = toolchain::VersionTuple;

  static const toolchain::StringMap<MergedLibrary> MergedLibs = {
    // Merged in macOS 14.0
    {"AppKit", {"liblanguageAppKit.dylib", VersionTuple{14}}},
    {"HealthKit", {"liblanguageHealthKit.dylib", VersionTuple{14}}},
    {"Network", {"liblanguageNetwork.dylib", VersionTuple{14}}},
    {"Photos", {"liblanguagePhotos.dylib", VersionTuple{14}}},
    {"PhotosUI", {"liblanguagePhotosUI.dylib", VersionTuple{14}}},
    {"SoundAnalysis", {"liblanguageSoundAnalysis.dylib", VersionTuple{14}}},
    {"Virtualization", {"liblanguageVirtualization.dylib", VersionTuple{14}}},
    // Merged in macOS 13.0
    {"Foundation", {"liblanguageFoundation.dylib", VersionTuple{13}}},
  };

  SmallVector<StringRef> NewLibs;
  for (auto &Lib : AllLinkLibraries) {
    auto I = MergedLibs.find(Lib.getName());
    if (I != MergedLibs.end() && Target.getOSVersion() < I->second.MovedIn)
      NewLibs.push_back(I->second.OldLibrary);
  }

  for (StringRef NewLib : NewLibs)
    AllLinkLibraries.emplace_back(
        NewLib, LibraryKind::Library, /*static=*/false);
}

bool language::immediate::autolinkImportedModules(ModuleDecl *M,
                                               const IRGenOptions &IRGenOpts) {
  // Perform autolinking.
  SmallVector<LinkLibrary, 4> AllLinkLibraries(IRGenOpts.LinkLibraries);
  auto addLinkLibrary = [&](LinkLibrary linkLib) {
    AllLinkLibraries.push_back(linkLib);
  };

  M->collectLinkLibraries(addLinkLibrary);

  auto &Target = M->getASTContext().LangOpts.Target;
  if (Target.isMacOSX())
    addMergedLibraries(AllLinkLibraries, Target);

  tryLoadLibraries(AllLinkLibraries, M->getASTContext().SearchPathOpts,
                   M->getASTContext().Diags);
  return false;
}

/// Log a compilation error to standard error
static void logError(toolchain::Error Err) {
  logAllUnhandledErrors(std::move(Err), toolchain::errs(), "");
}

int language::RunImmediately(CompilerInstance &CI, const ProcessCmdLine &CmdLine,
                          const IRGenOptions &IRGenOpts,
                          const SILOptions &SILOpts,
                          std::unique_ptr<SILModule> &&SM) {

  auto &Context = CI.getASTContext();

  // Load libCodiraCore to setup process arguments.
  //
  // This must be done here, before any library loading has been done, to avoid
  // racing with the static initializers in user code.
  // Setup interpreted process arguments.
  using ArgOverride = void (*LANGUAGE_CC(language))(const char **, int);
#if defined(_WIN32)
  auto stdlib = loadCodiraRuntime(Context.SearchPathOpts.RuntimeLibraryPaths);
  if (!stdlib) {
    CI.getDiags().diagnose(SourceLoc(),
                           diag::error_immediate_mode_missing_stdlib);
    return -1;
  }
  auto module = static_cast<HMODULE>(stdlib);
  auto emplaceProcessArgs = reinterpret_cast<ArgOverride>(
      GetProcAddress(module, "_language_stdlib_overrideUnsafeArgvArgc"));
  if (emplaceProcessArgs == nullptr)
    return -1;
#else
  // In case the compiler is built with language modules, it already has the stdlib
  // linked to. First try to lookup the symbol with the standard library
  // resolving.
  auto emplaceProcessArgs =
      (ArgOverride)dlsym(RTLD_DEFAULT, "_language_stdlib_overrideUnsafeArgvArgc");

  if (dlerror()) {
    // If this does not work (= the Codira modules are not linked to the tool),
    // we have to explicitly load the stdlib.
    auto stdlib = loadCodiraRuntime(Context.SearchPathOpts.RuntimeLibraryPaths);
    if (!stdlib) {
      CI.getDiags().diagnose(SourceLoc(),
                             diag::error_immediate_mode_missing_stdlib);
      return -1;
    }
    dlerror();
    emplaceProcessArgs =
        (ArgOverride)dlsym(stdlib, "_language_stdlib_overrideUnsafeArgvArgc");
    if (dlerror())
      return -1;
  }
#endif

  SmallVector<const char *, 32> argBuf;
  for (size_t i = 0; i < CmdLine.size(); ++i) {
    argBuf.push_back(CmdLine[i].c_str());
  }
  argBuf.push_back(nullptr);

  (*emplaceProcessArgs)(argBuf.data(), CmdLine.size());

  auto *languageModule = CI.getMainModule();
  if (autolinkImportedModules(languageModule, IRGenOpts))
    return -1;

  auto JIT = CodiraJIT::Create(CI);
  if (auto Err = JIT.takeError()) {
    logError(std::move(Err));
    return -1;
  }

  auto MU = std::make_unique<EagerCodiraMaterializationUnit>(
      **JIT, CI, IRGenOpts, std::move(SM));
  if (auto Err = (*JIT)->addCodira((*JIT)->getMainJITDylib(), std::move(MU))) {
    logError(std::move(Err));
    return -1;
  }

  auto Result = (*JIT)->runMain(CmdLine);

  // It is not safe to unmap memory that has been registered with the language or
  // objc runtime. Currently the best way to avoid that is to leak the JIT.
  // FIXME: Replace with "detach" toolchain/toolchain-project#56714.
  (void)JIT->release();

  if (!Result) {
    logError(Result.takeError());
    return -1;
  }
  return *Result;
}

int language::RunImmediatelyFromAST(CompilerInstance &CI) {
  CI.performSema();
  auto &Context = CI.getASTContext();
  if (Context.hadError()) {
    return -1;
  }
  const auto &Invocation = CI.getInvocation();
  const auto &FrontendOpts = Invocation.getFrontendOptions();

  const ProcessCmdLine &CmdLine = ProcessCmdLine(
      FrontendOpts.ImmediateArgv.begin(), FrontendOpts.ImmediateArgv.end());

  // Load libCodiraCore to setup process arguments.
  //
  // This must be done here, before any library loading has been done, to avoid
  // racing with the static initializers in user code.
  // Setup interpreted process arguments.
  using ArgOverride = void (*LANGUAGE_CC(language))(const char **, int);
#if defined(_WIN32)
  auto stdlib = loadCodiraRuntime(Context.SearchPathOpts.RuntimeLibraryPaths);
  if (!stdlib) {
    CI.getDiags().diagnose(SourceLoc(),
                           diag::error_immediate_mode_missing_stdlib);
    return -1;
  }
  auto module = static_cast<HMODULE>(stdlib);
  auto emplaceProcessArgs = reinterpret_cast<ArgOverride>(
      GetProcAddress(module, "_language_stdlib_overrideUnsafeArgvArgc"));
  if (emplaceProcessArgs == nullptr)
    return -1;
#else
  // In case the compiler is built with language modules, it already has the stdlib
  // linked to. First try to lookup the symbol with the standard library
  // resolving.
  auto emplaceProcessArgs =
      (ArgOverride)dlsym(RTLD_DEFAULT, "_language_stdlib_overrideUnsafeArgvArgc");

  if (dlerror()) {
    // If this does not work (= the Codira modules are not linked to the tool),
    // we have to explicitly load the stdlib.
    auto stdlib = loadCodiraRuntime(Context.SearchPathOpts.RuntimeLibraryPaths);
    if (!stdlib) {
      CI.getDiags().diagnose(SourceLoc(),
                             diag::error_immediate_mode_missing_stdlib);
      return -1;
    }
    dlerror();
    emplaceProcessArgs =
        (ArgOverride)dlsym(stdlib, "_language_stdlib_overrideUnsafeArgvArgc");
    if (dlerror())
      return -1;
  }
#endif

  SmallVector<const char *, 32> argBuf;
  for (size_t i = 0; i < CmdLine.size(); ++i) {
    argBuf.push_back(CmdLine[i].c_str());
  }
  argBuf.push_back(nullptr);

  (*emplaceProcessArgs)(argBuf.data(), CmdLine.size());

  auto *languageModule = CI.getMainModule();
  const auto &IRGenOpts = Invocation.getIRGenOptions();
  if (autolinkImportedModules(languageModule, IRGenOpts))
    return -1;

  auto &Target = languageModule->getASTContext().LangOpts.Target;
  assert(Target.isMacOSX());
  auto JIT = CodiraJIT::Create(CI);
  if (auto Err = JIT.takeError()) {
    logError(std::move(Err));
    return -1;
  }

  // We're compiling functions lazily, so need to rename
  // symbols defining functions for lazy reexports
  (*JIT)->addRenamer();
  auto MU = LazyCodiraMaterializationUnit::Create(**JIT, CI);
  if (auto Err = (*JIT)->addCodira((*JIT)->getMainJITDylib(), std::move(MU))) {
    logError(std::move(Err));
    return -1;
  }

  auto Result = (*JIT)->runMain(CmdLine);
  if (!Result) {
    logError(Result.takeError());
    return -1;
  }

  return *Result;
}
