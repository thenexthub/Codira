//===----- ModuleInterfaceBuilder.h - Compiles .codeinterface files ------===//
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

#ifndef LANGUAGE_FRONTEND_MODULEINTERFACEBUILDER_H
#define LANGUAGE_FRONTEND_MODULEINTERFACEBUILDER_H

#include "language/AST/ModuleLoader.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/SourceLoc.h"
#include "language/Frontend/Frontend.h"
#include "language/Serialization/SerializationOptions.h"
#include "toolchain/Support/StringSaver.h"

namespace toolchain {
namespace vfs {
class FileSystem;
}
} // namespace toolchain

namespace language {

class DiagnosticEngine;
class LangOptions;
class SearchPathOptions;
class DependencyTracker;

/// A utility class to build a Codira interface file into a module
/// using a `CompilerInstance` constructed from the flags specified in the
/// interface file.
class ImplicitModuleInterfaceBuilder {
  SourceManager &sourceMgr;
  DiagnosticEngine *diags;
  InterfaceSubContextDelegate &subASTDelegate;
  const StringRef interfacePath;
  const StringRef sdkPath;
  const std::optional<StringRef> sysroot;
  const StringRef moduleName;
  const StringRef moduleCachePath;
  const StringRef prebuiltCachePath;
  const StringRef backupInterfaceDir;
  const StringRef ABIDescriptorPath;
  const bool disableInterfaceFileLock;
  const bool silenceInterfaceDiagnostics;
  const SourceLoc diagnosticLoc;
  DependencyTracker *const dependencyTracker;
  SmallVector<StringRef, 3> extraDependencies;

public:
  /// Emit a diagnostic tied to this declaration.
  template <typename... ArgTypes>
  static InFlightDiagnostic
  diagnose(DiagnosticEngine *Diags, SourceManager &SM, StringRef InterfacePath,
           SourceLoc Loc, Diag<ArgTypes...> ID,
           typename detail::PassArgument<ArgTypes>::type... Args) {
    if (Loc.isInvalid()) {
      // Diagnose this inside the interface file, if possible.
      Loc = SM.getLocFromExternalSource(InterfacePath, 1, 1);
    }
    return Diags->diagnose(Loc, ID, std::move(Args)...);
  }

private:
  /// Emit a diagnostic tied to this declaration.
  template <typename... ArgTypes>
  InFlightDiagnostic
  diagnose(Diag<ArgTypes...> ID,
           typename detail::PassArgument<ArgTypes>::type... Args) const {
    return diagnose(diags, sourceMgr, interfacePath, diagnosticLoc, ID,
                    std::move(Args)...);
  }

  bool
  buildCodiraModuleInternal(StringRef OutPath, bool ShouldSerializeDeps,
                           std::unique_ptr<toolchain::MemoryBuffer> *ModuleBuffer,
                           ArrayRef<std::string> CandidateModules);

public:
  ImplicitModuleInterfaceBuilder(
      SourceManager &sourceMgr, DiagnosticEngine *diags,
      InterfaceSubContextDelegate &subASTDelegate,
      StringRef interfacePath, StringRef sdkPath,
      std::optional<StringRef> sysroot, StringRef moduleName,
      StringRef moduleCachePath, StringRef backupInterfaceDir,
      StringRef prebuiltCachePath, StringRef ABIDescriptorPath,
      bool disableInterfaceFileLock = false,
      bool silenceInterfaceDiagnostics = false,
      SourceLoc diagnosticLoc = SourceLoc(),
      DependencyTracker *tracker = nullptr)
      : sourceMgr(sourceMgr), diags(diags), subASTDelegate(subASTDelegate),
        interfacePath(interfacePath), sdkPath(sdkPath), sysroot(sysroot),
        moduleName(moduleName), moduleCachePath(moduleCachePath),
        prebuiltCachePath(prebuiltCachePath),
        backupInterfaceDir(backupInterfaceDir),
        ABIDescriptorPath(ABIDescriptorPath),
        disableInterfaceFileLock(disableInterfaceFileLock),
        silenceInterfaceDiagnostics(silenceInterfaceDiagnostics),
        diagnosticLoc(diagnosticLoc), dependencyTracker(tracker) {}

  /// Ensures the requested file name is added as a dependency of the resulting
  /// module.
  void addExtraDependency(StringRef path) { extraDependencies.push_back(path); }

  bool buildCodiraModule(StringRef OutPath, bool ShouldSerializeDeps,
                        std::unique_ptr<toolchain::MemoryBuffer> *ModuleBuffer,
                        toolchain::function_ref<void()> RemarkRebuild = nullptr,
                        ArrayRef<std::string> CandidateModules = {});
};

/// Use the provided `CompilerInstance` to build a language interface into a module
class ExplicitModuleInterfaceBuilder {
public:
  ExplicitModuleInterfaceBuilder(CompilerInstance &Instance,
                                 DiagnosticEngine *diags,
                                 SourceManager &sourceMgr,
                                 const StringRef moduleCachePath,
                                 const StringRef backupInterfaceDir,
                                 const StringRef prebuiltCachePath,
                                 const StringRef ABIDescriptorPath,
                                 const SmallVector<StringRef, 3> &extraDependencies,
                                 SourceLoc diagnosticLoc = SourceLoc(),
                                 DependencyTracker *tracker = nullptr)
      : Instance(Instance),
        diags(diags),
        sourceMgr(sourceMgr),
        moduleCachePath(moduleCachePath),
        prebuiltCachePath(prebuiltCachePath),
        backupInterfaceDir(backupInterfaceDir),
        ABIDescriptorPath(ABIDescriptorPath),
        extraDependencies(extraDependencies),
        diagnosticLoc(diagnosticLoc),
        dependencyTracker(tracker) {}

  std::error_code buildCodiraModuleFromInterface(
      StringRef interfacePath, StringRef outputPath, bool ShouldSerializeDeps,
      std::unique_ptr<toolchain::MemoryBuffer> *ModuleBuffer,
      ArrayRef<std::string> CompiledCandidates,
      StringRef CompilerVersion);

  /// Populate the provided \p Deps with \c FileDependency entries for all
  /// dependencies \p SubInstance's DependencyTracker recorded while compiling
  /// the module, excepting .codemodules in \p moduleCachePath or
  /// \p prebuiltCachePath. Those have _their_ dependencies added instead, both
  /// to avoid having to do recursive scanning when rechecking this dependency
  /// in future and to make the module caches relocatable.
  bool collectDepsForSerialization(
      SmallVectorImpl<SerializationOptions::FileDependency> &Deps,
      StringRef interfacePath, bool IsHashBased);

  /// Emit a diagnostic tied to this declaration.
  template <typename... ArgTypes>
  static InFlightDiagnostic
  diagnose(DiagnosticEngine *Diags, SourceManager &SM, StringRef InterfacePath,
           SourceLoc Loc, Diag<ArgTypes...> ID,
           typename detail::PassArgument<ArgTypes>::type... Args) {
    if (Loc.isInvalid()) {
      // Diagnose this inside the interface file, if possible.
      Loc = SM.getLocFromExternalSource(InterfacePath, 1, 1);
    }
    return Diags->diagnose(Loc, ID, std::move(Args)...);
  }

private:
  /// Emit a diagnostic tied to this declaration.
  template<typename ...ArgTypes>
  InFlightDiagnostic diagnose(
      Diag<ArgTypes...> ID,
      StringRef InterfacePath,
      typename detail::PassArgument<ArgTypes>::type... Args) const {
    return diagnose(diags, sourceMgr, InterfacePath, diagnosticLoc,
                    ID, std::move(Args)...);
  }

private:
  CompilerInstance &Instance;
  DiagnosticEngine *diags;
  SourceManager &sourceMgr;
  StringRef moduleCachePath;
  StringRef prebuiltCachePath;
  StringRef backupInterfaceDir;
  StringRef ABIDescriptorPath;
  const SmallVector<StringRef, 3> extraDependencies;
  const SourceLoc diagnosticLoc;
  DependencyTracker *const dependencyTracker;
};

} // end namespace language

#endif // defined(LANGUAGE_FRONTEND_MODULEINTERFACEBUILDER_H)
