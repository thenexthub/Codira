//===--- ASTContext.h - AST Context Object ----------------------*- C++ -*-===//
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
// This file defines the ASTContext interface.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_ASTCONTEXT_H
#define LANGUAGE_AST_ASTCONTEXT_H

#include "language/AST/ASTAllocated.h"
#include "language/AST/AvailabilityRange.h"
#include "language/AST/Evaluator.h"
#include "language/AST/Identifier.h"
#include "language/AST/Import.h"
#include "language/AST/ProtocolConformanceOptions.h"
#include "language/AST/SILOptions.h"
#include "language/AST/SearchPathOptions.h"
#include "language/AST/Type.h"
#include "language/AST/TypeAlignments.h"
#include "language/AST/Types.h"
#include "language/Basic/BlockList.h"
#include "language/Basic/CASOptions.h"
#include "language/Basic/LangOptions.h"
#include "language/Basic/Located.h"
#include "language/Basic/Malloc.h"
#include "language/Serialization/SerializationOptions.h"
#include "language/SymbolGraphGen/SymbolGraphOptions.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Basic/DarwinSDKInfo.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/MapVector.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringSet.h"
#include "toolchain/ADT/TinyPtrVector.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/DataTypes.h"
#include "toolchain/Support/VersionTuple.h"
#include "toolchain/Support/VirtualOutputBackend.h"
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace clang {
  class Decl;
  class MacroInfo;
  class Module;
  class ObjCInterfaceDecl;
}

namespace toolchain {
  class LLVMContext;
}

namespace language {
  class ABIAttr;
  class AbstractFunctionDecl;
  class ASTContext;
  enum class Associativity : unsigned char;
  class AvailabilityDomain;
  class AvailabilityMacroMap;
  class AvailabilityRange;
  class BoundGenericType;
  class BuiltinTupleDecl;
  class ClangModuleLoader;
  class ClangNode;
  class ClangTypeConverter;
  class ConcreteDeclRef;
  class ConstructorDecl;
  class Decl;
  class DeclContext;
  class DefaultArgumentInitializer;
  class DerivativeAttr;
  class DifferentiableAttr;
  class ExtensionDecl;
  struct ExternalSourceLocs;
  class ForeignRepresentationInfo;
  class FuncDecl;
  class GenericContext;
  class GenericSignature;
  class InFlightDiagnostic;
  class IterableDeclContext;
  class LazyContextData;
  class LazyIterableDeclContextData;
  class LazyMemberLoader;
  struct MacroDiscriminatorContext;
  class ModuleInterfaceChecker;
  class PatternBindingDecl;
  class PatternBindingInitializer;
  class PluginLoader;
  class SourceFile;
  class SourceLoc;
  struct TemplateInstantiationError;
  class Type;
  class TypeVariableType;
  class TupleType;
  class FunctionType;
  class ArchetypeType;
  class Identifier;
  class InheritedNameSet;
  class ModuleDecl;
  class PackageUnit;
  class ModuleDependenciesCache;
  class ModuleLoader;
  class NominalTypeDecl;
  class NormalProtocolConformance;
  class OpaqueTypeDecl;
  class InheritedProtocolConformance;
  class RootProtocolConformance;
  class SelfProtocolConformance;
  class SpecializedProtocolConformance;
  enum class BuiltinConformanceKind;
  class BuiltinProtocolConformance;
  enum class ProtocolConformanceState;
  class Pattern;
  enum PointerTypeKind : unsigned;
  class PrecedenceGroupDecl;
  struct RequirementMatch;
  class TupleTypeElt;
  class EnumElementDecl;
  class ProtocolDecl;
  class SubstitutableType;
  class SourceManager;
  class ValueDecl;
  class DiagnosticEngine;
  struct RawComment;
  class DocComment;
  class SILBoxType;
  class SILTransform;
  class TypeAliasDecl;
  class VarDecl;
  class UnifiedStatsReporter;
  class IndexSubset;
  struct SILAutoDiffDerivativeFunctionKey;
  struct InterfaceSubContextDelegate;

  enum class KnownProtocolKind : uint8_t;

namespace namelookup {
  class ImportCache;
}

namespace rewriting {
  class RewriteContext;
}

namespace ide {
  class TypeCheckCompletionCallback;
}

/// Lists the set of "known" Foundation entities that are used in the
/// compiler.
///
/// While the names of Foundation types aren't likely to change in
/// Objective-C, their mapping into Codira can. Therefore, when
/// referring to names of Foundation entities in Codira, use this enum
/// and \c language::getCodiraName or \c ASTContext::getCodiraId.
enum class KnownFoundationEntity {
#define FOUNDATION_ENTITY(Name) Name,
#include "language/AST/KnownFoundationEntities.def"
};

/// Retrieve the Foundation entity kind for the given Objective-C
/// entity name.
std::optional<KnownFoundationEntity> getKnownFoundationEntity(StringRef name);

/// Retrieve the Codira name for the given Foundation entity, where
/// "NS" prefix stripping will apply under omit-needless-words.
StringRef getCodiraName(KnownFoundationEntity kind);

/// Introduces a new constraint checker arena, whose lifetime is
/// tied to the lifetime of this RAII object.
class ConstraintCheckerArenaRAII {
  ASTContext &Self;
  void *Data;

public:
  /// Introduces a new constraint checker arena, supplanting any
  /// existing constraint checker arena.
  ///
  /// \param self The ASTContext into which this constraint checker arena
  /// will be installed.
  ///
  /// \param allocator The allocator used for allocating any data that
  /// goes into the constraint checker arena.
  ConstraintCheckerArenaRAII(ASTContext &self,
                             toolchain::BumpPtrAllocator &allocator);

  ConstraintCheckerArenaRAII(const ConstraintCheckerArenaRAII &) = delete;
  ConstraintCheckerArenaRAII(ConstraintCheckerArenaRAII &&) = delete;

  ConstraintCheckerArenaRAII &
  operator=(const ConstraintCheckerArenaRAII &) = delete;

  ConstraintCheckerArenaRAII &
  operator=(ConstraintCheckerArenaRAII &&) = delete;

  ~ConstraintCheckerArenaRAII();
};

class SILLayout; // From SIL

/// A set of missing witnesses for a given conformance. These are temporarily
/// stashed in the ASTContext so the type checker can get at them.
///
/// The only subclass is owned by the type checker, so it can hide its own
/// data structures.
class MissingWitnessesBase {
public:
  virtual ~MissingWitnessesBase();
};

/// Return value of ASTContext::getOpenedExistentialSignature().
struct OpenedExistentialSignature {
  /// The generalized existential type.
  ///
  /// The actual generic arguments of superclass and parameterized protocol
  /// types become fresh generic parameters in the generalization signature.
  CanType Shape;

  /// A substitution map sending each generic parameter of the generalization
  /// signature to the corresponding generic argument in the original
  /// existential type. May contain type variables.
  SubstitutionMap Generalization;

  /// The opened existential signature derived from the generalization signature.
  ///
  /// This is the generalization signature with one more generic parameter
  /// `Self` at the next highest depth, subject to the requirement
  /// `Self: Shape`, where `Shape` is above.
  CanGenericSignature OpenedSig;

  /// The `Self` parameter in the opened existential signature.
  CanType SelfType;
};

/// ASTContext - This object creates and owns the AST objects.
/// However, this class does more than just maintain context within an AST.
/// It is the closest thing to thread-local or compile-local storage in this
/// code base. Why? SourceKit uses this code with multiple threads per Unix
/// process. Each thread processes a different source file. Each thread has its
/// own instance of ASTContext, and that instance persists for the duration of
/// the thread, throughout all phases of the compilation. (The name "ASTContext"
/// is a bit of a misnomer here.) Why not use thread-local storage? This code
/// may use DispatchQueues and pthread-style TLS won't work with code that uses
/// DispatchQueues. Summary: if you think you need a global or static variable,
/// you probably need to put it here instead.

class ASTContext final {
  ASTContext(const ASTContext&) = delete;
  void operator=(const ASTContext&) = delete;

  ASTContext(
      LangOptions &langOpts, TypeCheckerOptions &typecheckOpts,
      SILOptions &silOpts, SearchPathOptions &SearchPathOpts,
      ClangImporterOptions &ClangImporterOpts,
      symbolgraphgen::SymbolGraphOptions &SymbolGraphOpts, CASOptions &casOpts,
      SerializationOptions &serializationOpts, SourceManager &SourceMgr,
      DiagnosticEngine &Diags,
      toolchain::IntrusiveRefCntPtr<toolchain::vfs::OutputBackend> OutBackend = nullptr);

public:
  // Members that should only be used by ASTContext.cpp.
  struct Implementation;
  Implementation &getImpl() const;

  struct GlobalCache;

  /// Retrieve a reference to the global cache within this ASTContext,
  /// which is a place where we can stash side tables without having to
  /// recompile the world every time we add a side table. See
  /// "language/AST/ASTContextGlobalCache.h"
  GlobalCache &getGlobalCache() const;

  friend ConstraintCheckerArenaRAII;

  void operator delete(void *Data) throw();

  static ASTContext *
  get(LangOptions &langOpts, TypeCheckerOptions &typecheckOpts,
      SILOptions &silOpts, SearchPathOptions &SearchPathOpts,
      ClangImporterOptions &ClangImporterOpts,
      symbolgraphgen::SymbolGraphOptions &SymbolGraphOpts, CASOptions &casOpts,
      SerializationOptions &serializationOpts, SourceManager &SourceMgr,
      DiagnosticEngine &Diags,
      toolchain::IntrusiveRefCntPtr<toolchain::vfs::OutputBackend> OutBackend = nullptr);
  ~ASTContext();

  /// Optional table of counters to report, nullptr when not collecting.
  ///
  /// This must be initialized early so that Allocate() doesn't try to access
  /// it before being set to null.
  UnifiedStatsReporter *Stats = nullptr;

  /// The language options used for translation.
  const LangOptions &LangOpts;

  /// The type checker options.
  const TypeCheckerOptions &TypeCheckerOpts;

  /// Options for SIL.
  const SILOptions &SILOpts;

  /// The search path options used by this AST context.
  SearchPathOptions &SearchPathOpts;

  /// The clang importer options used by this AST context.
  ClangImporterOptions &ClangImporterOpts;

  /// The symbol graph generation options used by this AST context.
  symbolgraphgen::SymbolGraphOptions &SymbolGraphOpts;

  /// The CAS options used by this AST context.
  const CASOptions &CASOpts;

  /// Options for Serialization
  const SerializationOptions &SerializationOpts;

  /// The source manager object.
  SourceManager &SourceMgr;

  /// Diags - The diagnostics engine.
  DiagnosticEngine &Diags;

  /// OutputBackend for writing outputs.
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::OutputBackend> OutputBackend;

  enum ModuleImportKind {
    Module = 0,
    Overlay,
    BridgingHeader
  };
  using PreModuleImportCallbackPtr =
      std::function<void(StringRef ModuleName, ModuleImportKind Kind)>;
  /// Set the callback function that is invoked before Codira module importing is
  /// performed.
  void SetPreModuleImportCallback(PreModuleImportCallbackPtr callback);

  /// Call the PreModuleImportCallback. Used by ClangImporter.
  void PreModuleImportHook(StringRef ModuleName, ModuleImportKind Kind) const;

  /// If the shared pointer is not a \c nullptr and the pointee is \c true,
  /// all operations working on this ASTContext should be aborted at the next
  /// possible opportunity.
  /// This is used by SourceKit to cancel requests for which the result is no
  /// longer of interest.
  /// The returned result will be discarded, so the operation that acknowledges
  /// the cancellation might return with any result.
  std::shared_ptr<std::atomic<bool>> CancellationFlag = nullptr;

  ide::TypeCheckCompletionCallback *CompletionCallback = nullptr;

  /// A callback that will be called when the constraint system found a
  /// solution. Called multiple times if the constraint system has ambiguous
  /// solutions.
  ide::TypeCheckCompletionCallback *SolutionCallback = nullptr;

  /// The request-evaluator that is used to process various requests.
  Evaluator evaluator;

  /// The builtin module.
  ModuleDecl * const TheBuiltinModule;

  /// The standard library module.
  mutable ModuleDecl *TheStdlibModule = nullptr;

  /// The name of the standard library module "Codira".
  Identifier StdlibModuleName;

  /// The name of the CodiraShims module "CodiraShims".
  Identifier CodiraShimsModuleName;

  /// Should we globally ignore languagemodule files adjacent to languageinterface
  /// files?
  bool IgnoreAdjacentModules = false;

  /// Accept recovering from more issues at deserialization, even if it can
  /// lead to an inconsistent state. This is enabled for index-while-building
  /// as it runs last and it can afford to read an AST with inconsistencies.
  bool ForceExtendedDeserializationRecovery = false;

  // Define the set of known identifiers.
#define IDENTIFIER_WITH_NAME(Name, IdStr) Identifier Id_##Name;
#include "language/AST/KnownIdentifiers.def"

  /// Cache for names of canonical GenericTypeParamTypes.
  mutable toolchain::DenseMap<unsigned, Identifier>
    CanonicalGenericTypeParamTypeNames;

  /// Cache of remapped types (useful for diagnostics).
  toolchain::StringMap<Type> RemappedTypes;

  /// The # of times we have performed typo correction.
  unsigned NumTypoCorrections = 0;

  /// Cached mapping from types to their associated tangent spaces.
  toolchain::DenseMap<Type, std::optional<TangentSpace>> AutoDiffTangentSpaces;

  /// A cache of derivative function types per configuration.
  toolchain::DenseMap<SILAutoDiffDerivativeFunctionKey, CanSILFunctionType>
      SILAutoDiffDerivativeFunctions;

  /// Cache of `@differentiable` attributes keyed by parameter indices. Used to
  /// diagnose duplicate `@differentiable` attributes for the same key.
  toolchain::DenseMap<std::pair<Decl *, IndexSubset *>, DifferentiableAttr *>
      DifferentiableAttrs;

  /// Cache of `@derivative` attributes keyed by parameter indices and
  /// derivative function kind. Used to diagnose duplicate `@derivative`
  /// attributes for the same key.
  // TODO(TF-1042): remove `DerivativeAttrs` from `ASTContext`. Serialize
  // derivative function configurations per original `AbstractFunctionDecl`.
  toolchain::DenseMap<
      std::tuple<Decl *, IndexSubset *, AutoDiffDerivativeFunctionKind>,
      toolchain::SmallPtrSet<DerivativeAttr *, 1>>
      DerivativeAttrs;

  /// The Codira module currently being compiled.
  ModuleDecl *MainModule = nullptr;

  /// The block list where we can find special actions based on module name;
  BlockListStore blockListConfig;
private:
  /// The current generation number, which reflects the number of
  /// times that external modules have been loaded.
  ///
  /// Various places in the AST, such as the set of extensions associated with
  /// a nominal type, keep track of the generation number they saw and will
  /// automatically update when they are out of date.
  unsigned CurrentGeneration = 0;

  friend class Pattern;

  /// Mapping from patterns that store interface types that will be lazily
  /// resolved to contextual types, to the declaration context in which the
  /// pattern resides.
  toolchain::DenseMap<const Pattern *, DeclContext *>
    DelayedPatternContexts;

  /// Cache of module names that fail the 'canImport' test in this context.
  mutable toolchain::StringSet<> FailedModuleImportNames;

  /// Versions of the modules found during versioned canImport checks. 
  struct ImportedModuleVersionInfo {
    toolchain::VersionTuple Version;
    toolchain::VersionTuple UnderlyingVersion;
  };

  /// Cache of module names that passed the 'canImport' test. This cannot be
  /// mutable since it needs to be queried for dependency discovery. Keep sorted
  /// so caller of `forEachCanImportVersionCheck` can expect deterministic
  /// ordering.
  std::map<std::string, ImportedModuleVersionInfo> CanImportModuleVersions;

  /// Set if a `-module-alias` was passed. Used to store mapping between module aliases and
  /// their corresponding real names, and vice versa for a reverse lookup, which is needed to check
  /// if the module names appearing in source files are aliases or real names.
  /// \see ASTContext::getRealModuleName.
  ///
  /// The boolean in the value indicates whether or not the entry is keyed by an alias vs real name,
  /// i.e. true if the entry is [key: alias_name, value: (real_name, true)].
  mutable toolchain::DenseMap<Identifier, std::pair<Identifier, bool>> ModuleAliasMap;

  /// Retrieve the allocator for the given arena.
  toolchain::BumpPtrAllocator &
  getAllocator(AllocationArena arena = AllocationArena::Permanent) const;

  /// An optional generic callback function invoked prior to importing a module.
  mutable PreModuleImportCallbackPtr PreModuleImportCallback;

public:
  /// Allocate - Allocate memory from the ASTContext bump pointer.
  void *Allocate(unsigned long bytes, unsigned alignment,
                 AllocationArena arena = AllocationArena::Permanent) const {
    if (bytes == 0)
      return nullptr;

    if (LangOpts.UseMalloc)
      return AlignedAlloc(bytes, alignment);

    if (arena == AllocationArena::Permanent && Stats)
      Stats->getFrontendCounters().NumASTBytesAllocated += bytes;
    return getAllocator(arena).Allocate(bytes, alignment);
  }

  template <typename T>
  T *Allocate(AllocationArena arena = AllocationArena::Permanent) const {
    T *res = (T *) Allocate(sizeof(T), alignof(T), arena);
    new (res) T();
    return res;
  }

  template <typename T>
  MutableArrayRef<T> AllocateUninitialized(unsigned NumElts,
              AllocationArena Arena = AllocationArena::Permanent) const {
    T *Data = (T *) Allocate(sizeof(T) * NumElts, alignof(T), Arena);
    return { Data, NumElts };
  }

  template <typename T>
  MutableArrayRef<T> Allocate(unsigned numElts,
              AllocationArena arena = AllocationArena::Permanent) const {
    T *res = (T *) Allocate(sizeof(T) * numElts, alignof(T), arena);
    for (unsigned i = 0; i != numElts; ++i)
      new (res+i) T();
    return {res, numElts};
  }

  /// Allocate a copy of the specified object.
  template <typename T>
  typename std::remove_reference<T>::type *AllocateObjectCopy(T &&t,
              AllocationArena arena = AllocationArena::Permanent) const {
    // This function cannot be named AllocateCopy because it would always win
    // overload resolution over the AllocateCopy(ArrayRef<T>).
    using TNoRef = typename std::remove_reference<T>::type;
    TNoRef *res = (TNoRef *) Allocate(sizeof(TNoRef), alignof(TNoRef), arena);
    new (res) TNoRef(std::forward<T>(t));
    return res;
  }

  template <typename T, typename It>
  T *AllocateCopy(It start, It end,
                  AllocationArena arena = AllocationArena::Permanent) const {
    T *res = (T*)Allocate(sizeof(T)*(end-start), alignof(T), arena);
    for (unsigned i = 0; start != end; ++start, ++i)
      new (res+i) T(*start);
    return res;
  }

  template<typename T, size_t N>
  MutableArrayRef<T> AllocateCopy(T (&array)[N],
      AllocationArena arena = AllocationArena::Permanent) const {
    return MutableArrayRef<T>(AllocateCopy<T>(array, array+N, arena), N);
  }

  template<typename T>
  MutableArrayRef<T> AllocateCopy(ArrayRef<T> array,
      AllocationArena arena = AllocationArena::Permanent) const {
    return MutableArrayRef<T>(AllocateCopy<T>(array.begin(),array.end(), arena),
                              array.size());
  }

  template <typename T>
  MutableArrayRef<T>
  AllocateCopy(const std::vector<T> &vec,
               AllocationArena arena = AllocationArena::Permanent) const {
    return AllocateCopy(ArrayRef<T>(vec), arena);
  }

  template<typename T>
  ArrayRef<T> AllocateCopy(const SmallVectorImpl<T> &vec,
      AllocationArena arena = AllocationArena::Permanent) const {
    return AllocateCopy(ArrayRef<T>(vec), arena);
  }

  template<typename T>
  MutableArrayRef<T>
  AllocateCopy(SmallVectorImpl<T> &vec,
               AllocationArena arena = AllocationArena::Permanent) const {
    return AllocateCopy(MutableArrayRef<T>(vec), arena);
  }

  StringRef AllocateCopy(StringRef Str,
                    AllocationArena arena = AllocationArena::Permanent) const {
    ArrayRef<char> Result =
        AllocateCopy(toolchain::ArrayRef(Str.data(), Str.size()), arena);
    return StringRef(Result.data(), Result.size());
  }

  template<typename T, typename Vector, typename Set, unsigned N>
  MutableArrayRef<T>
  AllocateCopy(toolchain::SetVector<T, Vector, Set, N> setVector,
               AllocationArena arena = AllocationArena::Permanent) const {
    return MutableArrayRef<T>(AllocateCopy<T>(setVector.begin(),
                                              setVector.end(),
                                              arena),
                              setVector.size());
  }

  template <typename Output, typename Range>
  MutableArrayRef<Output> AllocateTransform(
      Range &&input,
      toolchain::function_ref<Output(typename Range::const_reference)> transform,
      AllocationArena arena = AllocationArena::Permanent) {
    auto size = std::distance(std::cbegin(input), std::cend(input));
    auto storage = AllocateUninitialized<Output>(size, arena);
    for (auto i : indices(input))
      new (storage.data() + i) Output(transform(input[i]));
    return storage;
  }

  /// Set a new stats reporter.
  void setStatsReporter(UnifiedStatsReporter *stats);

public:
  /// getIdentifier - Return the uniqued and AST-Context-owned version of the
  /// specified string.
  Identifier getIdentifier(StringRef Str) const;

  /// getDollarIdentifier - Return the uniqued and AST-Context-owned version of
  /// anonymous closure parameter (e.g. '$1') name by the index.
  Identifier getDollarIdentifier(size_t Idx) const;

  /// Convert a given alias map to a map of Identifiers between module aliases and their actual names.
  /// For example, if '-module-alias Foo=X -module-alias Bar=Y' input is passed in, the aliases Foo and Bar are
  /// the names of the imported or referenced modules in source files in the main module, and X and Y
  /// are the real (physical) module names on disk.
  void setModuleAliases(const toolchain::StringMap<std::string> &aliasMap);

  /// Adds a given alias to the map of Identifiers between module aliases and
  /// their actual names.
  void addModuleAlias(StringRef moduleAlias, StringRef realName);

  /// Look up option used in \c getRealModuleName when module aliasing is applied.
  enum class ModuleAliasLookupOption {
    alwaysRealName,
    realNameFromAlias,
    aliasFromRealName
  };

  /// Look up the module alias map by the given \p key and a lookup \p option.
  ///
  /// \param key A module alias or real name to look up the map by.
  /// \param option A look up option \c ModuleAliasLookupOption. Defaults to alwaysRealName.
  ///
  /// \return The real name or alias mapped to the key.
  ///         If no aliasing is used, return \p key regardless of \p option.
  ///         If \p option is alwaysRealName, return the real module name whether the \p key is an alias
  ///         or a real name.
  ///         If \p option is realNameFromAlias, only return a real name if \p key is an alias.
  ///         If \p option is aliasFromRealName, only return an alias if \p key is a real name.
  ///         Else return a real name or an alias mapped to the \p key.
  Identifier getRealModuleName(Identifier key,
                               ModuleAliasLookupOption option = ModuleAliasLookupOption::alwaysRealName) const;

  /// Decide how to interpret two precedence groups.
  Associativity associateInfixOperators(PrecedenceGroupDecl *left,
                                        PrecedenceGroupDecl *right) const;

  /// Retrieve the declaration of Codira.Error.
  ProtocolDecl *getErrorDecl() const;
  CanType getErrorExistentialType() const;

#define KNOWN_STDLIB_TYPE_DECL(NAME, DECL_CLASS, NUM_GENERIC_PARAMS) \
  /** Retrieve the declaration of Codira.NAME. */ \
  DECL_CLASS *get##NAME##Decl() const; \
\
  /** Retrieve the type of Codira.NAME. */ \
  Type get##NAME##Type() const;
#include "language/AST/KnownStdlibTypes.def"

  /// Retrieve the declaration of Codira.Optional<T>.Some.
  EnumElementDecl *getOptionalSomeDecl() const;
  
  /// Retrieve the declaration of Codira.Optional<T>.None.
  EnumElementDecl *getOptionalNoneDecl() const;

  /// Retrieve the declaration of Codira.Void.
  TypeAliasDecl *getVoidDecl() const;

  /// Retrieve the type of Codira.Void.
  Type getVoidType() const;

  /// Retrieve the declaration of the "pointee" property of a pointer type.
  VarDecl *getPointerPointeePropertyDecl(PointerTypeKind ptrKind) const;

  /// Retrieve the type Codira.Any as an existential type.
  CanType getAnyExistentialType() const;

  /// Retrieve the existential type 'any ~Copyable & ~Escapable'.
  ///
  /// This is the most permissive existential type.
  CanType getUnconstrainedAnyExistentialType() const;

  /// Retrieve the type Codira.AnyObject as a constraint.
  CanType getAnyObjectConstraint() const;

  /// Retrieve the type Codira.AnyObject as an existential type.
  CanType getAnyObjectType() const;

#define KNOWN_SDK_TYPE_DECL(MODULE, NAME, DECL_CLASS, NUM_GENERIC_PARAMS) \
  /** Retrieve the declaration of MODULE.NAME. */ \
  DECL_CLASS *get##NAME##Decl() const; \
\
  /** Retrieve the type of MODULE.NAME. */ \
  Type get##NAME##Type() const;
#include "language/AST/KnownSDKTypes.def"

  // Declare accessors for the known declarations.
#define FUNC_DECL(Name, Id) \
  FuncDecl *get##Name() const;
#include "language/AST/KnownDecls.def"

  // Declare accessors for the known declarations.
#define KNOWN_SDK_FUNC_DECL(Module, Name, Id) \
  FuncDecl *get##Name() const;
#include "language/AST/KnownSDKDecls.def"

  /// Get the '+' function on two RangeReplaceableCollection.
  FuncDecl *getPlusFunctionOnRangeReplaceableCollection() const;

  /// Get the '+' function on two String.
  FuncDecl *getPlusFunctionOnString() const;

  /// Get Sequence.makeIterator().
  FuncDecl *getSequenceMakeIterator() const;

  /// Get AsyncSequence.makeAsyncIterator().
  FuncDecl *getAsyncSequenceMakeAsyncIterator() const;

  /// Get IteratorProtocol.next().
  FuncDecl *getIteratorNext() const;

  /// Get AsyncIteratorProtocol.next().
  FuncDecl *getAsyncIteratorNext() const;

  /// Get AsyncIteratorProtocol.next(actor).
  FuncDecl *getAsyncIteratorNextIsolated() const;

  /// Check whether the standard library provides all the correct
  /// intrinsic support for Optional<T>.
  ///
  /// If this is true, the four methods above all promise to return
  /// non-null.
  bool hasOptionalIntrinsics() const;

  /// Check whether the standard library provides all the correct
  /// intrinsic support for UnsafeMutablePointer<T> function arguments.
  ///
  /// If this is true, the methods getConvert*ToPointerArgument
  /// all promise to return non-null.
  bool hasPointerArgumentIntrinsics() const;

  /// Check whether the standard library provides all the correct
  /// intrinsic support for array literals.
  ///
  /// If this is true, the method getAllocateUninitializedArray
  /// promises to return non-null.
  bool hasArrayLiteralIntrinsics() const;

  /// Retrieve the declaration of Codira.Bool.init(_builtinBooleanLiteral:)
  ConcreteDeclRef getBoolBuiltinInitDecl() const;

  /// Retrieve the witness for init(_builtinIntegerLiteral:).
  ConcreteDeclRef getIntBuiltinInitDecl(NominalTypeDecl *intDecl) const;

  /// Retrieve the witness for init(_builtinFloatLiteral:).
  ConcreteDeclRef getFloatBuiltinInitDecl(NominalTypeDecl *floatDecl) const;

  /// Retrieve the witness for (_builtinStringLiteral:utf8CodeUnitCount:isASCII:).
  ConcreteDeclRef getStringBuiltinInitDecl(NominalTypeDecl *stringDecl) const;

  ConcreteDeclRef getBuiltinInitDecl(NominalTypeDecl *decl,
                                     KnownProtocolKind builtinProtocol,
                toolchain::function_ref<DeclName (ASTContext &ctx)> initName) const;

  /// Retrieve _StringProcessing.Regex.init(_regexString: String, version: Int).
  ConcreteDeclRef getRegexInitDecl(Type regexType) const;

  /// Retrieve the declaration of Codira.<(Int, Int) -> Bool.
  FuncDecl *getLessThanIntDecl() const;

  /// Retrieve the declaration of Codira.==(Int, Int) -> Bool.
  FuncDecl *getEqualIntDecl() const;

  /// Retrieve the declaration of Codira._hashValue<H>(for: H) -> Int.
  FuncDecl *getHashValueForDecl() const;

  /// Retrieve the declaration of Array.append(element:)
  FuncDecl *getArrayAppendElementDecl() const;

  /// Retrieve the declaration of
  /// Array.reserveCapacityForAppend(newElementsCount: Int)
  FuncDecl *getArrayReserveCapacityDecl() const;

  /// Retrieve the declaration of String.init(_builtinStringLiteral ...)
  ConstructorDecl *getMakeUTF8StringDecl() const;

  // Retrieve the declaration of Codira._stdlib_isOSVersionAtLeast.
  FuncDecl *getIsOSVersionAtLeastDecl() const;

  // Retrieve the declaration of Codira._stdlib_isVariantOSVersionAtLeast.
  FuncDecl *getIsVariantOSVersionAtLeastDecl() const;

  // Retrieve the declaration of
  // Codira._stdlib_isOSVersionAtLeastOrVariantVersionAtLeast.
  FuncDecl *getIsOSVersionAtLeastOrVariantVersionAtLeast() const;

  /// Look for the declaration with the given name within the
  /// passed in module.
  void lookupInModule(ModuleDecl *M, StringRef name,
                      SmallVectorImpl<ValueDecl *> &results) const;

  /// Look for the declaration with the given name within the
  /// Codira module.
  void lookupInCodiraModule(StringRef name,
                           SmallVectorImpl<ValueDecl *> &results) const;

  /// Retrieve a specific, known protocol.
  ProtocolDecl *getProtocol(KnownProtocolKind kind) const;

  /// Synthesizes the given invertible protocol decl into the stdlib (preferred)
  /// or, if the stdlib is not available, the Builtin module.
  ///
  /// Does *not* perform any name lookup to check whether, the module already
  /// contains a decl with the same name, only does synthesis.
  ProtocolDecl *synthesizeInvertibleProtocolDecl(InvertibleProtocolKind ip) const;
  
  /// Determine whether the given nominal type is one of the standard
  /// library or Cocoa framework types that is known to be bridged by another
  /// module's overlay, for layering or implementation detail reasons.
  bool isTypeBridgedInExternalModule(NominalTypeDecl *nominal) const;

  /// True if the given type is an Objective-C class that serves as the bridged
  /// object type for many Codira value types, meaning that the conversion from
  /// an object to a value is a conditional cast.
  bool isObjCClassWithMultipleCodiraBridgedTypes(Type t);

  /// Get the Objective-C type that a Codira type bridges to, if any.
  /// 
  /// \param dc The context in which bridging is occurring.
  /// \param type The Codira for which we are querying bridging behavior.
  /// \param bridgedValueType The specific value type that is bridged,
  /// which will usually by the same as \c type.
  Type getBridgedToObjC(const DeclContext *dc, Type type,
                        Type *bridgedValueType = nullptr) const;

private:
  ClangTypeConverter &getClangTypeConverter();

public:
  /// Get the Clang type corresponding to a Codira function type.
  ///
  /// \param params The function parameters.
  /// \param resultTy The Codira result type.
  /// \param trueRep The actual calling convention, which must be C-compatible.
  const clang::Type *
  getClangFunctionType(ArrayRef<AnyFunctionType::Param> params, Type resultTy,
                       FunctionTypeRepresentation trueRep);

  /// Get the canonical Clang type corresponding to a SIL function type.
  ///
  /// SIL analog of \c ASTContext::getClangFunctionType .
  const clang::Type *
  getCanonicalClangFunctionType(ArrayRef<SILParameterInfo> params,
                                std::optional<SILResultInfo> result,
                                SILFunctionType::Representation trueRep);

  /// Instantiates "Impl.Converter" if needed, then translate Codira generic
  /// substitutions to equivalent C++ types using \p templateParams and \p
  /// genericArgs. The converted Clang types are placed into \p templateArgs.
  ///
  /// \p templateArgs must be empty. \p templateParams and \p genericArgs must
  /// be equal in size.
  ///
  /// \returns nullptr if successful. If an error occurs, returns a list of
  /// types that couldn't be converted.
  std::unique_ptr<TemplateInstantiationError> getClangTemplateArguments(
      const clang::TemplateParameterList *templateParams,
      ArrayRef<Type> genericArgs,
      SmallVectorImpl<clang::TemplateArgument> &templateArgs);

  /// Get the Codira declaration that a Clang declaration was exported from,
  /// if applicable.
  const Decl *getCodiraDeclForExportedClangDecl(const clang::Decl *decl);

  /// General conversion method from Codira types -> Clang types.
  ///
  /// HACK: This method is only intended to be called from a specific place in
  /// IRGen. For converting function types, strongly prefer using one of the
  /// other methods instead, instead of manually iterating over parameters
  /// and results.
  const clang::Type *getClangTypeForIRGen(Type ty);

  /// Determine whether the given Codira type is representable in a
  /// given foreign language.
  ForeignRepresentationInfo
  getForeignRepresentationInfo(NominalTypeDecl *nominal,
                               ForeignLanguage language,
                               const DeclContext *dc);

  /// Add a cleanup function to be called when the ASTContext is deallocated.
  void addCleanup(std::function<void(void)> cleanup);

  /// Add a cleanup to run the given object's destructor when the ASTContext is
  /// deallocated.
  template<typename T>
  void addDestructorCleanup(T &object) {
    addCleanup([&object]{ object.~T(); });
  }

  /// Get the availability of features introduced in the specified version
  /// of the Codira compiler for the target platform.
  AvailabilityRange getCodiraAvailability(unsigned major, unsigned minor) const;

  // For each feature defined in FeatureAvailability, define two functions;
  // the latter, with the suffix RuntimeAvailabilty, is for use with
  // AvailabilityRange::forRuntimeTarget(), and only looks at the Codira
  // runtime version.
#define FEATURE(N, V)                                                          \
  inline AvailabilityRange get##N##Availability() const {                      \
    return getCodiraAvailability V;                                             \
  }                                                                            \
  inline AvailabilityRange get##N##RuntimeAvailability() const {               \
    return AvailabilityRange(VersionRange::allGTE(toolchain::VersionTuple V));      \
  }

#include "language/AST/FeatureAvailability.def"

  /// Get the runtime availability of features that have been introduced in the
  /// Codira compiler for future versions of the target platform.
  AvailabilityRange getCodiraFutureAvailability() const;

  /// Returns `true` if versioned availability annotations are supported for the
  /// target triple.
  bool supportsVersionedAvailability() const;

  //===--------------------------------------------------------------------===//
  // Compatibility availability functions
  //===--------------------------------------------------------------------===//

  // Note: Don't add more of these version-specific availability functions;
  // just use getCodiraAvailability() instead.

  /// Get the runtime availability of features introduced in the Codira 5.0
  /// compiler for the target platform.
  inline AvailabilityRange getCodira50Availability() const {
    return getCodiraAvailability(5, 0);
  }

  /// Get the runtime availability of features introduced in the Codira 5.1
  /// compiler for the target platform.
  inline AvailabilityRange getCodira51Availability() const {
    return getCodiraAvailability(5, 1);
  }

  /// Get the runtime availability of features introduced in the Codira 5.2
  /// compiler for the target platform.
  inline AvailabilityRange getCodira52Availability() const {
    return getCodiraAvailability(5, 2);
  }

  /// Get the runtime availability of features introduced in the Codira 5.3
  /// compiler for the target platform.
  inline AvailabilityRange getCodira53Availability() const {
    return getCodiraAvailability(5, 3);
  }

  /// Get the runtime availability of features introduced in the Codira 5.4
  /// compiler for the target platform.
  inline AvailabilityRange getCodira54Availability() const {
    return getCodiraAvailability(5, 4);
  }

  /// Get the runtime availability of features introduced in the Codira 5.5
  /// compiler for the target platform.
  inline AvailabilityRange getCodira55Availability() const {
    return getCodiraAvailability(5, 5);
  }

  /// Get the runtime availability of features introduced in the Codira 5.6
  /// compiler for the target platform.
  inline AvailabilityRange getCodira56Availability() const {
    return getCodiraAvailability(5, 6);
  }

  /// Get the runtime availability of features introduced in the Codira 5.7
  /// compiler for the target platform.
  inline AvailabilityRange getCodira57Availability() const {
    return getCodiraAvailability(5, 7);
  }

  /// Get the runtime availability of features introduced in the Codira 5.8
  /// compiler for the target platform.
  inline AvailabilityRange getCodira58Availability() const {
    return getCodiraAvailability(5, 8);
  }

  /// Get the runtime availability of features introduced in the Codira 5.9
  /// compiler for the target platform.
  inline AvailabilityRange getCodira59Availability() const {
    return getCodiraAvailability(5, 9);
  }

  /// Get the runtime availability of features introduced in the Codira 5.10
  /// compiler for the target platform.
  inline AvailabilityRange getCodira510Availability() const {
    return getCodiraAvailability(5, 10);
  }

  /// Get the runtime availability of features introduced in the Codira 6.0
  /// compiler for the target platform.
  inline AvailabilityRange getCodira60Availability() const {
    return getCodiraAvailability(6, 0);
  }

  /// Get the runtime availability for a particular version of Codira (5.0+).
  inline AvailabilityRange
  getCodira5PlusAvailability(toolchain::VersionTuple languageVersion) const {
    return getCodiraAvailability(languageVersion.getMajor(),
                                *languageVersion.getMinor());
  }

  /// Get the runtime availability of getters and setters of multi payload enum
  /// tag single payloads.
  inline AvailabilityRange getMultiPayloadEnumTagSinglePayload() const {
    // This didn't fit the pattern, so needed renaming
    return getMultiPayloadEnumTagSinglePayloadAvailability();
  }

  /// Availability macros parsed from the command line arguments.
  const AvailabilityMacroMap &getAvailabilityMacroMap() const;

  /// Test support utility for loading a platform remap file
  /// in case an SDK is not specified to the compilation.
  const clang::DarwinSDKInfo::RelatedTargetVersionMapping *
  getAuxiliaryDarwinPlatformRemapInfo(
      clang::DarwinSDKInfo::OSEnvPair Kind) const;

  //===--------------------------------------------------------------------===//
  // Diagnostics Helper functions
  //===--------------------------------------------------------------------===//

  bool hadError() const;
  
  //===--------------------------------------------------------------------===//
  // Type manipulation routines.
  //===--------------------------------------------------------------------===//

  // Builtin type and simple types that are used frequently.
  const CanType TheErrorType;             /// This is the ErrorType singleton.
  const CanType TheUnresolvedType;        /// This is the UnresolvedType singleton.
  const CanType TheEmptyTupleType;        /// This is '()', aka Void
  const CanType TheEmptyPackType;
  const CanType TheAnyType;               /// This is 'Any', the empty protocol composition
  const CanType TheUnconstrainedAnyType;  /// This is 'any ~Copyable & ~Escapable',
                                          /// the empty protocol composition
                                          /// without any implicit constraints.
  const CanGenericTypeParamType TheSelfType; /// The protocol 'Self' type;
                                             /// a generic parameter with
                                             /// depth 0 index 0
#define SINGLETON_TYPE(SHORT_ID, ID) \
  const CanType The##SHORT_ID##Type;
#include "language/AST/TypeNodes.def"

  const CanType TheIEEE32Type;            /// 32-bit IEEE floating point
  const CanType TheIEEE64Type;            /// 64-bit IEEE floating point
  
  // Target specific types.
  const CanType TheIEEE16Type;            /// 16-bit IEEE floating point
  const CanType TheIEEE80Type;            /// 80-bit IEEE floating point
  const CanType TheIEEE128Type;           /// 128-bit IEEE floating point
  const CanType ThePPC128Type;            /// 128-bit PowerPC 2xDouble

  /// Adds a search path to SearchPathOpts, unless it is already present.
  ///
  /// Does any proper bookkeeping to keep all module loaders up to date as well.
  void addSearchPath(StringRef searchPath, bool isFramework, bool isSystem);

  /// Adds a module loader to this AST context.
  ///
  /// \param loader The new module loader, which will be added after any
  ///               existing module loaders.
  /// \param isClang \c true if this module loader is responsible for loading
  ///                Clang modules, which are special-cased in some parts of the
  ///                compiler.
  /// \param isDWARF \c true if this module loader can load Clang modules
  ///                from DWARF.
  /// \param IsInterface \c true if this module loader can load Codira textual
  ///                interface.
  void addModuleLoader(std::unique_ptr<ModuleLoader> loader,
                       bool isClang = false, bool isDWARF = false,
                       bool IsInterface = false);

  /// Add a module interface checker to use for this AST context.
  void addModuleInterfaceChecker(std::unique_ptr<ModuleInterfaceChecker> checker);

  /// Retrieve the module interface checker associated with this AST context.
  ModuleInterfaceChecker *getModuleInterfaceChecker() const;

  /// Load extensions to the given nominal type from the external
  /// module loaders.
  ///
  /// \param nominal The nominal type whose extensions should be loaded.
  ///
  /// \param previousGeneration The previous generation number. The AST already
  /// contains extensions loaded from any generation up to and including this
  /// one.
  void loadExtensions(NominalTypeDecl *nominal, unsigned previousGeneration);

  /// Load the methods within the given type that produce
  /// Objective-C class or instance methods with the given selector.
  ///
  /// \param tyDecl The type in which we are searching for @objc methods.
  /// The search only considers this type and its extensions; not any
  /// superclasses.
  ///
  /// \param selector The selector to search for.
  ///
  /// \param isInstanceMethod Whether we are looking for an instance method
  /// (vs. a class method).
  ///
  /// \param previousGeneration The previous generation with which this
  /// callback was invoked. The list of methods will already contain all of
  /// the results from generations up and including \c previousGeneration.
  ///
  /// \param methods The list of @objc methods in this class that have this
  /// selector and are instance/class methods as requested. This list will be
  /// extended with any methods found in subsequent generations.
  ///
  /// \param languageOnly If true, only loads methods from imported Codira modules,
  /// skipping the Clang importer.
  ///
  /// \note Passing a protocol is supported, but currently a no-op, because
  /// Objective-C protocols cannot be extended in ways that make the ObjC method
  /// lookup table relevant.
  void loadObjCMethods(NominalTypeDecl *tyDecl, ObjCSelector selector,
                       bool isInstanceMethod, unsigned previousGeneration,
                       toolchain::TinyPtrVector<AbstractFunctionDecl *> &methods,
                       bool languageOnly = false);

  /// Load derivative function configurations for the given
  /// AbstractFunctionDecl.
  ///
  /// \param originalAFD The declaration whose derivative function
  /// configurations should be loaded.
  ///
  /// \param previousGeneration The previous generation number. The AST already
  /// contains derivative function configurations loaded from any generation up
  /// to and including this one.
  void loadDerivativeFunctionConfigurations(
      AbstractFunctionDecl *originalAFD, unsigned previousGeneration,
      toolchain::SetVector<AutoDiffConfig> &results);

  /// Given `Optional<T>.TangentVector` type, retrieve the
  /// `Optional<T>.TangentVector.init` declaration.
  ConstructorDecl *getOptionalTanInitDecl(CanType optionalTanType);

  /// Optional<T>.TangentVector is a struct with a single
  /// Optional<T.TangentVector> `value` property. This is an implementation
  /// detail of OptionalDifferentiation.code. Retrieve `VarDecl` corresponding
  /// to this property.
  VarDecl *getOptionalTanValueDecl(CanType optionalTanType);

  /// Retrieve the next macro expansion discriminator within the given
  /// name and context.
  unsigned getNextMacroDiscriminator(MacroDiscriminatorContext context,
                                     DeclBaseName baseName);

  /// Get the next discriminator within the given declaration context.
  unsigned getNextDiscriminator(const DeclContext *dc);

  /// Set the maximum assigned discriminator within the given declaration context.
  void setMaxAssignedDiscriminator(
      const DeclContext *dc, unsigned discriminator);

  /// Retrieve the Clang module loader for this ASTContext.
  ///
  /// If there is no Clang module loader, returns a null pointer.
  /// The loader is owned by the AST context.
  ClangModuleLoader *getClangModuleLoader() const;

  /// Retrieve the DWARF module loader for this ASTContext.
  ///
  /// If there is no Clang module loader, returns a null pointer.
  /// The loader is owned by the AST context.
  ClangModuleLoader *getDWARFModuleLoader() const;

  /// Check whether the module with a given name can be imported without
  /// importing it.
  ///
  /// Note that even if this check succeeds, errors may still occur if the
  /// module is loaded in full.
  bool canImportModuleImpl(ImportPath::Module ModulePath, SourceLoc loc,
                           toolchain::VersionTuple version, bool underlyingVersion,
                           bool isSourceCanImport,
                           toolchain::VersionTuple &foundVersion,
                           toolchain::VersionTuple &foundUnderlyingClangVersion) const;

  /// Add successful canImport modules.
  void addSucceededCanImportModule(StringRef moduleName,
                                   const toolchain::VersionTuple &versionInfo,
                                   const toolchain::VersionTuple &underlyingVersionInfo);

public:
  namelookup::ImportCache &getImportCache() const;

  /// Returns an iterator over the modules that are known by this context
  /// to be loaded.
  ///
  /// Iteration order is guaranteed to match the order in which
  /// \c addLoadedModule was called to register the loaded module
  /// with this context.
  iterator_range<toolchain::MapVector<Identifier, ModuleDecl *>::const_iterator>
  getLoadedModules() const;

  /// Returns the number of loaded modules known by this context to be loaded.
  unsigned getNumLoadedModules() const {
    auto eltRange = getLoadedModules();
    return std::distance(eltRange.begin(), eltRange.end());
  }

  /// Asks every module loader to verify the ASTs it has loaded.
  ///
  /// Does nothing in non-asserts (NDEBUG) builds.
  void verifyAllLoadedModules() const;

  /// Check whether the module with a given name can be imported without
  /// importing it.
  ///
  /// Note that even if this check succeeds, errors may still occur if the
  /// module is loaded in full.
  bool canImportModule(ImportPath::Module ModulePath, SourceLoc loc,
                       toolchain::VersionTuple version = toolchain::VersionTuple(),
                       bool underlyingVersion = false);

  /// Check whether the module with a given name can be imported without
  /// importing it. This is a const method that won't remember the outcome so
  /// repeated check of the same module will induce full cost and won't count
  /// as the dependency for current module.
  bool testImportModule(ImportPath::Module ModulePath,
                        toolchain::VersionTuple version = toolchain::VersionTuple(),
                        bool underlyingVersion = false) const;

  /// Callback on each successful imported.
  void forEachCanImportVersionCheck(
      std::function<void(StringRef, const toolchain::VersionTuple &,
                         const toolchain::VersionTuple &)>) const;

  /// \returns a module with a given name that was already loaded.  If the
  /// module was not loaded, returns nullptr.
  ModuleDecl *getLoadedModule(
      ImportPath::Module ModulePath) const;

  ModuleDecl *getLoadedModule(Identifier ModuleName) const;

  /// Attempts to load a module into this ASTContext.
  ///
  /// If a module by this name has already been loaded, the existing module will
  /// be returned.
  ///
  /// \param ModulePath The module's \c ImportPath which describes
  /// the name of the module being loaded, possibly including submodules.
  /// \param AllowMemoryCached Should we allow reuse of an already loaded
  /// module or force reloading from disk, defaults to true.
  ///
  /// \returns The requested module, or NULL if the module cannot be found.
  ModuleDecl *
  getModule(ImportPath::Module ModulePath, bool AllowMemoryCached = true);

  /// Attempts to load the matching overlay module for the given clang
  /// module into this ASTContext.
  ///
  /// \returns The Codira overlay module corresponding to the given Clang module,
  /// or NULL if the overlay module cannot be found.
  ModuleDecl *getOverlayModule(const FileUnit *ClangModule);

  ModuleDecl *getModuleByName(StringRef ModuleName);

  ModuleDecl *getModuleByIdentifier(Identifier ModuleID);

  /// Looks up all modules whose real name or ABI name match ModuleName.
  ///
  /// Modules that are being looked up by ABI name are only found if they are a
  /// dependency of a module that has that same name as its real name, as there
  /// is no efficient way to lazily load a module by ABI name.
  toolchain::ArrayRef<ModuleDecl *> getModulesByRealOrABIName(StringRef ModuleName);

  /// Notifies the AST context that a loaded module's ABI name will change.
  void moduleABINameWillChange(ModuleDecl *module, Identifier newName);

  /// Returns the standard library module, or null if the library isn't present.
  ///
  /// If \p loadIfAbsent is true, the ASTContext will attempt to load the module
  /// if it hasn't been set yet.
  ModuleDecl *getStdlibModule(bool loadIfAbsent = false);

  ModuleDecl *getStdlibModule() const {
    return const_cast<ASTContext *>(this)->getStdlibModule(false);
  }

  /// Insert an externally-sourced module into the set of known loaded modules
  /// in this context.
  void addLoadedModule(ModuleDecl *M);

  /// Remove an externally-sourced module from the set of known loaded modules
  /// in this context. Modules are added to the cache before being loaded to
  /// avoid loading a module twice when there is a cyclic dependency between an
  /// overlay and its Clang module. If a module import fails, the non-imported
  /// module should be removed from the cache again.
  void removeLoadedModule(Identifier RealName);

  /// Change the behavior of all loaders to ignore languagemodules next to
  /// languageinterfaces.
  void setIgnoreAdjacentModules(bool value);

  /// Retrieve the current generation number, which reflects the
  /// number of times a module import has caused mass invalidation of
  /// lookup tables.
  ///
  /// Various places in the AST keep track of the generation numbers at which
  /// their own information is valid, such as the list of extensions associated
  /// with a nominal type.
  unsigned getCurrentGeneration() const { return CurrentGeneration; }

  /// Increase the generation number, implying that various lookup
  /// tables have been significantly altered by the introduction of a new
  /// module import.
  ///
  /// \returns the previous generation number.
  unsigned bumpGeneration() { return CurrentGeneration++; }

  /// Produce a "normal" conformance for a nominal type.
  ///
  /// For ordinary conformance lookups, use ModuleDecl::lookupConformance()
  /// instead.
  NormalProtocolConformance *
  getNormalConformance(Type conformingType,
                       ProtocolDecl *protocol,
                       SourceLoc loc,
                       TypeRepr *inheritedTypeRepr,
                       DeclContext *dc,
                       ProtocolConformanceState state,
                       ProtocolConformanceOptions options);

  /// Produce a self-conformance for the given protocol.
  SelfProtocolConformance *
  getSelfConformance(ProtocolDecl *protocol);

  /// Produce the builtin conformance for some structural type to some protocol.
  BuiltinProtocolConformance *
  getBuiltinConformance(Type type, ProtocolDecl *protocol,
                        BuiltinConformanceKind kind);

  /// A callback used to produce a diagnostic for an ill-formed protocol
  /// conformance that was type-checked before we're actually walking the
  /// conformance itself, along with a bit indicating whether this diagnostic
  /// produces an error.
  struct DelayedConformanceDiag {
    bool IsError;
    std::function<void(NormalProtocolConformance *)> Callback;
  };

  /// Describes a missing witness during conformance checking.
  class MissingWitness {
  public:
    /// The requirement that is missing a witness.
    ValueDecl *requirement;

    /// The set of potential matching witnesses.
    std::vector<RequirementMatch> matches;

    MissingWitness(ValueDecl *requirement,
                   ArrayRef<RequirementMatch> matches);
  };

  /// Check whether current context has any errors associated with
  /// ill-formed protocol conformances which haven't been produced yet.
  ///
  /// @param conformance if non-null, will check only for errors specific to the
  /// provided conformance. Otherwise, checks for _any_ errors.
  ///
  /// @returns true iff there are any delayed diagnostic errors
  bool hasDelayedConformanceErrors(
                  NormalProtocolConformance const* conformance = nullptr) const;

  /// Add a delayed diagnostic produced while type-checking a
  /// particular protocol conformance.
  void addDelayedConformanceDiag(
      NormalProtocolConformance *conformance, bool isError,
      std::function<void(NormalProtocolConformance *)> callback);

  /// Retrieve the delayed-conformance diagnostic callbacks for the
  /// given normal protocol conformance.
  std::vector<DelayedConformanceDiag>
  takeDelayedConformanceDiags(NormalProtocolConformance const* conformance);

  /// Add delayed missing witnesses for the given normal protocol conformance.
  void addDelayedMissingWitness(
      NormalProtocolConformance *conformance,
      MissingWitness missingWitness);

  /// Retrieve the delayed missing witnesses for the given normal protocol
  /// conformance.
  std::vector<MissingWitness>
  takeDelayedMissingWitnesses(NormalProtocolConformance *conformance);

  /// Produce a specialized conformance, which takes a generic
  /// conformance and substitutions written in terms of the generic
  /// conformance's signature.
  ///
  /// \param type The type for which we are retrieving the conformance.
  ///
  /// \param generic The generic conformance.
  ///
  /// \param substitutions The set of substitutions required to produce the
  /// specialized conformance from the generic conformance.
  ProtocolConformance *
  getSpecializedConformance(Type type,
                            NormalProtocolConformance *generic,
                            SubstitutionMap substitutions);

  /// Produce an inherited conformance, which forwards all operations to a
  /// base conformance, except for getType(), which must return some subclass
  /// of the base conformance's conforming type. This models the case where a
  /// subclass conforms to a protocol because its superclass already conforms.
  ///
  /// \param type The type for which we are retrieving the conformance.
  ///
  /// \param inherited A normal or specialized conformance.
  ProtocolConformance *
  getInheritedConformance(Type type, ProtocolConformance *inherited);

  /// Get the lazy data for the given declaration.
  LazyContextData *getLazyContextData(const Decl *decl) const;

  /// Get or otherwise allocate the lazy data for the given declaration.
  ///
  /// \param lazyLoader If non-null, the lazy loader to use when creating the
  /// lazy data. The pointer must either be null or be consistent
  /// across all calls for the same \p decl.
  LazyContextData *getOrCreateLazyContextData(const Decl *decl,
                                              LazyMemberLoader *lazyLoader);

  /// Get the lazy iterable context for the given iterable declaration context.
  ///
  /// \param lazyLoader If non-null, the lazy loader to use when creating the
  /// iterable context data. The pointer must either be null or be consistent
  /// across all calls for the same \p idc.
  LazyIterableDeclContextData *getOrCreateLazyIterableContextData(
                                              const IterableDeclContext *idc,
                                              LazyMemberLoader *lazyLoader);

  /// Access the side cache for property wrapper backing property types,
  /// used because TypeChecker::typeCheckBinding() needs somewhere to stash
  /// the backing property type.
  Type getSideCachedPropertyWrapperBackingPropertyType(VarDecl *var) const;
  void setSideCachedPropertyWrapperBackingPropertyType(VarDecl *var,
                                                        Type type);
  
  /// Returns memory usage of this ASTContext.
  size_t getTotalMemory() const;
  
  /// Returns memory used exclusively by constraint solver.
  size_t getSolverMemory() const;

  /// Retrieve the Codira identifier for the given Foundation entity, where
  /// "NS" prefix stripping will apply under omit-needless-words.
  Identifier getCodiraId(KnownFoundationEntity kind) {
    return getIdentifier(language::getCodiraName(kind));
  }

  /// Populate \p names with visible top level module names.
  /// This guarantees that resulted \p names doesn't have duplicated names.
  void getVisibleTopLevelModuleNames(SmallVectorImpl<Identifier> &names) const;

  /// Whether to perform typo correction given the pre-configured correction limit.
  /// Increments \c NumTypoCorrections then checks this against the limit in
  /// the language options.
  bool shouldPerformTypoCorrection();

private:
  friend class IntrinsicInfo;
  /// Retrieve an LLVMContext that is used for scratch space for intrinsic lookup.
  toolchain::LLVMContext &getIntrinsicScratchContext() const;

public:
  rewriting::RewriteContext &getRewriteContext();

  /// This is a hack to break cycles. Don't introduce new callers of this
  /// method.
  bool isRecursivelyConstructingRequirementMachine(
      CanGenericSignature sig);

  /// This is a hack to break cycles. Don't introduce new callers of this
  /// method.
  bool isRecursivelyConstructingRequirementMachine(
      const ProtocolDecl *proto);

  /// Retrieve a generic parameter list with a single parameter named `Self`.
  /// This is for parsing @opened archetypes in textual SIL.
  GenericParamList *getSelfGenericParamList(DeclContext *dc) const;

  /// Retrieve a generic signature with a single unconstrained type parameter,
  /// like `<T>`.
  CanGenericSignature getSingleGenericParameterSignature() const;

  /// Retrieve a generic signature with a single type parameter conforming
  /// to the given protocol or composition type, like <T: P>.
  ///
  /// The opened archetype may have a different set of conformances from the
  /// corresponding existential. The opened archetype conformances are dictated
  /// by the ABI for generic arguments, while the existential value conformances
  /// are dictated by their layout (see \c Type::getExistentialLayout()). In
  /// particular, the opened archetype signature does not have requirements for
  /// conformances inherited from superclass constraints while existential
  /// values do.
  OpenedExistentialSignature getOpenedExistentialSignature(Type type);

  /// Get a generic signature where the generic parameter τ_d_i represents
  /// the element of the pack generic parameter τ_d_i… in \p baseGenericSig.
  ///
  /// This drops the parameter pack bit from each generic parameter,
  /// and converts same-element requirements to same-type requirements.
  CanGenericSignature getOpenedElementSignature(CanGenericSignature baseGenericSig,
                                                CanGenericTypeParamType shapeClass);

  GenericSignature getOverrideGenericSignature(const ValueDecl *base,
                                               const ValueDecl *derived);

  GenericSignature
  getOverrideGenericSignature(const NominalTypeDecl *baseNominal,
                              const NominalTypeDecl *derivedNominal,
                              GenericSignature baseGenericSig,
                              const GenericParamList *derivedParams);

  enum class OverrideGenericSignatureReqCheck {
    /// Base method's generic requirements are satisfied by derived method
    BaseReqSatisfiedByDerived,

    /// Derived method's generic requirements are satisfied by base method
    DerivedReqSatisfiedByBase
  };

  bool overrideGenericSignatureReqsSatisfied(
      const ValueDecl *base, const ValueDecl *derived,
      const OverrideGenericSignatureReqCheck direction);

  /// Whether our effective Codira version is at least 'major'.
  ///
  /// This is usually the check you want; for example, when introducing
  /// a new language feature which is only visible in Codira 5, you would
  /// check for isCodiraVersionAtLeast(5).
  bool isCodiraVersionAtLeast(unsigned major, unsigned minor = 0) const {
    return LangOpts.isCodiraVersionAtLeast(major, minor);
  }

  /// Check whether it's important to respect access control restrictions
  /// in current context.
  bool isAccessControlDisabled() const {
    return !LangOpts.EnableAccessControl;
  }

  /// Each kind and SourceFile has its own cache for a Type.
  Type &getDefaultTypeRequestCache(SourceFile *, KnownProtocolKind);

  using SILTransformCtors = ArrayRef<SILTransform *(*)(void)>;

  /// Register IRGen specific SIL passes such that the SILOptimizer can access
  /// and execute them without directly depending on IRGen.
  void registerIRGenSILTransforms(SILTransformCtors fns);

  /// Retrieve the IRGen specific SIL passes.
  SILTransformCtors getIRGenSILTransforms() const;
  
  /// Check whether a given string would be considered "pure ASCII" by the
  /// standard library's String implementation.
  bool isASCIIString(StringRef s) const;

  /// Retrieve the name of to be used for the entry point, either main or an
  /// alternative specified via the -entry-point-function-name frontend flag.
  std::string getEntryPointFunctionName() const;

  /// The special Builtin.TheTupleType, which parents tuple extensions and
  /// conformances.
  BuiltinTupleDecl *getBuiltinTupleDecl();

  /// The declared interface type of Builtin.TheTupleType.
  BuiltinTupleType *getBuiltinTupleType();

  Type getNamedCodiraType(ModuleDecl *module, StringRef name);

  /// Set the plugin loader.
  void setPluginLoader(std::unique_ptr<PluginLoader> loader);

  /// Get the plugin loader.
  PluginLoader &getPluginLoader();

  /// Get the output backend. The output backend needs to be initialized via
  /// constructor or `setOutputBackend`.
  toolchain::vfs::OutputBackend &getOutputBackend() const {
    assert(OutputBackend && "OutputBackend is not setup");
    return *OutputBackend;
  }
  /// Set output backend for virtualized outputs.
  void setOutputBackend(
      toolchain::IntrusiveRefCntPtr<toolchain::vfs::OutputBackend> OutBackend) {
    OutputBackend = std::move(OutBackend);
  }

private:
  friend Decl;

  std::optional<ExternalSourceLocs *> getExternalSourceLocs(const Decl *D);
  void setExternalSourceLocs(const Decl *D, ExternalSourceLocs *Locs);

  friend TypeBase;
  friend ArchetypeType;
  friend OpaqueTypeDecl;

  /// Provide context-level uniquing for SIL lowered type layouts and boxes.
  friend SILLayout;
  friend SILBoxType;

public:
  clang::DarwinSDKInfo *getDarwinSDKInfo() const;

  /// Returns the availability domain corresponding to the target triple. If
  /// there isn't a `PlatformKind` associated with the current target triple,
  /// then this returns the universal domain (`*`).
  AvailabilityDomain getTargetAvailabilityDomain() const;
};

} // end namespace language

#endif
