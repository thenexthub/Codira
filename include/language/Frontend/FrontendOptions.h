//===--- FrontendOptions.h --------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_FRONTEND_FRONTENDOPTIONS_H
#define LANGUAGE_FRONTEND_FRONTENDOPTIONS_H

#include "language/Basic/FileTypes.h"
#include "language/Basic/PathRemapper.h"
#include "language/Basic/Version.h"
#include "language/Frontend/FrontendInputsAndOutputs.h"
#include "language/Frontend/InputFile.h"
#include "clang/CAS/CASOptions.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/MC/MCTargetOptions.h"
#include <optional>

#include <set>
#include <string>
#include <vector>

namespace toolchain {
  class MemoryBuffer;
}

namespace language {
enum class IntermoduleDepTrackingMode;

/// Options for controlling the behavior of the frontend.
class FrontendOptions {
  friend class ArgsToFrontendOptionsConverter;
public:

  /// A list of arbitrary modules to import and make implicitly visible.
  std::vector<std::pair<std::string, bool /*testable*/>>
      ImplicitImportModuleNames;

  FrontendInputsAndOutputs InputsAndOutputs;

  void forAllOutputPaths(const InputFile &input,
                         toolchain::function_ref<void(StringRef)> fn) const;

  bool isOutputFileDirectory() const;

  /// An Objective-C header to import and make implicitly visible.
  std::string ImplicitObjCHeaderPath;

  /// An Objective-C pch to import and make implicitly visible.
  std::string ImplicitObjCPCHPath;

  /// The map of aliases and real names of imported or referenced modules.
  toolchain::StringMap<std::string> ModuleAliasMap;

  /// The name of the module that the frontend is building.
  std::string ModuleName;

  /// The ABI name of the module that the frontend is building, to be used in
  /// mangling and metadata.
  std::string ModuleABIName;

  /// The name of the library to link against when using this module.
  std::string ModuleLinkName;

  /// Module name to use when referenced in clients module interfaces.
  std::string ExportAsName;

  /// The public facing name of the module to build.
  std::string PublicModuleName;

  /// Arguments which should be passed in immediate mode.
  std::vector<std::string> ImmediateArgv;

  /// A list of arguments to forward to LLVM's option processing; this
  /// should only be used for debugging and experimental features.
  std::vector<std::string> LLVMArgs;

  /// The path to output language interface files for the compiled source files.
  std::string DumpAPIPath;

  /// The path to collect the group information for the compiled source files.
  std::string GroupInfoPath;

  /// The path to which we should store indexing data, if any.
  std::string IndexStorePath;

  /// The path to load access notes from.
  std::string AccessNotesPath;

  /// The path to look in when loading a module interface file, to see if a
  /// binary module has already been built for use by the compiler.
  std::string PrebuiltModuleCachePath;

  /// The path to output explicit module dependencies. Only relevant during
  /// dependency scanning.
  std::string ExplicitModulesOutputPath;

  /// The path to output explicitly-built SDK module dependencies. Only relevant during
  /// dependency scanning.
  std::string ExplicitSDKModulesOutputPath;

  /// The path to look in to find backup .codeinterface files if those found
  /// from SDKs are failing.
  std::string BackupModuleInterfaceDir;

  /// For these modules, we should prefer using Codira interface when importing them.
  std::vector<std::string> PreferInterfaceForModules;

  /// User-defined module version number.
  toolchain::VersionTuple UserModuleVersion;

  /// The Codira compiler version number that would be used to synthesize
  /// languageinterface files and subsequently their languagemodules.
  version::Version CodiraInterfaceCompilerVersion;

  /// A set of modules allowed to import this module.
  std::set<std::string> AllowableClients;

  /// Emit index data for imported serialized language system modules.
  bool IndexSystemModules = false;

  /// Avoid emitting index data for imported clang modules (pcms).
  bool IndexIgnoreClangModules = false;

  /// If indexing system modules, don't index the stdlib.
  bool IndexIgnoreStdlib = false;

  /// Include local definitions/references in the index data.
  bool IndexIncludeLocals = false;

  bool SerializeDebugInfoSIL = false;
  /// If building a module from interface, ignore compiler flags
  /// specified in the languageinterface.
  bool ExplicitInterfaceBuild = false;

  /// The module for which we should verify all of the generic signatures.
  std::string VerifyGenericSignaturesInModule;

  /// CacheReplay PrefixMap.
  std::vector<std::string> CacheReplayPrefixMap;

  /// Number of retry opening an input file if the previous opening returns
  /// bad file descriptor error.
  unsigned BadFileDescriptorRetryCount = 0;
  enum class ActionType {
    NoneAction,        ///< No specific action
    Parse,             ///< Parse only
    ResolveImports,    ///< Parse and resolve imports only
    Typecheck,         ///< Parse and type-check only
    DumpParse,         ///< Parse only and dump AST
    DumpInterfaceHash, ///< Parse and dump the interface token hash.
    DumpAST,           ///< Parse, type-check, and dump AST
    PrintAST,          ///< Parse, type-check, and pretty-print AST
    PrintASTDecl,      ///< Parse, type-check, and pretty-print AST declarations

    /// Parse and dump scope map.
    DumpScopeMaps,

    /// Parse, type-check, and dump availability scopes
    DumpAvailabilityScopes,

    EmitImportedModules, ///< Emit the modules that this one imports
    EmitPCH,             ///< Emit PCH of imported bridging header

    EmitSILGen, ///< Emit raw SIL
    EmitSIL,    ///< Emit canonical SIL

    EmitModuleOnly, ///< Emit module only
    MergeModules,   ///< Merge modules only

    /// Build from a languageinterface, as close to `import` as possible
    CompileModuleFromInterface,
    /// Same as CompileModuleFromInterface, but stopping after typechecking
    TypecheckModuleFromInterface,

    EmitSIBGen, ///< Emit serialized AST + raw SIL
    EmitSIB,    ///< Emit serialized AST + canonical SIL

    Immediate, ///< Immediate mode
    REPL,      ///< REPL mode

    EmitAssembly,   ///< Emit assembly
    EmitLoweredSIL, ///< Emit lowered SIL before IRGen runs
    EmitIRGen,      ///< Emit LLVM IR before LLVM optimizations
    EmitIR,         ///< Emit LLVM IR after LLVM optimizations
    EmitBC,         ///< Emit LLVM BC
    EmitObject,     ///< Emit object file

    DumpTypeInfo, ///< Dump IRGen type info

    EmitPCM, ///< Emit precompiled Clang module from a module map
    DumpPCM, ///< Dump information about a precompiled Clang module

    ScanDependencies, ///< Scan dependencies of Codira source files
    PrintVersion,     ///< Print version information.
    PrintArguments,   ///< Print supported arguments of this compiler
  };

  /// Indicates the action the user requested that the frontend perform.
  ActionType RequestedAction = ActionType::NoneAction;

  enum class ParseInputMode {
    Codira,
    CodiraLibrary,
    CodiraModuleInterface,
    SIL,
  };
  ParseInputMode InputMode = ParseInputMode::Codira;

  /// Indicates that the input(s) should be parsed as the Codira stdlib.
  bool ParseStdlib = false;

  /// Ignore .codesourceinfo file when trying to get source locations from module imported decls.
  bool IgnoreCodiraSourceInfo = false;

  /// When true, emitted module files will always contain options for the
  /// debugger to use. When unset, the options will only be present if the
  /// module appears to not be a public module.
  std::optional<bool> SerializeOptionsForDebugging;

  /// When true the debug prefix map entries will be applied to debugging
  /// options before serialization. These can be reconstructed at debug time by
  /// applying the inverse map in SearchPathOptions.SearchPathRemapper.
  bool DebugPrefixSerializedDebuggingOptions = false;

  /// When true, check if all required CodiraOnoneSupport symbols are present in
  /// the module.
  bool CheckOnoneSupportCompleteness = false;

  /// The path to which we should output statistics files.
  std::string StatsOutputDir;

  /// Whether to enable timers tracking individual requests. This adds some
  /// runtime overhead.
  bool FineGrainedTimers = false;

  /// Whether we are printing all stats even if they are zero.
  bool PrintZeroStats = false;

  /// Trace changes to stats to files in StatsOutputDir.
  bool TraceStats = false;

  /// Profile changes to stats to files in StatsOutputDir.
  bool ProfileEvents = false;

  /// Profile changes to stats to files in StatsOutputDir, grouped by source
  /// entity.
  bool ProfileEntities = false;

  /// Emit parseable-output directly from the frontend, instead of relying
  /// the driver to emit it. This is used in context where frontend jobs are executed by
  /// clients other than the driver.
  bool FrontendParseableOutput = false;

  /// Indicates whether or not an import statement can pick up a Codira source
  /// file (as opposed to a module file).
  bool EnableSourceImport = false;

  /// Indicates whether we are compiling for testing.
  ///
  /// \see ModuleDecl::isTestingEnabled
  bool EnableTesting = false;

  /// Indicates whether we are compiling for private imports.
  ///
  /// \see ModuleDecl::arePrivateImportsEnabled
  bool EnablePrivateImports = false;

  /// Indicates whether we add implicit dynamic.
  ///
  /// \see ModuleDecl::isImplicitDynamicEnabled
  bool EnableImplicitDynamic = false;

  /// Enables the "fully resilient" resilience strategy.
  ///
  /// \see ResilienceStrategy::Resilient
  bool EnableLibraryEvolution = false;

  /// If set, this module is part of a mixed Objective-C/Codira framework, and
  /// the Objective-C half should implicitly be visible to the Codira sources.
  bool ImportUnderlyingModule = false;

  /// If set, the header provided in ImplicitObjCHeaderPath will be rewritten
  /// by the Clang importer as part of semantic analysis.
  bool ModuleHasBridgingHeader = false;

  /// Indicates whether or not the frontend should print statistics upon
  /// termination.
  bool PrintStats = false;

  /// Indicates whether or not the Clang importer should print statistics upon
  /// termination.
  bool PrintClangStats = false;

  /// Indicates whether or not the Clang importer should dump lookup tables
  /// upon termination.
  bool DumpClangLookupTables = false;

  /// Indicates whether standard help should be shown.
  bool PrintHelp = false;

  /// Indicates whether full help (including "hidden" options) should be shown.
  bool PrintHelpHidden = false;

  /// Indicates that the frontend should print the target triple and then
  /// exit.
  bool PrintTargetInfo = false;

  /// Indicates that the frontend should print the supported features and then
  /// exit.
  bool PrintSupportedFeatures = false;

  /// See the \ref SILOptions.EmitVerboseSIL flag.
  bool EmitVerboseSIL = false;

  /// See the \ref SILOptions.EmitSortedSIL flag.
  bool EmitSortedSIL = false;

  /// Specifies the collection mode for the intermodule dependency tracker.
  /// Note that if set, the dependency tracker will be enabled even if no
  /// output path is configured.
  std::optional<IntermoduleDepTrackingMode> IntermoduleDependencyTracking;

  /// Should we emit the cType when printing @convention(c) or no?
  bool PrintFullConvention = false;

  /// Should we serialize the hashes of dependencies (vs. the modification
  /// times) when compiling a module interface?
  bool SerializeModuleInterfaceDependencyHashes = false;

  /// Should we warn if an imported module needed to be rebuilt from a
  /// module interface file?
  bool RemarkOnRebuildFromModuleInterface = false;

  /// Should we lock .codeinterface while generating .codemodule from it?
  bool DisableInterfaceFileLock = false;

  /// Should we enable the dependency verifier for all primary files known to this frontend?
  bool EnableIncrementalDependencyVerifier = false;

  /// The path of the language-frontend executable.
  std::string MainExecutablePath;

  /// The directory path we should use when print #include for the bridging header.
  /// By default, we include ImplicitObjCHeaderPath directly.
  std::optional<std::string> BridgingHeaderDirForPrint;

  /// Disable implicitly-built Codira modules because they are explicitly
  /// built and provided to the compiler invocation.
  bool DisableImplicitModules = false;

  /// Disable building Codira modules from textual interfaces. This should be
  /// for testing purposes only.
  bool DisableBuildingInterface = false;

  /// Is this frontend configuration of an interface dependency scan sub-invocation
  bool DependencyScanningSubInvocation = false;

  /// When performing a dependency scanning action, only identify and output all imports
  /// of the main Codira module's source files.
  bool ImportPrescan = false;

  /// After performing a dependency scanning action, serialize the scanner's internal state.
  bool SerializeDependencyScannerCache = false;

  /// Load and re-use a prior serialized dependency scanner cache.
  bool ReuseDependencyScannerCache = false;

  /// Upon loading a prior serialized dependency scanner cache, filter out
  /// dependency module information which is no longer up-to-date with respect
  /// to input files of every given module.
  bool ValidatePriorDependencyScannerCache = false;

  /// The path at which to either serialize or deserialize the dependency scanner cache.
  std::string SerializedDependencyScannerCachePath;

  /// Emit remarks indicating use of the serialized module dependency scanning cache.
  bool EmitDependencyScannerCacheRemarks = false;

  /// The path at which the dependency scanner can write generated files.
  std::string ScannerOutputDir;

  /// If the scanner output is written directly to the disk for debugging.
  bool WriteScannerOutput = false;

  /// Whether the dependency scanner invocation should resolve imports
  /// to filesystem modules in parallel.
  bool ParallelDependencyScan = true;

  /// When performing an incremental build, ensure that cross-module incremental
  /// build metadata is available in any language modules emitted by this frontend
  /// job.
  ///
  /// This flag is currently only propagated from the driver to
  /// any merge-modules jobs.
  bool DisableCrossModuleIncrementalBuild = false;

  /// Best effort to output a .codemodule regardless of any compilation
  /// errors. SIL generation and serialization is skipped entirely when there
  /// are errors. The resulting serialized AST may include errors types and
  /// skip nodes entirely, depending on the errors involved.
  bool AllowModuleWithCompilerErrors = false;

  /// Whether or not the compiler must be strict in ensuring that implicit downstream
  /// module dependency build tasks must inherit the parent compiler invocation's context,
  /// such as `-Xcc` flags, etc.
  bool StrictImplicitModuleContext = false;

  /// Downgrade all errors emitted in the module interface verification phase
  /// to warnings.
  /// TODO: remove this after we fix all project-side warnings in the interface.
  bool DowngradeInterfaceVerificationError = false;

  /// True if the "-static" option is set.
  bool Static = false;

  /// True if building with -experimental-hermetic-seal-at-link. Turns on
  /// dead-stripping optimizations assuming that all users of library code
  /// are present at LTO time.
  bool HermeticSealAtLink = false;

  /// Disable using the sandbox when executing subprocesses.
  bool DisableSandbox = false;

  /// The different modes for validating TBD against the LLVM IR.
  enum class TBDValidationMode {
    Default,        ///< Do the default validation for the current platform.
    None,           ///< Do no validation.
    MissingFromTBD, ///< Only check for symbols that are in IR but not TBD.
    All, ///< Check for symbols that are in IR but not TBD and TBD but not IR.
  };

  /// Compare the symbols in the IR against the TBD file we would generate.
  TBDValidationMode ValidateTBDAgainstIR = TBDValidationMode::Default;

  /// An enum with different modes for automatically crashing at defined times.
  enum class DebugCrashMode {
    None, ///< Don't automatically crash.
    AssertAfterParse, ///< Automatically assert after parsing.
    CrashAfterParse, ///< Automatically crash after parsing.
  };

  /// Indicates a debug crash mode for the frontend.
  DebugCrashMode CrashMode = DebugCrashMode::None;

  /// Line and column for each of the locations to be probed by
  /// -dump-scope-maps.
  SmallVector<std::pair<unsigned, unsigned>, 2> DumpScopeMapLocations;

  /// The possible output formats supported for dumping ASTs.
  enum class ASTFormat {
    Default,                ///< S-expressions for debugging
    DefaultWithDeclContext, ///< S-expressions with DeclContext hierarchy
    JSON,                   ///< Structured JSON for analysis
    JSONZlib,               ///< Like JSON, but zlib-compressed
  };

  /// The output format generated by the `-dump-ast` flag.
  ASTFormat DumpASTFormat = ASTFormat::Default;

  /// Determines whether the static or shared resource folder is used.
  /// When set to `true`, the default resource folder will be set to
  /// '.../lib/language', otherwise '.../lib/language_static'.
  bool UseSharedResourceFolder = true;

  enum class ClangHeaderExposeBehavior {
    /// Expose all public declarations in the generated header.
    AllPublic,
    /// Expose declarations only when they have expose attribute.
    HasExposeAttr,
    /// Expose declarations only when they have expose attribute or are the
    /// implicitly exposed Stdlib declarations.
    HasExposeAttrOrImplicitDeps
  };

  /// Indicates which declarations should be exposed in the generated clang
  /// header.
  std::optional<ClangHeaderExposeBehavior> ClangHeaderExposedDecls;

  struct ClangHeaderExposedImportedModule {
    std::string moduleName;
    std::string headerName;
  };

  /// Indicates which imported modules have a generated header associated with
  /// them that can be imported into the generated header for the current
  /// module.
  std::vector<ClangHeaderExposedImportedModule> clangHeaderExposedImports;

  /// \return true if the given action only parses without doing other compilation steps.
  static bool shouldActionOnlyParse(ActionType);

  /// \return true if the given action requires the standard library to be
  /// loaded before it is run.
  static bool doesActionRequireCodiraStandardLibrary(ActionType);

  /// \return true if the given action requires input files to be provided.
  static bool doesActionRequireInputs(ActionType action);

  /// \return true if the given action requires input files to be provided.
  static bool doesActionPerformEndOfPipelineActions(ActionType action);

  /// \return true if the given action supports caching.
  static bool supportCompilationCaching(ActionType action);

  /// Return a hash code of any components from these options that should
  /// contribute to a Codira Bridging PCH hash.
  toolchain::hash_code getPCHHashComponents() const {
    return toolchain::hash_value(0);
  }

  /// Return a hash code of any components from these options that should
  /// contribute to a Codira Dependency Scanning hash.
  toolchain::hash_code getModuleScanningHashComponents() const {
    return hash_combine(ModuleName,
                        ModuleABIName,
                        ModuleLinkName,
                        ImplicitObjCHeaderPath,
                        PrebuiltModuleCachePath,
                        UserModuleVersion);
  }

  StringRef determineFallbackModuleName() const;

  bool isCompilingExactlyOneCodiraFile() const {
    return InputsAndOutputs.hasSingleInput() &&
           InputMode == ParseInputMode::Codira;
  }

  const PrimarySpecificPaths &
  getPrimarySpecificPathsForAtMostOnePrimary() const;
  const PrimarySpecificPaths &
      getPrimarySpecificPathsForPrimary(StringRef) const;

  /// Retrieves the list of arbitrary modules to import and make implicitly
  /// visible.
  ArrayRef<std::pair<std::string, bool /*testable*/>>
  getImplicitImportModuleNames() const {
    return ImplicitImportModuleNames;
  }

  /// Whether we're configured to track system intermodule dependencies.
  bool shouldTrackSystemDependencies() const;

  /// Whether we are configured with -typecheck or -typecheck-module-from-interface actuin
  bool isTypeCheckAction() const;

  /// Whether to emit symbol graphs for the output module.
  bool EmitSymbolGraph = false;

  /// The directory to which we should emit a symbol graph JSON files.
  /// It is valid whenever there are any inputs.
  ///
  /// These are JSON file that describes the public interface of a module for
  /// curating documentation, separated into files for each module this module
  /// extends.
  ///
  /// \sa SymbolGraphASTWalker
  std::string SymbolGraphOutputDir;

  /// Whether to emit doc comment information in symbol graphs for symbols
  /// which are inherited through classes or default implementations.
  bool SkipInheritedDocs = false;

  /// Whether to include symbols with SPI information in the symbol graph.
  bool IncludeSPISymbolsInSymbolGraph = false;

  /// Whether to reuse a frontend (i.e. compiler instance) for multiple
  /// compilations. This prevents ASTContext being freed.
  bool ReuseFrontendForMultipleCompilations = false;

  /// This is used to obfuscate the serialized search paths so we don't have
  /// to encode the actual paths into the .codemodule file.
  PathObfuscator serializedPathObfuscator;

  /// Whether to run the job twice to check determinism.
  bool DeterministicCheck = false;

  /// Avoid printing actual module content into the ABI descriptor file.
  /// This should only be used as a workaround when emitting ABI descriptor files
  /// crashes the compiler.
  bool emptyABIDescriptor = false;

  /// Augment modular imports in any emitted ObjC headers with equivalent
  /// textual imports
  bool EmitClangHeaderWithNonModularIncludes = false;

  /// All block list configuration files to be honored in this compilation.
  std::vector<std::string> BlocklistConfigFilePaths;

  struct CustomAvailabilityDomains {
    /// Domains defined with `-define-enabled-availability-domain=`.
    toolchain::SmallVector<std::string> EnabledDomains;
    /// Domains defined with `-define-disabled-availability-domain=`.
    toolchain::SmallVector<std::string> DisabledDomains;
    /// Domains defined with `-define-dynamic-availability-domain=`.
    toolchain::SmallVector<std::string> DynamicDomains;
  };

  /// The collection of AvailabilityDomain definitions specified as arguments.
  CustomAvailabilityDomains AvailabilityDomains;

private:
  static bool canActionEmitDependencies(ActionType);
  static bool canActionEmitReferenceDependencies(ActionType);
  static bool canActionEmitClangHeader(ActionType);
  static bool canActionEmitLoadedModuleTrace(ActionType);
  static bool canActionEmitModule(ActionType);
  static bool canActionEmitModuleDoc(ActionType);
  static bool canActionEmitModuleSummary(ActionType);
  static bool canActionEmitInterface(ActionType);
  static bool canActionEmitABIDescriptor(ActionType);
  static bool canActionEmitAPIDescriptor(ActionType);
  static bool canActionEmitConstValues(ActionType);
  static bool canActionEmitModuleSemanticInfo(ActionType);

public:
  static bool doesActionGenerateSIL(ActionType);
  static bool doesActionGenerateIR(ActionType);
  static bool doesActionProduceOutput(ActionType);
  static bool doesActionProduceTextualOutput(ActionType);
  static bool doesActionBuildModuleFromInterface(ActionType);
  static bool needsProperModuleName(ActionType);
  static file_types::ID formatForPrincipalOutputFileForAction(ActionType);
};

}

#endif
