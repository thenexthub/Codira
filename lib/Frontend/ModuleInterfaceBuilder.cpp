//===----- ModuleInterfaceBuilder.cpp - Compiles .codeinterface files ----===//
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

#define DEBUG_TYPE "textual-module-interface"

#include "language/Frontend/ModuleInterfaceLoader.h"
#include "ModuleInterfaceBuilder.h"
#include "language/AST/ASTContext.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/FileSystem.h"
#include "language/AST/Module.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/ModuleInterfaceSupport.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/Serialization/SerializationOptions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/Support/xxhash.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/CrashRecoveryContext.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Errc.h"
#include "toolchain/Support/Regex.h"
#include "toolchain/Support/StringSaver.h"
#include "toolchain/Support/LockFileManager.h"
#include "toolchain/ADT/STLExtras.h"

using namespace language;
using FileDependency = SerializationOptions::FileDependency;
namespace path = toolchain::sys::path;

/// If the file dependency in \p FullDepPath is inside the \p Base directory,
/// this returns its path relative to \p Base. Otherwise it returns None.
static std::optional<StringRef> getRelativeDepPath(StringRef DepPath,
                                                   StringRef Base) {
  // If Base is the root directory, or DepPath does not start with Base, bail.
  if (Base.size() <= 1 || !DepPath.starts_with(Base)) {
    return std::nullopt;
  }

  assert(DepPath.size() > Base.size() &&
      "should never depend on a directory");

  // Is the DepName something like ${Base}/foo.h"?
  if (path::is_separator(DepPath[Base.size()]))
    return DepPath.substr(Base.size() + 1);

  // Is the DepName something like "${Base}foo.h", where Base
  // itself contains a trailing slash?
  if (path::is_separator(Base.back()))
    return DepPath.substr(Base.size());

  // We have something next to Base, like "Base.h", that's somehow
  // become a dependency.
  return std::nullopt;
}

struct ErrorDowngradeConsumerRAII: DiagnosticConsumer {
  DiagnosticEngine &Diag;
  std::vector<DiagnosticConsumer *> allConsumers;
  bool SeenError;
  ErrorDowngradeConsumerRAII(DiagnosticEngine &Diag): Diag(Diag),
      allConsumers(Diag.takeConsumers()), SeenError(false) {
    Diag.addConsumer(*this);
  }
  ~ErrorDowngradeConsumerRAII() {
    for (auto *consumer: allConsumers) {
      Diag.addConsumer(*consumer);
    }
    Diag.removeConsumer(*this);
  }
  void handleDiagnostic(SourceManager &SM, const DiagnosticInfo &Info) override {
    DiagnosticInfo localInfo(Info);
    if (localInfo.Kind == DiagnosticKind::Error) {
      localInfo.Kind = DiagnosticKind::Warning;
      SeenError = true;
      for (auto *consumer: allConsumers) {
        consumer->handleDiagnostic(SM, localInfo);
      }
    }
  }
};

bool ExplicitModuleInterfaceBuilder::collectDepsForSerialization(
    SmallVectorImpl<FileDependency> &Deps, StringRef interfacePath,
    bool IsHashBased) {
  toolchain::vfs::FileSystem &fs = *Instance.getSourceMgr().getFileSystem();

  auto &Opts = Instance.getASTContext().SearchPathOpts;
  SmallString<128> SDKPath(Opts.getSDKPath());
  path::native(SDKPath);
  SmallString<128> ResourcePath(Opts.RuntimeResourcePath);
  path::native(ResourcePath);
  // When compiling with a relative resource dir, the clang
  // importer will track inputs with absolute paths. To avoid
  // serializing resource dir inputs we need to check for
  // relative _and_ absolute prefixes.
  SmallString<128> AbsResourcePath(ResourcePath);
  toolchain::sys::fs::make_absolute(AbsResourcePath);

  auto DTDeps = Instance.getDependencyTracker()->getDependencies();
  SmallVector<std::string, 16> InitialDepNames(DTDeps.begin(), DTDeps.end());
  auto IncDeps =
      Instance.getDependencyTracker()->getIncrementalDependencyPaths();
  InitialDepNames.append(IncDeps.begin(), IncDeps.end());
  auto MacroDeps =
      Instance.getDependencyTracker()->getMacroPluginDependencyPaths();
  InitialDepNames.append(MacroDeps.begin(), MacroDeps.end());
  InitialDepNames.push_back(interfacePath.str());
  for (const auto &extra : extraDependencies) {
    InitialDepNames.push_back(extra.str());
  }
  SmallString<128> Scratch;

  for (const auto &InitialDepName : InitialDepNames) {
    path::native(InitialDepName, Scratch);
    StringRef DepName = Scratch.str();

    assert(moduleCachePath.empty() || !DepName.starts_with(moduleCachePath));

    // Serialize the paths of dependencies in the SDK relative to it.
    std::optional<StringRef> SDKRelativePath =
        getRelativeDepPath(DepName, SDKPath);
    StringRef DepNameToStore = SDKRelativePath.value_or(DepName);
    bool IsSDKRelative = SDKRelativePath.has_value();

    // Forwarding modules add the underlying prebuilt module to their
    // dependency list -- don't serialize that.
    if (!prebuiltCachePath.empty() && DepName.starts_with(prebuiltCachePath))
      continue;
    // Don't serialize interface path if it's from the preferred interface dir.
    // This ensures the prebuilt module caches generated from these interfaces
    // are relocatable.
    if (!backupInterfaceDir.empty() && DepName.starts_with(backupInterfaceDir))
      continue;
    if (dependencyTracker) {
      dependencyTracker->addDependency(DepName, /*isSystem*/ IsSDKRelative);
    }

    // Don't serialize compiler-relative deps so the cache is relocatable.
    if (DepName.starts_with(ResourcePath) || DepName.starts_with(AbsResourcePath))
      continue;

    auto Status = fs.status(DepName);
    if (!Status) {
      Instance.getDiags().diagnose(SourceLoc(), diag::cannot_open_file, DepName,
                                   Status.getError().message());
      return true;
    }

    /// Lazily load the dependency buffer if we need it. If we're not
    /// dealing with a hash-based dependencies, and if the dependency is
    /// not a .codemodule, we can avoid opening the buffer.
    std::unique_ptr<toolchain::MemoryBuffer> DepBuf = nullptr;
    auto getDepBuf = [&]() -> toolchain::MemoryBuffer * {
      if (DepBuf)
        return DepBuf.get();
      auto Buf = fs.getBufferForFile(DepName, /*FileSize=*/-1,
                                     /*RequiresNullTerminator=*/false);
      if (Buf) {
        DepBuf = std::move(Buf.get());
        return DepBuf.get();
      }
      Instance.getDiags().diagnose(SourceLoc(), diag::cannot_open_file, DepName,
                                   Buf.getError().message());
      return nullptr;
    };

    if (IsHashBased) {
      auto buf = getDepBuf();
      if (!buf)
        return true;
      uint64_t hash = xxHash64(buf->getBuffer());
      Deps.push_back(FileDependency::hashBased(DepNameToStore, IsSDKRelative,
                                               Status->getSize(), hash));
    } else {
      uint64_t mtime =
          Status->getLastModificationTime().time_since_epoch().count();
      Deps.push_back(FileDependency::modTimeBased(DepNameToStore, IsSDKRelative,
                                                  Status->getSize(), mtime));
    }
  }
  return false;
}

std::error_code ExplicitModuleInterfaceBuilder::buildCodiraModuleFromInterface(
    StringRef InterfacePath, StringRef OutputPath, bool ShouldSerializeDeps,
    std::unique_ptr<toolchain::MemoryBuffer> *ModuleBuffer,
    ArrayRef<std::string> CompiledCandidates,
    StringRef CompilerVersion) {
  auto Invocation = Instance.getInvocation();
  // Try building forwarding module first. If succeed, return.
  if (Instance.getASTContext()
          .getModuleInterfaceChecker()
          ->tryEmitForwardingModule(Invocation.getModuleName(), InterfacePath,
                                    CompiledCandidates,
                                    Instance.getOutputBackend(), OutputPath)) {
    return std::error_code();
  }
  FrontendOptions &FEOpts = Invocation.getFrontendOptions();
  bool isTypeChecking = FEOpts.isTypeCheckAction();
  const auto &InputInfo = FEOpts.InputsAndOutputs.firstInput();
  StringRef InPath = InputInfo.getFileName();

  // Build the .codemodule; this is a _very_ abridged version of the logic
  // in performCompile in libFrontendTool, specialized, to just the one
  // module-serialization task we're trying to do here.
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Setting up instance to compile " << InPath
                          << " to " << OutputPath << "\n");

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Performing sema\n");
  if (isTypeChecking &&
      Instance.downgradeInterfaceVerificationErrors()) {
    ErrorDowngradeConsumerRAII R(Instance.getDiags());
    Instance.performSema();
    return std::error_code();
  }
  LANGUAGE_DEFER {
    // Make sure to emit a generic top-level error if a module fails to
    // load. This is not only good for users; it also makes sure that we've
    // emitted an error in the parent diagnostic engine, which is what
    // determines whether the process exits with a proper failure status.
    if (Instance.getASTContext().hadError()) {
      auto builtByCompiler = getCodiraInterfaceCompilerVersionForCurrentCompiler(
          Instance.getASTContext());

      if (!isTypeChecking && CompilerVersion.str() != builtByCompiler) {
        diagnose(diag::module_interface_build_failed_mismatching_compiler,
                 InterfacePath,
                 Invocation.getModuleName(),
                 CompilerVersion,
                 builtByCompiler);
      } else {
        diagnose(diag::module_interface_build_failed,
                 InterfacePath,
                 isTypeChecking,
                 Invocation.getModuleName(),
                 CompilerVersion.str() == builtByCompiler,
                 CompilerVersion.str(),
                 builtByCompiler);
      }
    }
  };

  Instance.performSema();
  if (Instance.getASTContext().hadError()) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "encountered errors\n");
    return std::make_error_code(std::errc::not_supported);
  }
  // If we are just type-checking the interface, we are done.
  if (isTypeChecking)
    return std::error_code();

  SILOptions &SILOpts = Invocation.getSILOptions();
  auto Mod = Instance.getMainModule();
  Mod->setIsBuiltFromInterface(true);
  auto &TC = Instance.getSILTypes();
  auto SILMod = performASTLowering(Mod, TC, SILOpts);
  if (!SILMod) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "SILGen did not produce a module\n");
    return std::make_error_code(std::errc::not_supported);
  }

  // Setup the callbacks for serialization, which can occur during the
  // optimization pipeline.
  SerializationOptions SerializationOpts;
  std::string OutPathStr = OutputPath.str();
  SerializationOpts.StaticLibrary = FEOpts.Static;
  SerializationOpts.OutputPath = OutPathStr.c_str();
  SerializationOpts.ModuleLinkName = FEOpts.ModuleLinkName;
  SerializationOpts.AutolinkForceLoad =
      !Invocation.getIRGenOptions().ForceLoadSymbolName.empty();
  SerializationOpts.PublicDependentLibraries =
      Invocation.getIRGenOptions().PublicLinkLibraries;
  SerializationOpts.UserModuleVersion = FEOpts.UserModuleVersion;
  SerializationOpts.AllowableClients = FEOpts.AllowableClients;
  StringRef SDKPath = Instance.getASTContext().SearchPathOpts.getSDKPath();

  auto SDKRelativePath = getRelativeDepPath(InPath, SDKPath);
  if (SDKRelativePath.has_value()) {
    SerializationOpts.ModuleInterface = SDKRelativePath.value();
    SerializationOpts.IsInterfaceSDKRelative = true;
  } else
    SerializationOpts.ModuleInterface = InPath;

  SerializationOpts.SDKName = Instance.getASTContext().LangOpts.SDKName;
  SerializationOpts.ABIDescriptorPath = ABIDescriptorPath.str();
  SmallVector<FileDependency, 16> Deps;
  bool SerializeHashes = FEOpts.SerializeModuleInterfaceDependencyHashes;
  if (ShouldSerializeDeps) {
    if (collectDepsForSerialization(Deps, InterfacePath, SerializeHashes))
      return std::make_error_code(std::errc::not_supported);
    SerializationOpts.Dependencies = Deps;
  }
  SerializationOpts.IsOSSA = SILOpts.EnableOSSAModules;

  SILMod->setSerializeSILAction([&]() {
    // We don't want to serialize module docs in the cache -- they
    // will be serialized beside the interface file.
    serializeToBuffers(Mod, SerializationOpts, ModuleBuffer,
                       /*ModuleDocBuffer*/ nullptr,
                       /*SourceInfoBuffer*/ nullptr, SILMod.get());
  });

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Running SIL processing passes\n");
  if (Instance.performSILProcessing(SILMod.get())) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "encountered errors\n");
    return std::make_error_code(std::errc::not_supported);
  }
  if (Instance.getDiags().hadAnyError()) {
    return std::make_error_code(std::errc::not_supported);
  }
  return std::error_code();
}

bool ImplicitModuleInterfaceBuilder::buildCodiraModuleInternal(
    StringRef OutPath,
    bool ShouldSerializeDeps,
    std::unique_ptr<toolchain::MemoryBuffer> *ModuleBuffer,
    ArrayRef<std::string> CompiledCandidates) {

  auto outerPrettyStackState = toolchain::SavePrettyStackState();

  bool SubError = false;
  static const size_t ThreadStackSize = 8 << 20; // 8 MB.
  bool RunSuccess = toolchain::CrashRecoveryContext().RunSafelyOnThread([&] {
    // Pretend we're on the original thread for pretty-stack-trace purposes.
    auto savedInnerPrettyStackState = toolchain::SavePrettyStackState();
    toolchain::RestorePrettyStackState(outerPrettyStackState);
    LANGUAGE_DEFER {
      toolchain::RestorePrettyStackState(savedInnerPrettyStackState);
    };

    NullDiagnosticConsumer noopConsumer;
    std::optional<DiagnosticEngine> localDiags;
    DiagnosticEngine *rebuildDiags = diags;
    if (silenceInterfaceDiagnostics) {
      // To silence diagnostics, use a local temporary engine.
      localDiags.emplace(sourceMgr);
      rebuildDiags = &*localDiags;
      rebuildDiags->addConsumer(noopConsumer);
    }

    SubError = (bool)subASTDelegate.runInSubCompilerInstance(
        moduleName, interfacePath, sdkPath, sysroot, OutPath, diagnosticLoc,
        silenceInterfaceDiagnostics,
        [&](SubCompilerInstanceInfo &info) {
          auto EBuilder = ExplicitModuleInterfaceBuilder(
              *info.Instance, rebuildDiags, sourceMgr, moduleCachePath, backupInterfaceDir,
              prebuiltCachePath, ABIDescriptorPath, extraDependencies, diagnosticLoc,
              dependencyTracker);
          return EBuilder.buildCodiraModuleFromInterface(
              interfacePath, OutPath, ShouldSerializeDeps, ModuleBuffer,
              CompiledCandidates, info.CompilerVersion);
        });
  }, ThreadStackSize);
  return !RunSuccess || SubError;
}

bool ImplicitModuleInterfaceBuilder::buildCodiraModule(StringRef OutPath,
                                                      bool ShouldSerializeDeps,
                                                      std::unique_ptr<toolchain::MemoryBuffer> *ModuleBuffer,
                                                      toolchain::function_ref<void()> RemarkRebuild,
                                                      ArrayRef<std::string> CompiledCandidates) {
  auto build = [&]() {
    if (RemarkRebuild) {
      RemarkRebuild();
    }
    return buildCodiraModuleInternal(OutPath, ShouldSerializeDeps,
                                    ModuleBuffer, CompiledCandidates);
  };
  if (disableInterfaceFileLock) {
    return build();
  }
  while (1) {
  // Attempt to lock the interface file. Only one process is allowed to build
  // module from the interface so we don't consume too much memory when multiple
  // processes are doing the same.
  // FIXME: We should surface the module building step to the build system so
  // we don't need to synchronize here.
  toolchain::LockFileManager Lock(OutPath);
  bool Owned;
  if (toolchain::Error Err = Lock.tryLock().moveInto(Owned)) {
    toolchain::consumeError(std::move(Err));
    // ModuleInterfaceBuilder takes care of correctness and locks are only
    // necessary for performance. Fallback to building the module in case of any lock
    // related errors.
    if (RemarkRebuild) {
      diagnose(diag::interface_file_lock_failure);
    }
    return build();
  }
  if (Owned) {
    return build();
  }
  // Someone else is responsible for building the module. Wait for them to
  // finish.
  switch (Lock.waitForUnlockFor(std::chrono::seconds(256))) {
  case toolchain::WaitForUnlockResult::Success: {
    // This process may have a different module output path. If the other
    // process doesn't build the interface to this output path, we should try
    // building ourselves.
    auto bufferOrError = toolchain::MemoryBuffer::getFile(OutPath);
    if (!bufferOrError)
      continue;
    if (ModuleBuffer)
      *ModuleBuffer = std::move(bufferOrError.get());
    return false;
  }
  case toolchain::WaitForUnlockResult::OwnerDied: {
    continue; // try again to get the lock.
  }
  case toolchain::WaitForUnlockResult::Timeout: {
    // Since ModuleInterfaceBuilder takes care of correctness, we try waiting for
    // another process to complete the build so language does not do it done
    // twice. If case of timeout, build it ourselves.
    if (RemarkRebuild) {
      diagnose(diag::interface_file_lock_timed_out, interfacePath);
    }
    // Clear the lock file so that future invocations can make progress.
    Lock.unsafeMaybeUnlock();
    continue;
  }
  }
  }
}
