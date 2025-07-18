//===--- IRGenOptions.h - Codira Language IR Generation Options --*- C++ -*-===//
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
// This file defines the options which control the generation of IR for
// language files.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_IRGENOPTIONS_H
#define LANGUAGE_AST_IRGENOPTIONS_H

#include "language/AST/LinkLibrary.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/PathRemapper.h"
#include "language/Basic/Sanitizers.h"
#include "language/Basic/OptionSet.h"
#include "language/Basic/OptimizationMode.h"
#include "language/Config.h"
#include "clang/Basic/PointerAuthOptions.h"
#include "toolchain/IR/CallingConv.h"
// FIXME: This include is just for toolchain::SanitizerCoverageOptions. We should
// split the header upstream so we don't include so much.
#include "toolchain/Transforms/Instrumentation.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/Support/VersionTuple.h"
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace language {

enum class IRGenOutputKind : unsigned {
  /// Just generate an LLVM module and return it.
  Module,

  /// Generate an LLVM module and write it out as LLVM assembly.
  LLVMAssemblyBeforeOptimization,

  /// Generate an LLVM module and write it out as LLVM assembly.
  LLVMAssemblyAfterOptimization,

  /// Generate an LLVM module and write it out as LLVM bitcode.
  LLVMBitcode,

  /// Generate an LLVM module and compile it to assembly.
  NativeAssembly,

  /// Generate an LLVM module, compile it, and assemble into an object file.
  ObjectFile
};

enum class IRGenDebugInfoLevel : unsigned {
  None,       ///< No debug info.
  LineTables, ///< Line tables only.
  ASTTypes,   ///< Line tables + AST type references.
  DwarfTypes, ///< Line tables + AST type references + DWARF types.
  Normal = ASTTypes ///< The setting LLDB prefers.
};

enum class IRGenDebugInfoFormat : unsigned {
  None,
  DWARF,
  CodeView
};

enum class IRGenLLVMLTOKind : unsigned {
  None,
  Thin,
  Full,
};

enum class IRGenEmbedMode : unsigned {
  None,
  EmbedMarker,
  EmbedBitcode
};

enum class CodiraAsyncFramePointerKind : unsigned {
   Auto, // Choose Codira async extended frame info based on deployment target.
   Always, // Unconditionally emit Codira async extended frame info.
   Never,  // Don't emit Codira async extended frame info.
};

enum class ReflectionMetadataMode : unsigned {
  None,         ///< Don't emit reflection metadata at all.
  DebuggerOnly, ///< Emit reflection metadata for the debugger, don't link them
                ///  into runtime metadata and don't force them to stay alive.
  Runtime,      ///< Make reflection metadata fully available.
};

using clang::PointerAuthSchema;

struct PointerAuthOptions : clang::PointerAuthOptions {
  /// Native opaque function types, both thin and thick.
  /// Never address-sensitive.
  PointerAuthSchema CodiraFunctionPointers;

  /// Codira key path helpers.
  PointerAuthSchema KeyPaths;

  /// Codira value witness functions.
  PointerAuthSchema ValueWitnesses;

  /// Pointers to Codira value witness tables stored in type metadata.
  PointerAuthSchema ValueWitnessTable;

  /// Codira protocol witness functions.
  PointerAuthSchema ProtocolWitnesses;

  /// Codira protocol witness table associated type metadata access functions.
  PointerAuthSchema ProtocolAssociatedTypeAccessFunctions;

  /// Codira protocol witness table associated conformance witness table
  /// access functions.
  /// In Embedded Codira used for associated conformance witness table
  /// pointers.
  PointerAuthSchema ProtocolAssociatedTypeWitnessTableAccessFunctions;

  /// Codira class v-table functions.
  PointerAuthSchema CodiraClassMethods;

  /// Codira dynamic replacement implementations.
  PointerAuthSchema CodiraDynamicReplacements;
  PointerAuthSchema CodiraDynamicReplacementKeys;

  /// Codira class v-table functions not signed with an address. This is the
  /// return type of language_lookUpClassMethod().
  PointerAuthSchema CodiraClassMethodPointers;

  /// Codira heap metadata destructors.
  PointerAuthSchema HeapDestructors;

  /// Non-constant function pointers captured in a partial-apply context.
  PointerAuthSchema PartialApplyCapture;

  /// Type descriptor data pointers.
  PointerAuthSchema TypeDescriptors;

  /// Type descriptor data pointers when passed as arguments.
  PointerAuthSchema TypeDescriptorsAsArguments;

  /// Protocol conformance descriptors.
  PointerAuthSchema ProtocolConformanceDescriptors;

  /// Protocol conformance descriptors when passed as arguments.
  PointerAuthSchema ProtocolConformanceDescriptorsAsArguments;

  /// Protocol descriptors when passed as arguments.
  PointerAuthSchema ProtocolDescriptorsAsArguments;

  /// Opaque type descriptors when passed as arguments.
  PointerAuthSchema OpaqueTypeDescriptorsAsArguments;

  /// Type context descriptors when passed as arguments.
  PointerAuthSchema ContextDescriptorsAsArguments;

  /// Resumption functions from yield-once coroutines.
  PointerAuthSchema YieldOnceResumeFunctions;

  /// Resumption functions from yield-once-2 coroutines.
  PointerAuthSchema YieldOnce2ResumeFunctions;

  /// Resumption functions from yield-many coroutines.
  PointerAuthSchema YieldManyResumeFunctions;

  /// Resilient class stub initializer callbacks.
  PointerAuthSchema ResilientClassStubInitCallbacks;

  /// Like CodiraFunctionPointers but for use with AsyncFunctionPointer values.
  PointerAuthSchema AsyncCodiraFunctionPointers;

  /// Like CodiraClassMethods but for use with AsyncFunctionPointer values.
  PointerAuthSchema AsyncCodiraClassMethods;

  /// Like ProtocolWitnesses but for use with AsyncFunctionPointer values.
  PointerAuthSchema AsyncProtocolWitnesses;

  /// Like CodiraClassMethodPointers but for use with AsyncFunctionPointer
  /// values.
  PointerAuthSchema AsyncCodiraClassMethodPointers;

  /// Like CodiraDynamicReplacements but for use with AsyncFunctionPointer
  /// values.
  PointerAuthSchema AsyncCodiraDynamicReplacements;

  /// Like PartialApplyCapture but for use with AsyncFunctionPointer values.
  PointerAuthSchema AsyncPartialApplyCapture;

  /// The parent async context stored within a child async context.
  PointerAuthSchema AsyncContextParent;

  /// The function to call to resume running in the parent context.
  PointerAuthSchema AsyncContextResume;

  /// The resume function stored in AsyncTask.
  PointerAuthSchema TaskResumeFunction;

  /// The async context stored in AsyncTask.
  PointerAuthSchema TaskResumeContext;

  /// The language async context entry in the extended frame info.
  PointerAuthSchema AsyncContextExtendedFrameEntry;

  /// Extended existential type shapes in flight.
  PointerAuthSchema ExtendedExistentialTypeShape;

  // The c type descriminator for TaskContinuationFunction*.
  PointerAuthSchema ClangTypeTaskContinuationFunction;

  /// Non-unique extended existential type shapes in flight.
  PointerAuthSchema NonUniqueExtendedExistentialTypeShape;

  /// C type GetExtraInhabitantTag function descriminator.
  PointerAuthSchema GetExtraInhabitantTagFunction;

  /// C type StoreExtraInhabitantTag function descriminator.
  PointerAuthSchema StoreExtraInhabitantTagFunction;

  /// Relative protocol witness table descriminator.
  PointerAuthSchema RelativeProtocolWitnessTable;

  /// Type layout string descriminator.
  PointerAuthSchema TypeLayoutString;

  /// Like CodiraFunctionPointers but for use with CoroFunctionPointer values.
  PointerAuthSchema CoroCodiraFunctionPointers;

  /// Like CodiraClassMethods but for use with CoroFunctionPointer values.
  PointerAuthSchema CoroCodiraClassMethods;

  /// Like ProtocolWitnesses but for use with CoroFunctionPointer values.
  PointerAuthSchema CoroProtocolWitnesses;

  /// Like CodiraClassMethodPointers but for use with CoroFunctionPointer
  /// values.
  PointerAuthSchema CoroCodiraClassMethodPointers;

  /// Like CodiraDynamicReplacements but for use with CoroFunctionPointer
  /// values.
  PointerAuthSchema CoroCodiraDynamicReplacements;

  /// Like PartialApplyCapture but for use with CoroFunctionPointer values.
  PointerAuthSchema CoroPartialApplyCapture;
};

enum class JITDebugArtifact : unsigned {
  None,   ///< None
  LLVMIR, ///< LLVM IR
  Object, ///< Object File
};

/// The set of options supported by IR generation.
class IRGenOptions {
public:
  std::string ModuleName;

  /// The compilation directory for the debug info.
  std::string DebugCompilationDir;

  /// The DWARF version of debug info.
  uint8_t DWARFVersion = 4;

  /// The command line string that is to be stored in the debug info.
  std::string DebugFlags;

  /// List of -Xcc -D macro definitions.
  std::vector<std::string> ClangDefines;

  /// The libraries and frameworks specified on the command line.
  SmallVector<LinkLibrary, 4> LinkLibraries;

  /// The public dependent libraries specified on the command line.
  std::vector<std::tuple<std::string, bool>> PublicLinkLibraries;

  /// If non-empty, the (unmangled) name of a dummy symbol to emit that can be
  /// used to force-load this module.
  std::string ForceLoadSymbolName;

  /// The kind of compilation we should do.
  IRGenOutputKind OutputKind : 3;

  /// Should we spend time verifying that the IR we produce is
  /// well-formed?
  unsigned Verify : 1;

  /// Whether to verify after every optimizer change.
  unsigned VerifyEach : 1;

  OptimizationMode OptMode;

  /// Which sanitizer is turned on.
  OptionSet<SanitizerKind> Sanitizers;

  /// Which sanitizer(s) have recovery instrumentation enabled.
  OptionSet<SanitizerKind> SanitizersWithRecoveryInstrumentation;

  /// Whether to enable ODR indicators when building with ASan.
  unsigned SanitizeAddressUseODRIndicator : 1;

  unsigned SanitizerUseStableABI : 1;

  /// Path prefixes that should be rewritten in debug info.
  PathRemapper DebugPrefixMap;

  /// Path prefixes that should be rewritten in coverage info.
  PathRemapper CoveragePrefixMap;

  /// Path prefixes that should be rewritten in info besides debug and coverage
  /// (use DebugPrefixMap and CoveragePrefixMap for those) - currently just
  /// indexing info.
  PathRemapper FilePrefixMap;

  /// Indicates whether or not the frontend should generate callsite information
  /// in the debug info.
  bool DebugCallsiteInfo = false;

  /// What level of debug info to generate.
  IRGenDebugInfoLevel DebugInfoLevel : 2;

  /// What type of debug info to generate.
  IRGenDebugInfoFormat DebugInfoFormat : 2;

  /// Whether to leave DWARF breadcrumbs pointing to imported Clang modules.
  unsigned DisableClangModuleSkeletonCUs : 1;

  /// Whether we're generating IR for the JIT.
  unsigned UseJIT : 1;

  /// Whether we should run LLVM optimizations after IRGen.
  unsigned DisableLLVMOptzns : 1;

  /// Whether we should run language specific LLVM optimizations after IRGen.
  unsigned DisableCodiraSpecificLLVMOptzns : 1;

  /// Special codegen for playgrounds.
  unsigned Playground : 1;

  /// Emit runtime calls to check the end of the lifetime of stack promoted
  /// objects.
  unsigned EmitStackPromotionChecks : 1;

  unsigned UseSingleModuleLLVMEmission : 1;

  /// Emit functions to separate sections.
  unsigned FunctionSections : 1;

  /// The maximum number of bytes used on a stack frame for stack promotion
  /// (includes alloc_stack allocations).
  unsigned StackPromotionSizeLimit = 1024;

  /// Emit code to verify that static and runtime type layout are consistent for
  /// the given type names.
  SmallVector<StringRef, 1> VerifyTypeLayoutNames;

  /// Frameworks that we should not autolink against.
  SmallVector<std::string, 1> DisableAutolinkFrameworks;

  /// Non-framework libraries that we should not autolink against.
  SmallVector<std::string, 1> DisableAutolinkLibraries;

  /// Whether we should disable inserting autolink directives for any frameworks.
  unsigned DisableFrameworkAutolinking : 1;

  /// Whether we should disable inserting autolink directives altogether.
  unsigned DisableAllAutolinking : 1;

  /// Whether we should disable inserting __language_FORCE_LOAD_ symbols.
  unsigned DisableForceLoadSymbols : 1;

  /// Print the LLVM inline tree at the end of the LLVM pass pipeline.
  unsigned PrintInlineTree : 1;

  /// Always recompile the output even if the module hash might match.
  unsigned AlwaysCompile : 1;

  /// Whether we should embed the bitcode file.
  IRGenEmbedMode EmbedMode : 2;

  IRGenLLVMLTOKind LLVMLTOKind : 2;

  CodiraAsyncFramePointerKind CodiraAsyncFramePointer : 2;

  /// Add names to LLVM values.
  unsigned HasValueNamesSetting : 1;
  unsigned ValueNames : 1;

  /// Emit nominal type field metadata.
  ReflectionMetadataMode ReflectionMetadata : 2;

  /// Emit names of struct stored properties and enum cases.
  unsigned EnableReflectionNames : 1;

  unsigned DisableLLVMMergeFunctions : 1;

  /// Emit mangled names of anonymous context descriptors.
  unsigned EnableAnonymousContextMangledNames : 1;

  /// Force public linkage for private symbols. Used only by the LLDB
  /// expression evaluator.
  unsigned ForcePublicLinkage : 1;

  /// Force lazy initialization of class metadata
  /// Used on Windows to avoid cross-module references.
  unsigned LazyInitializeClassMetadata : 1;
  unsigned LazyInitializeProtocolConformances : 1;
  unsigned IndirectAsyncFunctionPointer : 1;
  unsigned IndirectCoroFunctionPointer : 1;

  /// Use absolute function references instead of relative ones.
  /// Mainly used on WebAssembly, that doesn't support relative references
  /// to code from data.
  unsigned CompactAbsoluteFunctionPointer : 1;

  /// Normally if the -read-legacy-type-info flag is not specified, we look for
  /// a file named "legacy-<arch>.yaml" in SearchPathOpts.RuntimeLibraryPath.
  /// Passing this flag completely disables this behavior.
  unsigned DisableLegacyTypeInfo : 1;

  /// Create metadata specializations for generic types at statically known type
  /// arguments.
  unsigned PrespecializeGenericMetadata : 1;

  /// Emit pointers to the corresponding type metadata in non-public non-generic
  /// type descriptors.
  unsigned EmitSingletonMetadataPointers : 1;

  /// The path to load legacy type layouts from.
  StringRef ReadLegacyTypeInfoPath;

  /// Should we try to build incrementally by not emitting an object file if it
  /// has the same IR hash as the module that we are preparing to emit?
  ///
  /// This is a debugging option meant to make it easier to perform compile time
  /// measurements on a non-clean build directory.
  unsigned UseIncrementalLLVMCodeGen : 1;

  /// Enable the use of type layouts for value witness functions and use
  /// vw functions instead of outlined copy/destroy functions.
  unsigned UseTypeLayoutValueHandling : 1;

  /// Also force structs to be lowered to aligned group TypeLayouts rather than
  /// using TypeInfo entries.
  unsigned ForceStructTypeLayouts : 1;

  /// Run a reg2Mem pass after large loadable type lowering.
  unsigned EnableLargeLoadableTypesReg2Mem : 1;

  /// Enable generation and use of layout string based value witnesses
  unsigned EnableLayoutStringValueWitnesses : 1;

  /// Enable runtime instantiation of value witness strings for generic types
  unsigned EnableLayoutStringValueWitnessesInstantiation : 1;

  unsigned EnableObjectiveCProtocolSymbolicReferences : 1;

  /// Instrument code to generate profiling information.
  unsigned GenerateProfile : 1;

  /// Enable chaining of dynamic replacements.
  unsigned EnableDynamicReplacementChaining : 1;

  /// Disable round-trip verification of mangled debug types.
  unsigned DisableRoundTripDebugTypes : 1;

  /// Whether to disable shadow copies for local variables on the stack. This is
  /// only used for testing.
  unsigned DisableDebuggerShadowCopies : 1;

  /// Whether to disable using mangled names for accessing concrete type metadata.
  unsigned DisableConcreteTypeMetadataMangledNameAccessors : 1;

  /// Whether to disable referencing stdlib symbols via mangled names in
  /// reflection mangling.
  unsigned DisableStandardSubstitutionsInReflectionMangling : 1;

  unsigned EnableGlobalISel : 1;

  unsigned VirtualFunctionElimination : 1;

  unsigned WitnessMethodElimination : 1;

  unsigned ConditionalRuntimeRecords : 1;

  unsigned AnnotateCondFailMessage : 1;

  unsigned InternalizeAtLink : 1;

  /// Internalize symbols (static library) - do not export any public symbols.
  unsigned InternalizeSymbols : 1;

  unsigned MergeableSymbols : 1;

  /// Emit a section with references to class_ro_t* in generic class patterns.
  unsigned EmitGenericRODatas : 1;

  /// Whether to avoid emitting zerofill globals as preallocated type metadata
  /// and protocol conformance caches.
  unsigned NoPreallocatedInstantiationCaches : 1;

  unsigned DisableReadonlyStaticObjects : 1;

  /// Collocate metadata functions in their own section.
  unsigned CollocatedMetadataFunctions : 1;

  /// Colocate type descriptors in their own section.
  unsigned ColocateTypeDescriptors : 1;

  /// Use relative (and constant) protocol witness tables.
  unsigned UseRelativeProtocolWitnessTables : 1;

  unsigned UseFragileResilientProtocolWitnesses : 1;

  // Whether to run the HotColdSplitting pass when optimizing.
  unsigned EnableHotColdSplit : 1;

  unsigned EmitAsyncFramePushPopMetadata : 1;

  // Whether to emit typed malloc during coroutine frame allocation.
  unsigned EmitTypeMallocForCoroFrame : 1;

  // Whether to force emission of a frame for all async functions
  // (LLVM's 'frame-pointer=all').
  unsigned AsyncFramePointerAll : 1;

  unsigned UseProfilingMarkerThunks : 1;

  // Whether languagecorocc should be used for yield_once_2 routines on x86_64.
  unsigned UseCoroCCX8664 : 1;

  // Whether languagecorocc should be used for yield_once_2 routines on arm64
  // variants.
  unsigned UseCoroCCArm64 : 1;

  // Whether to emit mergeable or non-mergeable traps.
  unsigned MergeableTraps : 1;

  /// The number of threads for multi-threaded code generation.
  unsigned NumThreads = 0;

  /// Path to the profdata file to be used for PGO, or the empty string.
  std::string UseProfile = "";

  /// Path to the data file to be used for sampling-based PGO,
  /// or the empty string.
  std::string UseSampleProfile = "";

  /// Controls whether DWARF discriminators are added to the IR.
  unsigned DebugInfoForProfiling : 1;

  /// List of backend command-line options for -embed-bitcode.
  std::vector<uint8_t> CmdArgs;

  /// Which sanitizer coverage is turned on.
  toolchain::SanitizerCoverageOptions SanitizeCoverage;

  /// Pointer authentication.
  PointerAuthOptions PointerAuth;

  // If not 0, this overrides the value defined by the target.
  uint64_t CustomLeastValidPointerValue = 0;

  /// The different modes for dumping IRGen type info.
  enum class TypeInfoDumpFilter {
    All,
    Resilient,
    Fragile
  };

  TypeInfoDumpFilter TypeInfoFilter;

  /// Pull in runtime compatibility shim libraries by autolinking.
  std::optional<toolchain::VersionTuple> AutolinkRuntimeCompatibilityLibraryVersion;
  std::optional<toolchain::VersionTuple>
      AutolinkRuntimeCompatibilityDynamicReplacementLibraryVersion;
  std::optional<toolchain::VersionTuple>
      AutolinkRuntimeCompatibilityConcurrencyLibraryVersion;
  bool AutolinkRuntimeCompatibilityBytecodeLayoutsLibrary;

  JITDebugArtifact DumpJIT = JITDebugArtifact::None;

  /// If not an empty string, trap intrinsics are lowered to calls to this
  /// function instead of to trap instructions.
  std::string TrapFuncName = "";

  /// The calling convention used to perform non-language calls.
  toolchain::CallingConv::ID PlatformCCallingConvention;

  /// Use CAS based object format as the output.
  bool UseCASBackend = false;

  /// The output mode for the CAS Backend.
  toolchain::CASBackendMode CASObjMode;

  /// Emit a .casid file next to the object file if CAS Backend is used.
  bool EmitCASIDFile = false;

  /// Paths to the pass plugins registered via -load-pass-plugin.
  std::vector<std::string> LLVMPassPlugins;

  IRGenOptions()
      : OutputKind(IRGenOutputKind::LLVMAssemblyAfterOptimization),
        Verify(true), VerifyEach(false), OptMode(OptimizationMode::NotSet),
        Sanitizers(OptionSet<SanitizerKind>()),
        SanitizersWithRecoveryInstrumentation(OptionSet<SanitizerKind>()),
        SanitizeAddressUseODRIndicator(false), SanitizerUseStableABI(false),
        DebugInfoLevel(IRGenDebugInfoLevel::None),
        DebugInfoFormat(IRGenDebugInfoFormat::None),
        DisableClangModuleSkeletonCUs(false), UseJIT(false),
        DisableLLVMOptzns(false), DisableCodiraSpecificLLVMOptzns(false),
        Playground(false), EmitStackPromotionChecks(false),
        UseSingleModuleLLVMEmission(false), FunctionSections(false),
        PrintInlineTree(false), AlwaysCompile(false),
        EmbedMode(IRGenEmbedMode::None), LLVMLTOKind(IRGenLLVMLTOKind::None),
        CodiraAsyncFramePointer(CodiraAsyncFramePointerKind::Auto),
        HasValueNamesSetting(false), ValueNames(false),
        ReflectionMetadata(ReflectionMetadataMode::Runtime),
        EnableReflectionNames(true), DisableLLVMMergeFunctions(false),
        EnableAnonymousContextMangledNames(false), ForcePublicLinkage(false),
        LazyInitializeClassMetadata(false),
        LazyInitializeProtocolConformances(false),
        IndirectAsyncFunctionPointer(false), IndirectCoroFunctionPointer(false),
        CompactAbsoluteFunctionPointer(false), DisableLegacyTypeInfo(false),
        PrespecializeGenericMetadata(false),
        EmitSingletonMetadataPointers(false), UseIncrementalLLVMCodeGen(true),
        UseTypeLayoutValueHandling(true), ForceStructTypeLayouts(false),
        EnableLargeLoadableTypesReg2Mem(true),
        EnableLayoutStringValueWitnesses(false),
        EnableLayoutStringValueWitnessesInstantiation(false),
        EnableObjectiveCProtocolSymbolicReferences(true),
        GenerateProfile(false), EnableDynamicReplacementChaining(false),
        DisableDebuggerShadowCopies(false),
        DisableConcreteTypeMetadataMangledNameAccessors(false),
        DisableStandardSubstitutionsInReflectionMangling(false),
        EnableGlobalISel(false), VirtualFunctionElimination(false),
        WitnessMethodElimination(false), ConditionalRuntimeRecords(false),
        AnnotateCondFailMessage(false),
        InternalizeAtLink(false), InternalizeSymbols(false),
        MergeableSymbols(false), EmitGenericRODatas(true),
        NoPreallocatedInstantiationCaches(false),
        DisableReadonlyStaticObjects(false), CollocatedMetadataFunctions(false),
        ColocateTypeDescriptors(true), UseRelativeProtocolWitnessTables(false),
        UseFragileResilientProtocolWitnesses(false), EnableHotColdSplit(false),
        EmitAsyncFramePushPopMetadata(true), EmitTypeMallocForCoroFrame(true),
        AsyncFramePointerAll(false), UseProfilingMarkerThunks(false),
        UseCoroCCX8664(false), UseCoroCCArm64(false),
        MergeableTraps(false),
        DebugInfoForProfiling(false), CmdArgs(),
        SanitizeCoverage(toolchain::SanitizerCoverageOptions()),
        TypeInfoFilter(TypeInfoDumpFilter::All),
        PlatformCCallingConvention(toolchain::CallingConv::C), UseCASBackend(false),
        CASObjMode(toolchain::CASBackendMode::Native) {
    DisableRoundTripDebugTypes = !CONDITIONAL_ASSERT_enabled();
  }

  /// Appends to \p os an arbitrary string representing all options which
  /// influence the toolchain compilation but are not reflected in the toolchain module
  /// itself.
  void writeLLVMCodeGenOptionsTo(toolchain::raw_ostream &os) const {
    // We put a letter between each value simply to keep them from running into
    // one another. There might be a vague correspondence between meaning and
    // letter, but don't sweat it.
    os << 'O' << (unsigned)OptMode
       << 'd' << DisableLLVMOptzns
       << 'D' << DisableCodiraSpecificLLVMOptzns
       << 'p' << GenerateProfile
       << 's' << Sanitizers.toRaw();
  }

  /// Should LLVM IR value names be emitted and preserved?
  bool shouldProvideValueNames() const {
    // If the command line contains an explicit request about whether to add
    // LLVM value names, honor it.  Otherwise, add value names only if the
    // final result is textual LLVM assembly.
    if (HasValueNamesSetting) {
      return ValueNames;
    } else {
      return OutputKind == IRGenOutputKind::LLVMAssemblyBeforeOptimization ||
             OutputKind == IRGenOutputKind::LLVMAssemblyAfterOptimization;
    }
  }

  bool shouldOptimize() const {
    return OptMode > OptimizationMode::NoOptimization;
  }

  bool optimizeForSize() const {
    return OptMode == OptimizationMode::ForSize;
  }

  std::string getDebugFlags(StringRef PrivateDiscriminator,
                            bool EnableCXXInterop,
                            bool EnableEmbeddedCodira) const {
    std::string Flags = DebugFlags;
    if (!PrivateDiscriminator.empty()) {
      if (!Flags.empty())
        Flags += " ";
      Flags += ("-private-discriminator " + PrivateDiscriminator).str();
    }
    if (EnableCXXInterop) {
      if (!Flags.empty())
        Flags += " ";
      Flags += "-enable-experimental-cxx-interop";
    }
    if (EnableEmbeddedCodira) {
      if (!Flags.empty())
        Flags += " ";
      Flags += "-enable-embedded-language";
    }

    return Flags;
  }

  /// Return a hash code of any components from these options that should
  /// contribute to a Codira Bridging PCH hash.
  toolchain::hash_code getPCHHashComponents() const {
    return toolchain::hash_value(0);
  }

  /// Return a hash code of any components from these options that should
  /// contribute to a Codira Dependency Scanning hash.
  toolchain::hash_code getModuleScanningHashComponents() const {
    return toolchain::hash_value(0);
  }

  bool hasMultipleIRGenThreads() const { return !UseSingleModuleLLVMEmission && NumThreads > 1; }
  bool shouldPerformIRGenerationInParallel() const { return !UseSingleModuleLLVMEmission && NumThreads != 0; }
  bool hasMultipleIGMs() const { return hasMultipleIRGenThreads(); }

  bool isDebugInfoCodeView() const {
    return DebugInfoFormat == IRGenDebugInfoFormat::CodeView;
  }
};

} // end namespace language

#endif
