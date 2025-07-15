//===--- Frontend.h - frontend utility methods ------------------*- C++ -*-===//
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
// This file contains declarations of utility methods for parsing and
// performing semantic on modules.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_FRONTEND_H
#define LANGUAGE_FRONTEND_H

#include "language/AST/DiagnosticConsumer.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/IRGenOptions.h"
#include "language/AST/LinkLibrary.h"
#include "language/AST/Module.h"
#include "language/AST/SILOptions.h"
#include "language/AST/SearchPathOptions.h"
#include "language/AST/SourceFile.h"
#include "language/Basic/CASOptions.h"
#include "language/Basic/DiagnosticOptions.h"
#include "language/Basic/LangOptions.h"
#include "language/Basic/SourceManager.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/Frontend/CASOutputBackends.h"
#include "language/Frontend/CachedDiagnostics.h"
#include "language/Frontend/DiagnosticVerifier.h"
#include "language/Frontend/FrontendOptions.h"
#include "language/Frontend/ModuleInterfaceSupport.h"
#include "language/IRGen/TBDGen.h"
#include "language/Migrator/MigratorOptions.h"
#include "language/Parse/IDEInspectionCallbacks.h"
#include "language/Sema/SourceLoader.h"
#include "language/Serialization/Validation.h"
#include "language/Subsystems.h"
#include "language/SymbolGraphGen/SymbolGraphOptions.h"
#include "clang/Basic/FileManager.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/CAS/ActionCache.h"
#include "toolchain/CAS/ObjectStore.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Support/BLAKE3.h"
#include "toolchain/Support/HashingOutputBackend.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/VirtualOutputBackend.h"
#include "toolchain/TargetParser/Host.h"

#include <memory>

namespace language {

class FrontendObserver;
class SerializedModuleLoaderBase;
class MemoryBufferSerializedModuleLoader;
class SILModule;

namespace Lowering {
class TypeConverter;
}

struct ModuleBuffers {
  std::unique_ptr<toolchain::MemoryBuffer> ModuleBuffer;
  std::unique_ptr<toolchain::MemoryBuffer> ModuleDocBuffer;
  std::unique_ptr<toolchain::MemoryBuffer> ModuleSourceInfoBuffer;
  ModuleBuffers(std::unique_ptr<toolchain::MemoryBuffer> ModuleBuffer,
                std::unique_ptr<toolchain::MemoryBuffer> ModuleDocBuffer = nullptr,
                std::unique_ptr<toolchain::MemoryBuffer> ModuleSourceInfoBuffer = nullptr):
                  ModuleBuffer(std::move(ModuleBuffer)),
                  ModuleDocBuffer(std::move(ModuleDocBuffer)),
                  ModuleSourceInfoBuffer(std::move(ModuleSourceInfoBuffer)) {}
};

/// The abstract configuration of the compiler, including:
///   - options for all stages of translation,
///   - information about the build environment,
///   - information about the job being performed, and
///   - lists of inputs and outputs.
///
/// A CompilerInvocation can be built from a frontend command line
/// using parseArgs.  It can then be used to build a CompilerInstance,
/// which manages the actual compiler execution.
class CompilerInvocation {
  LangOptions LangOpts;
  TypeCheckerOptions TypeCheckerOpts;
  FrontendOptions FrontendOpts;
  ClangImporterOptions ClangImporterOpts;
  symbolgraphgen::SymbolGraphOptions SymbolGraphOpts;
  SearchPathOptions SearchPathOpts;
  DiagnosticOptions DiagnosticOpts;
  MigratorOptions MigratorOpts;
  SILOptions SILOpts;
  IRGenOptions IRGenOpts;
  TBDGenOptions TBDGenOpts;
  ModuleInterfaceOptions ModuleInterfaceOpts;
  CASOptions CASOpts;
  SerializationOptions SerializationOpts;
  toolchain::MemoryBuffer *IDEInspectionTargetBuffer = nullptr;

  /// The offset that IDEInspection wants to further examine in offset of bytes
  /// from the beginning of the main source file.  Valid only if
  /// \c isIDEInspection() == true.
  unsigned IDEInspectionOffset = ~0U;

  IDEInspectionCallbacksFactory *IDEInspectionFactory = nullptr;

public:
  CompilerInvocation();

  /// Initializes the compiler invocation and diagnostic engine for the list of
  /// arguments.
  ///
  /// All parsing should be additive, i.e. options should not be reset to their
  /// default values given the /absence/ of a flag. This is because \c parseArgs
  /// may be used to modify an already partially configured invocation.
  ///
  /// As a side-effect of parsing, the diagnostic engine will be configured with
  /// the options specified by the parsed arguments. This ensures that the
  /// arguments can effect the behavior of diagnostics emitted during parsing.
  ///
  /// Any configuration files loaded as a result of parsing arguments will be
  /// stored in \p ConfigurationFileBuffers, if non-null. The contents of these
  /// buffers should \e not be interpreted by the caller; they are only present
  /// in order to make it possible to reproduce how these arguments were parsed
  /// if the compiler ends up crashing or exhibiting other bad behavior.
  ///
  /// If non-empty, relative search paths are resolved relative to
  /// \p workingDirectory.
  ///
  /// \returns true if there was an error, false on success.
  bool parseArgs(ArrayRef<const char *> Args, DiagnosticEngine &Diags,
                 SmallVectorImpl<std::unique_ptr<toolchain::MemoryBuffer>>
                     *ConfigurationFileBuffers = nullptr,
                 StringRef workingDirectory = {},
                 StringRef mainExecutablePath = {});

  /// Sets specific options based on the given serialized Codira binary data.
  ///
  /// This is additive, i.e. options are not reset to their default values given
  /// the /absence/ of a flag. However, flags that only have a single value may
  /// (and should) be overwritten by this method.
  ///
  /// Invoking this on more than one serialized AST is likely to result in
  /// one or both of them failing to load. Please pick one AST to provide base
  /// flags for the entire ASTContext and let the others succeed or fail the
  /// normal way. (Some additive flags, like search paths, will be handled
  /// properly during normal module loading.)
  ///
  /// \returns Status::Valid on success, one of the Status issues on error.
  serialization::Status loadFromSerializedAST(StringRef data);

  /// Serialize the command line arguments for emitting them
  /// to DWARF or CodeView and inject SDKPath if necessary.
  static void buildDebugFlags(std::string &Output,
                              const toolchain::opt::ArgList &Args,
                              StringRef SDKPath,
                              StringRef ResourceDir);

  /// Configures the diagnostic engine for the invocation's options.
  void setUpDiagnosticEngine(DiagnosticEngine &diags);

  void setTargetTriple(const toolchain::Triple &Triple);
  void setTargetTriple(StringRef Triple);

  StringRef getTargetTriple() const {
    return LangOpts.Target.str();
  }

  bool requiresCAS() const {
    return CASOpts.EnableCaching || IRGenOpts.UseCASBackend ||
           CASOpts.ImportModuleFromCAS;
  }

  void setClangModuleCachePath(StringRef Path) {
    ClangImporterOpts.ModuleCachePath = Path.str();
  }

  void setClangScannerModuleCachePath(StringRef Path) {
    ClangImporterOpts.ClangScannerModuleCachePath = Path.str();
  }

  StringRef getClangModuleCachePath() const {
    return ClangImporterOpts.ModuleCachePath;
  }

  StringRef getClangScannerModuleCachePath() const {
    return ClangImporterOpts.ClangScannerModuleCachePath;
  }

  void setImportSearchPaths(
      const std::vector<SearchPathOptions::SearchPath> &Paths) {
    SearchPathOpts.setImportSearchPaths(Paths);
  }

  void setSerializedPathObfuscator(const PathObfuscator &obfuscator) {
    FrontendOpts.serializedPathObfuscator = obfuscator;
    SearchPathOpts.DeserializedPathRecoverer = obfuscator;
  }

  ArrayRef<SearchPathOptions::SearchPath> getImportSearchPaths() const {
    return SearchPathOpts.getImportSearchPaths();
  }

  void setFrameworkSearchPaths(
      const std::vector<SearchPathOptions::SearchPath> &Paths) {
    SearchPathOpts.setFrameworkSearchPaths(Paths);
  }

  ArrayRef<SearchPathOptions::SearchPath> getFrameworkSearchPaths() const {
    return SearchPathOpts.getFrameworkSearchPaths();
  }

  void setVFSOverlays(const std::vector<std::string> &Overlays) {
    SearchPathOpts.VFSOverlayFiles = Overlays;
  }

  void setSysRoot(StringRef SysRoot) {
    SearchPathOpts.setSysRoot(SysRoot);
  }

  void setExtraClangArgs(const std::vector<std::string> &Args) {
    ClangImporterOpts.ExtraArgs = Args;
  }

  ArrayRef<std::string> getExtraClangArgs() const {
    return ClangImporterOpts.ExtraArgs;
  }

  void addLinkLibrary(StringRef name, LibraryKind kind, bool isStaticLibrary) {
    IRGenOpts.LinkLibraries.emplace_back(name, kind, isStaticLibrary);
  }

  ArrayRef<LinkLibrary> getLinkLibraries() const {
    return IRGenOpts.LinkLibraries;
  }

  void setMainExecutablePath(StringRef Path);

  void setRuntimeResourcePath(StringRef Path);

  void setPlatformAvailabilityInheritanceMapPath(StringRef Path);

  /// Compute the default prebuilt module cache path for a given resource path
  /// and SDK version. This function is also used by LLDB.
  static std::string
  computePrebuiltCachePath(StringRef RuntimeResourcePath, toolchain::Triple target,
                           std::optional<toolchain::VersionTuple> sdkVer);

  /// If we haven't explicitly passed -prebuilt-module-cache-path, set it to
  /// the default value of <resource-dir>/<platform>/prebuilt-modules.
  /// @note This should be called once, after search path options and frontend
  ///       options have been parsed.
  void setDefaultPrebuiltCacheIfNecessary();

  /// If we haven't explicitly passed -blocklist-paths, set it to the default value.
  void setDefaultBlocklistsIfNecessary();

  /// If we haven't explicitly passed '-in-process-plugin-server-path', infer
  /// it as a default value.
  ///
  /// FIXME: Remove this after all the clients start sending it.
  void setDefaultInProcessPluginServerPathIfNecessary();

  /// Determine which C++ stdlib should be used for this compilation, and which
  /// C++ stdlib is the default for the specified target.
  void computeCXXStdlibOptions();

  /// Computes the runtime resource path relative to the given Codira
  /// executable.
  static void computeRuntimeResourcePathFromExecutablePath(
      StringRef mainExecutablePath, bool shared,
      toolchain::SmallVectorImpl<char> &runtimeResourcePath);

  /// Appends `lib/language[_static]` to the given path
  static void appendCodiraLibDir(toolchain::SmallVectorImpl<char> &path, bool shared);

  void setSDKPath(const std::string &Path);

  StringRef getSDKPath() const { return SearchPathOpts.getSDKPath(); }

  LangOptions &getLangOptions() {
    return LangOpts;
  }
  const LangOptions &getLangOptions() const {
    return LangOpts;
  }

  TypeCheckerOptions &getTypeCheckerOptions() { return TypeCheckerOpts; }
  const TypeCheckerOptions &getTypeCheckerOptions() const {
    return TypeCheckerOpts;
  }

  FrontendOptions &getFrontendOptions() { return FrontendOpts; }
  const FrontendOptions &getFrontendOptions() const { return FrontendOpts; }

  CASOptions &getCASOptions() { return CASOpts; }
  const CASOptions &getCASOptions() const { return CASOpts; }

  TBDGenOptions &getTBDGenOptions() { return TBDGenOpts; }
  const TBDGenOptions &getTBDGenOptions() const { return TBDGenOpts; }

  ModuleInterfaceOptions &getModuleInterfaceOptions() { return ModuleInterfaceOpts; }
  const ModuleInterfaceOptions &getModuleInterfaceOptions() const { return ModuleInterfaceOpts; }

  ClangImporterOptions &getClangImporterOptions() { return ClangImporterOpts; }
  const ClangImporterOptions &getClangImporterOptions() const {
    return ClangImporterOpts;
  }

  symbolgraphgen::SymbolGraphOptions &getSymbolGraphOptions() { return SymbolGraphOpts; }
  const symbolgraphgen::SymbolGraphOptions &getSymbolGraphOptions() const {
    return SymbolGraphOpts;
  }

  SearchPathOptions &getSearchPathOptions() { return SearchPathOpts; }
  const SearchPathOptions &getSearchPathOptions() const {
    return SearchPathOpts;
  }

  DiagnosticOptions &getDiagnosticOptions() { return DiagnosticOpts; }
  const DiagnosticOptions &getDiagnosticOptions() const {
    return DiagnosticOpts;
  }

  const MigratorOptions &getMigratorOptions() const {
    return MigratorOpts;
  }

  SILOptions &getSILOptions() { return SILOpts; }
  const SILOptions &getSILOptions() const { return SILOpts; }

  IRGenOptions &getIRGenOptions() { return IRGenOpts; }
  const IRGenOptions &getIRGenOptions() const { return IRGenOpts; }

  SerializationOptions &getSerializationOptions() { return SerializationOpts; }
  const SerializationOptions &getSerializationOptions() const {
    return SerializationOpts;
  }

  void setParseStdlib() {
    FrontendOpts.ParseStdlib = true;
  }

  bool getParseStdlib() const {
    return FrontendOpts.ParseStdlib;
  }

  void setModuleName(StringRef Name) {
    FrontendOpts.ModuleName = Name.str();
    IRGenOpts.ModuleName = Name.str();
  }

  StringRef getModuleName() const {
    return FrontendOpts.ModuleName;
  }

  /// Sets the module alias map with string args passed in via `-module-alias`.
  /// \param args The arguments to `-module-alias`. If input has `-module-alias Foo=Bar
  ///             -module-alias Baz=Qux`, the args are ['Foo=Bar', 'Baz=Qux'].  The name
  ///             Foo is the name that appears in source files, while it maps to Bar, the name
  ///             of the binary on disk, /path/to/Bar.codemodule(interface), under the hood.
  /// \param diags Used to print diagnostics in case validation of the string args fails.
  ///        See \c ModuleAliasesConverter::computeModuleAliases on validation details.
  /// \return Whether setting module alias map succeeded; false if args validation fails.
  bool setModuleAliasMap(std::vector<std::string> args, DiagnosticEngine &diags);

  std::string getOutputFilename() const {
    return FrontendOpts.InputsAndOutputs.getSingleOutputFilename();
  }

  void setIDEInspectionTarget(toolchain::MemoryBuffer *Buf, unsigned Offset) {
    assert(Buf);
    IDEInspectionTargetBuffer = Buf;
    IDEInspectionOffset = Offset;
    // We don't need typo-correction for code-completion.
    // FIXME: This isn't really true, but is a performance issue.
    LangOpts.TypoCorrectionLimit = 0;
  }

  std::pair<toolchain::MemoryBuffer *, unsigned> getIDEInspectionTarget() const {
    return std::make_pair(IDEInspectionTargetBuffer, IDEInspectionOffset);
  }

  /// \returns true if we are doing code completion.
  bool isIDEInspection() const {
    return IDEInspectionOffset != ~0U;
  }

  /// Retrieve a module hash string that is suitable for uniquely
  /// identifying the conditions under which the module was built, for use
  /// in generating a cached PCH file for the bridging header.
  std::string getPCHHash() const;

  /// Retrieve a module hash string that is suitable for uniquely
  /// identifying the conditions under which the current module is built,
  /// from the perspective of a dependency scanning action.
  std::string getModuleScanningHash() const;

  /// Retrieve the stdlib kind to implicitly import.
  ImplicitStdlibKind getImplicitStdlibKind() const {
    if (FrontendOpts.InputMode == FrontendOptions::ParseInputMode::SIL) {
      return ImplicitStdlibKind::None;
    }
    if (FrontendOpts.InputsAndOutputs.shouldTreatAsLLVM()) {
      return ImplicitStdlibKind::None;
    }
    if (getParseStdlib()) {
      return ImplicitStdlibKind::Builtin;
    }
    return ImplicitStdlibKind::Stdlib;
  }

  /// Whether the Codira -Onone support library should be implicitly imported.
  bool shouldImportCodiraONoneSupport() const;

  /// Whether the Codira Concurrency support library should be implicitly
  /// imported.
  bool shouldImportCodiraConcurrency() const;

  /// Whether the Codira String Processing support library should be implicitly
  /// imported.
  bool shouldImportCodiraStringProcessing() const;

  /// Whether the CXX module should be implicitly imported.
  bool shouldImportCxx() const;

  /// Performs input setup common to these tools:
  /// sil-opt, sil-fn-extractor, sil-toolchain-gen, and sil-nm.
  /// Return value includes the buffer so caller can keep it alive.
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>>
  setUpInputForSILTool(StringRef inputFilename, StringRef moduleNameArg,
                       bool alwaysSetModuleToMain, bool bePrimary,
                       serialization::ExtendedValidationInfo &extendedInfo);

  const PrimarySpecificPaths &
  getPrimarySpecificPathsForAtMostOnePrimary() const;
  const PrimarySpecificPaths &
  getPrimarySpecificPathsForPrimary(StringRef filename) const;
  const PrimarySpecificPaths &
  getPrimarySpecificPathsForSourceFile(const SourceFile &SF) const;

  std::string getOutputFilenameForAtMostOnePrimary() const;
  std::string getMainInputFilenameForDebugInfoForAtMostOnePrimary() const;
  std::string getClangHeaderOutputPathForAtMostOnePrimary() const;
  std::string getModuleOutputPathForAtMostOnePrimary() const;
  std::string
  getReferenceDependenciesFilePathForPrimary(StringRef filename) const;
  std::string getConstValuesFilePathForPrimary(StringRef filename) const;
  std::string getSerializedDiagnosticsPathForAtMostOnePrimary() const;

  /// TBDPath only makes sense in whole module compilation mode,
  /// so return the TBDPath when in that mode and fail an assert
  /// if not in that mode.
  std::string getTBDPathForWholeModule() const;

  /// ModuleInterfaceOutputPath only makes sense in whole module compilation
  /// mode, so return the ModuleInterfaceOutputPath when in that mode and
  /// fail an assert if not in that mode.
  std::string getModuleInterfaceOutputPathForWholeModule() const;
  std::string getPrivateModuleInterfaceOutputPathForWholeModule() const;
  std::string getPackageModuleInterfaceOutputPathForWholeModule() const;

  /// APIDescriptorPath only makes sense in whole module compilation mode,
  /// so return the APIDescriptorPath when in that mode and fail an assert
  /// if not in that mode.
  std::string getAPIDescriptorPathForWholeModule() const;

public:
  /// Given the current configuration of this frontend invocation, a set of
  /// supplementary output paths, and a module, compute the appropriate set of
  /// serialization options.
  ///
  /// FIXME: The \p module parameter supports the
  /// \c SerializeOptionsForDebugging hack.
  SerializationOptions
  computeSerializationOptions(const SupplementaryOutputPaths &outs,
                              const ModuleDecl *module) const;
};

/// A class which manages the state and execution of the compiler.
/// This owns the primary compiler singletons, such as the ASTContext,
/// as well as various build products such as the SILModule.
///
/// Before a CompilerInstance can be used, it must be configured by
/// calling \a setup.  If successful, this will create an ASTContext
/// and set up the basic compiler invariants.  Calling \a setup multiple
/// times on a single CompilerInstance is not permitted.
class CompilerInstance {
  CompilerInvocation Invocation;

  /// CAS Instances.
  /// This needs to be declared before SourceMgr because when using CASFS,
  /// the file buffer provided by CAS needs to outlive the SourceMgr.
  std::shared_ptr<toolchain::cas::ObjectStore> CAS;
  std::shared_ptr<toolchain::cas::ActionCache> ResultCache;
  std::optional<toolchain::cas::ObjectRef> CompileJobBaseKey;

  SourceManager SourceMgr;
  DiagnosticEngine Diagnostics{SourceMgr};
  std::unique_ptr<ASTContext> Context;
  std::unique_ptr<Lowering::TypeConverter> TheSILTypes;
  std::unique_ptr<DiagnosticVerifier> DiagVerifier;
  std::unique_ptr<CachingDiagnosticsProcessor> CDP;

  /// A cache describing the set of inter-module dependencies that have been queried.
  /// Null if not present.
  std::unique_ptr<ModuleDependenciesCache> ModDepCache;

  /// Null if no tracker.
  std::unique_ptr<DependencyTracker> DepTracker;
  /// If there is no stats output directory by the time the
  /// instance has completed its setup, this will be null.
  std::unique_ptr<UnifiedStatsReporter> Stats;

  /// Virtual OutputBackend.
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::OutputBackend> OutputBackend = nullptr;

  /// CAS OutputBackend.
  toolchain::IntrusiveRefCntPtr<language::cas::CodiraCASOutputBackend> CASOutputBackend =
      nullptr;

  /// The verification output backend.
  using HashBackendTy = toolchain::vfs::HashingOutputBackend<toolchain::BLAKE3>;
  toolchain::IntrusiveRefCntPtr<HashBackendTy> HashBackend;

  mutable ModuleDecl *MainModule = nullptr;
  SerializedModuleLoaderBase *DefaultSerializedLoader = nullptr;
  MemoryBufferSerializedModuleLoader *MemoryBufferLoader = nullptr;

  /// Contains buffer IDs for input source code files.
  std::vector<unsigned> InputSourceCodeBufferIDs;

  /// Contains \c MemoryBuffers for partial serialized module files and
  /// corresponding partial serialized module documentation files. This is
  /// \c mutable as it is consumed by \c loadPartialModulesAndImplicitImports.
  mutable std::vector<ModuleBuffers> PartialModules;

  /// Identifies the set of input buffers in the SourceManager that are
  /// considered primaries.
  toolchain::SetVector<unsigned> PrimaryBufferIDs;

  /// Return whether there is an entry in PrimaryInputs for buffer \p BufID.
  bool isPrimaryInput(unsigned BufID) const {
    return PrimaryBufferIDs.count(BufID) != 0;
  }

  /// Record in PrimaryBufferIDs the fact that \p BufID is a primary.
  /// If \p BufID is already in the set, do nothing.
  void recordPrimaryInputBuffer(unsigned BufID);

  bool isWholeModuleCompilation() const { return PrimaryBufferIDs.empty(); }

public:
  // Out of line to avoid having to import SILModule.h.
  CompilerInstance();
  ~CompilerInstance();

  CompilerInstance(const CompilerInstance &) = delete;
  void operator=(const CompilerInstance &) = delete;
  CompilerInstance(CompilerInstance &&) = delete;
  void operator=(CompilerInstance &&) = delete;

  SourceManager &getSourceMgr() { return SourceMgr; }
  const SourceManager &getSourceMgr() const { return SourceMgr; }

  DiagnosticEngine &getDiags() { return Diagnostics; }
  const DiagnosticEngine &getDiags() const { return Diagnostics; }

  toolchain::vfs::FileSystem &getFileSystem() const {
    return *SourceMgr.getFileSystem();
  }

  toolchain::vfs::OutputBackend &getOutputBackend() const {
    return *OutputBackend;
  }
  language::cas::CodiraCASOutputBackend &getCASOutputBackend() const {
    return *CASOutputBackend;
  }

  void
  setOutputBackend(toolchain::IntrusiveRefCntPtr<toolchain::vfs::OutputBackend> Backend) {
    OutputBackend = std::move(Backend);
  }
  using HashingBackendPtrTy = toolchain::IntrusiveRefCntPtr<HashBackendTy>;
  HashingBackendPtrTy getHashingBackend() { return HashBackend; }

  toolchain::cas::ObjectStore &getObjectStore() const { return *CAS; }
  toolchain::cas::ActionCache &getActionCache() const { return *ResultCache; }
  std::shared_ptr<toolchain::cas::ActionCache> getSharedCacheInstance() const {
    return ResultCache;
  }
  std::shared_ptr<toolchain::cas::ObjectStore> getSharedCASInstance() const {
    return CAS;
  }
  std::optional<toolchain::cas::ObjectRef> getCompilerBaseKey() const {
    return CompileJobBaseKey;
  }
  CachingDiagnosticsProcessor *getCachingDiagnosticsProcessor() const {
    return CDP.get();
  }

  ASTContext &getASTContext() { return *Context; }
  const ASTContext &getASTContext() const { return *Context; }

  bool hasASTContext() const { return Context != nullptr; }

  const SILOptions &getSILOptions() const { return Invocation.getSILOptions(); }

  Lowering::TypeConverter &getSILTypes();

  void addDiagnosticConsumer(DiagnosticConsumer *DC) {
    Diagnostics.addConsumer(*DC);
  }

  void removeDiagnosticConsumer(DiagnosticConsumer *DC) {
    Diagnostics.removeConsumer(*DC);
  }

  DependencyTracker *getDependencyTracker() { return DepTracker.get(); }
  const DependencyTracker *getDependencyTracker() const { return DepTracker.get(); }

  UnifiedStatsReporter *getStatsReporter() const { return Stats.get(); }

  /// Retrieve the main module containing the files being compiled.
  ModuleDecl *getMainModule() const;

  /// Replace the current main module with a new one. This is used for top-level
  /// cached code completion.
  void setMainModule(ModuleDecl *newMod);

  MemoryBufferSerializedModuleLoader *
  getMemoryBufferSerializedModuleLoader() const {
    return MemoryBufferLoader;
  }

  ArrayRef<unsigned> getInputBufferIDs() const {
    return InputSourceCodeBufferIDs;
  }

  ArrayRef<LinkLibrary> getLinkLibraries() const {
    return Invocation.getLinkLibraries();
  }

  bool hasSourceImport() const {
    return Invocation.getFrontendOptions().EnableSourceImport;
  }

  /// Gets the set of SourceFiles which are the primary inputs for this
  /// CompilerInstance.
  ArrayRef<SourceFile *> getPrimarySourceFiles() const {
    return getMainModule()->getPrimarySourceFiles();
  }

  /// Verify that if an implicit import of the `Concurrency` module if expected,
  /// it can actually be imported. Emit a warning, otherwise.
  void verifyImplicitConcurrencyImport();

  /// Whether the Codira Concurrency support library can be imported
  /// i.e. if it can be found.
  bool canImportCodiraConcurrency() const;

  /// Whether the Codira Concurrency Shims support Clang library can be imported
  /// i.e. if it can be found.
  bool canImportCodiraConcurrencyShims() const;

  /// Verify that if an implicit import of the `StringProcessing` module if
  /// expected, it can actually be imported. Emit a warning, otherwise.
  void verifyImplicitStringProcessingImport();

  /// Whether the Codira String Processing support library can be imported
  /// i.e. if it can be found.
  bool canImportCodiraStringProcessing() const;

  /// Whether the Cxx library can be imported
  bool canImportCxx() const;

  /// Whether the CxxShim library can be imported
  /// i.e. if it can be found.
  bool canImportCxxShim() const;

  /// Whether this compiler instance supports caching.
  bool supportCaching() const;

  /// Whether errors during interface verification can be downgrated
  /// to warnings.
  bool downgradeInterfaceVerificationErrors() const;

  /// Gets the SourceFile which is the primary input for this CompilerInstance.
  /// \returns the primary SourceFile, or nullptr if there is no primary input;
  /// if there are _multiple_ primary inputs, fails with an assertion.
  ///
  /// FIXME: This should be removed eventually, once there are no longer any
  /// codepaths that rely on a single primary file.
  SourceFile *getPrimarySourceFile() const {
    auto primaries = getPrimarySourceFiles();
    if (primaries.empty()) {
      return nullptr;
    } else {
      assert(primaries.size() == 1);
      return *primaries.begin();
    }
  }

  /// Returns true if there was an error during setup.
  bool setup(const CompilerInvocation &Invocation, std::string &Error,
             ArrayRef<const char *> Args = {});

  /// The fast setup function for cache replay.
  bool setupForReplay(const CompilerInvocation &Invocation, std::string &Error,
                      ArrayRef<const char *> Args = {});

  const CompilerInvocation &getInvocation() const { return Invocation; }

  /// If a IDE inspection buffer has been set, returns the corresponding source
  /// file.
  SourceFile *getIDEInspectionFile() const;

  /// Retrieve the printing path for bridging header.
  std::string getBridgingHeaderPath() const;

private:
  /// Set up the file system by loading and validating all VFS overlay YAML
  /// files. If the process of validating VFS files failed, or the overlay
  /// file system could not be initialized, this function returns true. Else it
  /// returns false if setup succeeded.
  bool setUpVirtualFileSystemOverlays();
  void setUpLLVMArguments();
  void setUpDiagnosticOptions();
  bool setUpModuleLoaders();
  bool setUpPluginLoader();
  bool setUpInputs();
  bool setUpASTContextIfNeeded();
  void setupStatsReporter();
  void setupDependencyTrackerIfNeeded();
  bool setupCASIfNeeded(ArrayRef<const char *> Args);
  void setupOutputBackend();
  void setupCachingDiagnosticsProcessorIfNeeded();

  /// \return false if successful, true on error.
  bool setupDiagnosticVerifierIfNeeded();

  std::optional<unsigned> setUpIDEInspectionTargetBuffer();

  /// Find a buffer for a given input file and ensure it is recorded in
  /// SourceMgr, PartialModules, or InputSourceCodeBufferIDs as appropriate.
  /// Return the buffer ID if it is not already compiled, or None if so.
  /// Set failed on failure.

  std::optional<unsigned> getRecordedBufferID(const InputFile &input,
                                              const bool shouldRecover,
                                              bool &failed);

  /// Given an input file, return a buffer to use for its contents,
  /// and a buffer for the corresponding module doc file if one exists.
  /// On failure, return a null pointer for the first element of the returned
  /// pair.
  std::optional<ModuleBuffers> getInputBuffersIfPresent(const InputFile &input);

  /// Try to open the module doc file corresponding to the input parameter.
  /// Return None for error, nullptr if no such file exists, or the buffer if
  /// one was found.
  std::optional<std::unique_ptr<toolchain::MemoryBuffer>>
  openModuleDoc(const InputFile &input);

  /// Try to open the module source info file corresponding to the input parameter.
  /// Return None for error, nullptr if no such file exists, or the buffer if
  /// one was found.
  std::optional<std::unique_ptr<toolchain::MemoryBuffer>>
  openModuleSourceInfo(const InputFile &input);

public:
  /// Parses and type-checks all input files.
  void performSema();

  /// Parses and performs import resolution on all input files.
  ///
  /// This is similar to a parse-only invocation, but module imports will also
  /// be processed.
  bool performParseAndResolveImportsOnly();

  /// Performs mandatory, diagnostic, and optimization passes over the SIL.
  /// \param silModule The SIL module that was generated during SILGen.
  /// \returns true if any errors occurred.
  bool performSILProcessing(SILModule *silModule);

private:
  /// Creates a new source file for the main module.
  SourceFile *createSourceFileForMainModule(ModuleDecl *mod,
                                            SourceFileKind FileKind,
                                            unsigned BufferID,
                                            bool isMainBuffer = false) const;

  /// Creates all the files to be added to the main module, appending them to
  /// \p files. If a loading error occurs, returns \c true.
  bool createFilesForMainModule(ModuleDecl *mod,
                                SmallVectorImpl<FileUnit *> &files) const;
  SourceFile *computeMainSourceFileForModule(ModuleDecl *mod) const;

public:
  void freeASTContext();

  /// If an implicit standard library import is expected, loads the standard
  /// library, returning \c false if we should continue, i.e. no error.
  bool loadStdlibIfNeeded();

  /// If \p fn returns true, exits early and returns true.
  bool forEachFileToTypeCheck(toolchain::function_ref<bool(SourceFile &)> fn);
  bool forEachSourceFile(toolchain::function_ref<bool(SourceFile &)> fn);

  /// Whether the cancellation of the current operation has been requested.
  bool isCancellationRequested() const;

private:
  /// Compute the parsing options for a source file in the main module.
  SourceFile::ParsingOptions getSourceFileParsingOptions(bool forPrimary) const;

  /// Retrieve a description of which modules should be implicitly imported.
  ImplicitImportInfo getImplicitImportInfo() const;

  /// For any serialized AST inputs, loads them in as partial module files,
  /// appending them to \p partialModules. If a loading error occurs, returns
  /// \c true.
  bool loadPartialModulesAndImplicitImports(
      ModuleDecl *mod, SmallVectorImpl<FileUnit *> &partialModules) const;

  void finishTypeChecking();

public:
  const PrimarySpecificPaths &
  getPrimarySpecificPathsForWholeModuleOptimizationMode() const;
  const PrimarySpecificPaths &
  getPrimarySpecificPathsForPrimary(StringRef filename) const;
  const PrimarySpecificPaths &
  getPrimarySpecificPathsForAtMostOnePrimary() const;
  const PrimarySpecificPaths &
  getPrimarySpecificPathsForSourceFile(const SourceFile &SF) const;
};

} // namespace language

#endif
