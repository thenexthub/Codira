//===--- ModuleFile.h - Info about a loaded serialized module ---*- C++ -*-===//
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

#ifndef LANGUAGE_SERIALIZATION_MODULEFILE_H
#define LANGUAGE_SERIALIZATION_MODULEFILE_H

#include "ModuleFileSharedCore.h"
#include "ModuleFormat.h"
#include "language/AST/FileUnit.h"
#include "language/AST/Identifier.h"
#include "language/AST/LazyResolver.h"
#include "language/AST/LinkLibrary.h"
#include "language/AST/Module.h"
#include "language/AST/SILLayout.h"
#include "language/Basic/BasicSourceInfo.h"
#include "language/Basic/Toolchain.h"
#include "language/Serialization/Validation.h"
#include "clang/AST/Type.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/TinyPtrVector.h"
#include "toolchain/Bitstream/BitstreamReader.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/MemoryBuffer.h"

namespace toolchain {
  class BitstreamCursor;
  class BitstreamReader;
  class MemoryBuffer;
  template <typename Info> class OnDiskIterableChainedHashTable;
}

namespace language {
class Decl;
class FileUnit;
class ModuleDecl;
class Pattern;
class ProtocolConformance;

/// A serialized module, along with the tools to access it.
class ModuleFile
  : public LazyMemberLoader,
    public LazyConformanceLoader {
  friend class SerializedASTFile;
  friend class DeclDeserializer;
  friend class TypeDeserializer;
  friend class SILDeserializer;
  friend class ProtocolConformanceDeserializer;
  template <serialization::decls_block::detail::TypeRecords TypeRecord>
  friend class serialization::decls_block::detail::TypeRecordDispatch;
  friend struct serialization::decls_block::detail::function_deserializer;
  using Status = serialization::Status;
  using TypeID = serialization::TypeID;
  using ProtocolConformanceID = serialization::ProtocolConformanceID;

  /// The core data of a serialized module file. This is accessed as immutable
  /// and thread-safe.
  const std::shared_ptr<const ModuleFileSharedCore> Core;

  /// A reference back to the AST representation of the file.
  FileUnit *FileContext = nullptr;

  /// The module that this module is an overlay of, if any.
  ModuleDecl *UnderlyingModule = nullptr;

  /// Once this module file has been associated with the AST node representing
  /// it, resolve the potentially-SDK-relative module-defining `.codeinterface`
  /// path to an absolute path.
  std::optional<std::string> ResolvedModuleDefiningFilename;

  /// The cursor used to lazily load things from the file.
  toolchain::BitstreamCursor DeclTypeCursor;

  toolchain::BitstreamCursor SILCursor;
  toolchain::BitstreamCursor SILIndexCursor;
  toolchain::BitstreamCursor DeclMemberTablesCursor;

public:
  static std::unique_ptr<toolchain::MemoryBuffer> getModuleName(ASTContext &Ctx,
                                                           StringRef modulePath,
                                                           std::string &Name);

  /// Represents another module that has been imported as a dependency.
  class Dependency {
  public:
    const ModuleFileSharedCore::Dependency &Core;

    std::optional<ImportedModule> Import = std::nullopt;
    SmallVector<Identifier, 4> spiGroups;

    Dependency(const ModuleFileSharedCore::Dependency &coreDependency)
      : Core(coreDependency) {}

    bool isLoaded() const {
      return Import.has_value() && Import->importedModule != nullptr;
    }

    bool isExported() const {
      return Core.isExported();
    }
    bool isImplementationOnly() const {
      return Core.isImplementationOnly();
    }
    bool isInternalOrBelow() const {
      return Core.isInternalOrBelow();
    }
    bool isPackageOnly() const {
      return Core.isPackageOnly();
    }

    bool isHeader() const { return Core.isHeader(); }
    bool isScoped() const { return Core.isScoped(); }
  };

private:
  /// All modules this module depends on.
  SmallVector<Dependency, 8> Dependencies;

  /// External macro modules.
  SmallVector<ExternalMacroPlugin, 4> MacroModuleNames;

public:
  template <typename T>
  class Serialized {
  private:
    using RawBitOffset = uint64_t;

    using ImplTy = PointerUnion<T, serialization::BitOffset>;
    ImplTy Value;

  public:
    /*implicit*/ Serialized(serialization::BitOffset offset) : Value(offset) {}

    bool isComplete() const {
      return Value.template is<T>();
    }

    T get() const {
      return Value.template get<T>();
    }

    /*implicit*/ operator T() const {
      return get();
    }

    /*implicit*/ operator serialization::BitOffset() const {
      return Value.template get<serialization::BitOffset>();
    }

    /*implicit*/ operator RawBitOffset() const {
      return Value.template get<serialization::BitOffset>();
    }

    Serialized &operator=(T deserialized) {
      assert(!isComplete() || ImplTy(deserialized) == Value);
      Value = deserialized;
      return *this;
    }

    void uncheckedOverwrite(T t) {
      Value = t;
    }
  };

  /// A class for holding a value that can be partially deserialized.
  ///
  /// This class assumes that "T()" is not a valid deserialized value.
  template <typename T>
  class PartiallySerialized {
  private:
    using RawBitOffset = decltype(DeclTypeCursor.GetCurrentBitNo());

    /// The deserialized value.
    T Value;

    /// The offset.
    unsigned Offset : 31;

    unsigned IsFullyDeserialized : 1;

  public:
    /*implicit*/ PartiallySerialized(serialization::BitOffset offset)
      : Value(), Offset(offset), IsFullyDeserialized(0) {}

    /*implicit*/ PartiallySerialized(RawBitOffset offset)
      : Value(), Offset(static_cast<unsigned>(offset)), IsFullyDeserialized(0) {
      assert(Offset == offset && "offset is too large");
    }

    bool isDeserialized() const {
      return Value != T();
    }

    bool isFullyDeserialized() const {
      return isDeserialized() && IsFullyDeserialized;
    }

    serialization::BitOffset getOffset() const {
      assert(!isFullyDeserialized());
      return Offset;
    }

    T get() const {
      assert(isDeserialized());
      return Value;
    }

    void reset() {
      IsFullyDeserialized = 0;
      Value = T();
    }

    void set(T value, bool isFullyDeserialized) {
      assert(!isDeserialized() || Value == value);
      Value = value;
      IsFullyDeserialized = isFullyDeserialized;
    }
  };

private:
  /// An allocator for buffers owned by the file.
  toolchain::BumpPtrAllocator Allocator;

  /// Allocates a buffer using #Allocator and initializes it with the contents
  /// of the container \p rawData, then stores it in \p buffer.
  ///
  /// \p buffer is passed as an argument rather than returned so that the
  /// element type can be inferred.
  template <typename T, typename RawData>
  void allocateBuffer(MutableArrayRef<T> &buffer, const RawData &rawData);

  /// Allocates a buffer using #Allocator and initializes it with the contents
  /// of the container \p rawData, then stores it in \p buffer.
  ///
  /// \p buffer is passed as an argument rather than returned so that the
  /// element type can be inferred.
  template <typename T, typename RawData>
  void allocateBuffer(ArrayRef<T> &buffer, const RawData &rawData) {
    assert(buffer.empty());
    MutableArrayRef<T> result;
    allocateBuffer(result, rawData);
    buffer = result;
  }

  /// Decls referenced by this module.
  MutableArrayRef<Serialized<Decl*>> Decls;

  /// Local DeclContexts referenced by this module.
  MutableArrayRef<Serialized<DeclContext*>> LocalDeclContexts;

  /// Protocol conformances referenced by this module.
  MutableArrayRef<Serialized<ProtocolConformance *>> Conformances;

  /// Abstract conformances referenced by this module.
  MutableArrayRef<Serialized<AbstractConformance *>> AbstractConformances;

  /// Pack conformances referenced by this module.
  MutableArrayRef<Serialized<PackConformance *>> PackConformances;

  /// SILLayouts referenced by this module.
  MutableArrayRef<Serialized<SILLayout *>> SILLayouts;

  /// Types referenced by this module.
  MutableArrayRef<Serialized<Type>> Types;

  /// Clang types referenced by this module.
  MutableArrayRef<Serialized<const clang::Type *>> ClangTypes;

  /// Generic signatures referenced by this module.
  MutableArrayRef<Serialized<GenericSignature>> GenericSignatures;

  /// Generic environments referenced by this module.
  MutableArrayRef<Serialized<GenericEnvironment *>> GenericEnvironments;

  /// Substitution maps referenced by this module.
  MutableArrayRef<Serialized<SubstitutionMap>> SubstitutionMaps;

  uint64_t
  createLazyConformanceLoaderToken(ArrayRef<uint64_t> ids);
  ArrayRef<ProtocolConformanceID>
  claimLazyConformanceLoaderToken(uint64_t token);

  /// Represents an identifier that may or may not have been deserialized yet.
  ///
  /// If \c Ident is empty, the identifier has not been loaded yet.
  class SerializedIdentifier {
  public:
    Identifier Ident;
    unsigned Offset;

    template <typename IntTy>
    /*implicit*/ SerializedIdentifier(IntTy rawOffset)
      : Offset(static_cast<unsigned>(rawOffset)) {
      assert(Offset == rawOffset && "not enough bits");
    }
  };

  /// Identifiers referenced by this module.
  MutableArrayRef<SerializedIdentifier> Identifiers;

  using SerializedDeclMembersTable =
      ModuleFileSharedCore::SerializedDeclMembersTable;

  toolchain::DenseMap<uint32_t,
           std::unique_ptr<SerializedDeclMembersTable>> DeclMembersTables;

  toolchain::DenseMap<const Decl *, Identifier> PrivateDiscriminatorsByValue;
  toolchain::DenseMap<const Decl *, StringRef> FilenamesForPrivateValues;

  TinyPtrVector<Decl *> ImportDecls;

  /// Maps USRs to their deserialized comment object.
  mutable toolchain::StringMap<
      std::unique_ptr<ModuleFileSharedCore::DeserializedCommentInfo>>
      CommentsCache;

  struct ModuleBits {
    /// Whether or not ImportDecls is valid.
    unsigned ComputedImportDecls : 1;

    /// Whether an error has been detected setting up this module file.
    unsigned HasError : 1;

    // Explicitly pad out to the next word boundary.
    unsigned : 0;
  } Bits = {};
  static_assert(sizeof(ModuleBits) <= 8, "The bit set should be small");

  bool hasError() const {
    return Bits.HasError || Core->hasError();
  }

  /// Whether or not this module file comes from a context that had a main entry point.
  bool hasEntryPoint() const {
    return Core->Bits.HasEntryPoint;
  }

  /// The decl ID of the main class in this module file, if it has one.
  unsigned getEntryPointDeclID() const {
    return Core->Bits.EntryPointDeclID;
  }

  /// Creates a new AST node to represent a deserialized decl.
  template <typename T, typename ...Args>
  T *createDecl(Args &&... args);

  Decl *handleErrorAndSupplyMissingMiscMember(toolchain::Error &&error) const;

  Decl *handleErrorAndSupplyMissingMember(ASTContext &context,
                                          Decl *container,
                                          toolchain::Error &&error) const;
  Decl *handleErrorAndSupplyMissingClassMember(
      ASTContext &context, toolchain::Error &&error,
      ClassDecl *containingClass) const;

public:
  /// Change the status of the current module.
  Status error(Status issue) {
    assert(issue != Status::Valid);
    assert((issue != Status::Malformed || !FileContext) &&
           "too late to complain about the well-formedness of the module");
    Bits.HasError = true;
    return issue;
  }

  /// Enrich \c error with contextual information, emits a fatal diagnostic in
  /// the ASTContext's DiagnosticsEngine, and return the augmented error.
  ///
  /// The `diagnoseFatal` methods return only in LLDB where high error
  /// tolerance is expected, or when hitting a project error. During normal
  /// compilation, most calls won't return and lead to a compiler crash.
  toolchain::Error diagnoseFatal(toolchain::Error error) const;

  /// Emit a generic deserialization error via \c diagnoseFatal().
  toolchain::Error diagnoseFatal() const {
    return diagnoseFatal(createFatalError());
  }

  /// Emit a fatal error via \c diagnoseFatal() and consume it.
  void diagnoseAndConsumeFatal(toolchain::Error error) const {
    toolchain::consumeError(diagnoseFatal(std::move(error)));
  }

  /// Emit a generic fatal error via \c diagnoseFatal() and consume it.
  void diagnoseAndConsumeFatal() const {
    toolchain::consumeError(diagnoseFatal());
  }

  /// Use this in \p void functions as:
  ///
  ///    if (diagnoseAndConsumeFatalIfNotSuccess(...)) return;
  bool diagnoseAndConsumeFatalIfNotSuccess(toolchain::Error error) const {
    if (!error)
      return false;
    toolchain::consumeError(diagnoseFatal(std::move(error)));
    return true;
  }

  /// Use this in functions that return Expected<T> as:
  ///
  ///    if (auto error = diagnoseFatalIfNotSuccess(...)) return error;
  toolchain::Error diagnoseFatalIfNotSuccess(toolchain::Error error) const {
    if (!error)
      return error;
    return diagnoseFatal(std::move(error));
  }

  /// Emit and return a string error via \c diagnoseFatal().
  toolchain::Error diagnoseFatal(StringRef msg) const {
    return diagnoseFatal(toolchain::make_error<toolchain::StringError>(
        msg, toolchain::inconvertibleErrorCode()));
  }

  /// Emit and consume a string error via \c diagnoseFatal().
  void diagnoseAndConsumeFatal(StringRef msg) const {
    return toolchain::consumeError(diagnoseFatal(msg));
  }

  /// Consume errors that are usually safe to ignore because they
  /// are expected to support language features or caused by project
  /// misconfigurations.
  ///
  /// If the error is handled, success is returned, otherwise the original
  /// error is returned.
  toolchain::Error consumeExpectedError(toolchain::Error &&error);

  /// Report project errors as remarks if desired, otherwise drop the error
  /// silently.
  void diagnoseAndConsumeError(toolchain::Error error) const;

  /// Report modularization error, either directly or indirectly if it
  /// caused a \c TypeError or \c ExtensionError. Returns any unhandled errors.
  toolchain::Error
  diagnoseModularizationError(toolchain::Error error,
                   DiagnosticBehavior limit = DiagnosticBehavior::Fatal) const;

  /// Report an unexpected format error that could happen only from a
  /// memory-level inconsistency. Please prefer passing an error to
  /// `fatal(toolchain::Error error)` when possible.
  static toolchain::Error createFatalError(
      toolchain::StringRef msg =
          "Memory corruption or serialization format inconsistency.") {
    return toolchain::make_error<toolchain::StringError>(msg,
                                               toolchain::inconvertibleErrorCode());
  }

  /// Emit a fatal error and abort.  This function is deprecated, try to use
  /// diagnoseFatal() instead. Clients such as LLDB really prefer not to be
  /// killed.
//  TOOLCHAIN_DEPRECATED("Use diagnoseFatal and pass up the error instead.",
//                  "diagnoseFatal")
  [[noreturn]] void fatal(toolchain::Error error = createFatalError()) const;

  /// Emit a fatal error and abort.  This function is deprecated, try to use
  /// diagnoseFatal() instead. Clients such as LLDB really prefer not to be
  /// killed.
//  TOOLCHAIN_DEPRECATED("Use diagnoseFatal and pass up the error instead.",
//                  "diagnoseFatal")
  [[noreturn]] void fatal(toolchain::StringRef msg) const {
    fatal(createFatalError(msg));
  }

  /// Emit a fatal error and abort if unexpected. Try to avoid using this
  /// function. See comment in \p fatal().
//  TOOLCHAIN_DEPRECATED("Use diagnoseFatal and pass up the error instead.",
//                  "diagnoseFatal")
  template <typename T>
  T fatalIfUnexpected(toolchain::Expected<T> expected) const {
    if (expected)
      return std::move(expected.get());
    fatal(expected.takeError());
  }

  /// Outputs information useful for diagnostics to \p out
  void outputDiagnosticInfo(toolchain::raw_ostream &os) const;

  ASTContext &getContext() const {
    assert(FileContext && "no associated context yet");
    return FileContext->getParentModule()->getASTContext();
  }

  ModuleDecl *getAssociatedModule() const {
    assert(FileContext && "no associated context yet");
    return FileContext->getParentModule();
  }

  FileUnit *getFile() const {
    assert(FileContext && "no associated context yet");
    return FileContext;
  }

private:
  /// Recursively reads a pattern from \c DeclTypeCursor.
  toolchain::Expected<Pattern *> readPattern(DeclContext *owningDC);

  toolchain::Expected<ParameterList *> readParameterList();
  
  /// Reads a generic param list from \c DeclTypeCursor.
  ///
  /// If the record at the cursor is not a generic param list, returns null
  /// without moving the cursor.
  toolchain::Expected<GenericParamList *> maybeReadGenericParams(DeclContext *DC);

  /// Reads a set of requirements from \c DeclTypeCursor.
  void deserializeGenericRequirements(ArrayRef<uint64_t> scratch,
                                      unsigned &nextIndex,
                                 SmallVectorImpl<Requirement> &requirements);

  /// Reads a set of requirements from \c DeclTypeCursor, returns the first
  /// error, if any.
  toolchain::Error
  deserializeGenericRequirementsChecked(ArrayRef<uint64_t> scratch,
                                        unsigned &nextIndex,
                                   SmallVectorImpl<Requirement> &requirements);

  /// Read the requirement signature of a protocol, which consists of a list of
  /// generic requirements and a list of protocol typealias records.
  void readRequirementSignature(SmallVectorImpl<Requirement> &requirements,
                                SmallVectorImpl<ProtocolTypeAlias> &typeAliases,
                                toolchain::BitstreamCursor &Cursor);

  /// Read a list of associated type declarations in a protocol.
  void readAssociatedTypes(
      SmallVectorImpl<AssociatedTypeDecl *> &assocTypes,
      toolchain::BitstreamCursor &Cursor);

  /// Read a list of primary associated type declarations in a protocol.
  void readPrimaryAssociatedTypes(
      SmallVectorImpl<AssociatedTypeDecl *> &assocTypes,
      toolchain::BitstreamCursor &Cursor);

  /// Read a list of the protocol declarations inherited by another protocol.
  bool readInheritedProtocols(SmallVectorImpl<ProtocolDecl *> &inherited);

  /// Populates the protocol's default witness table.
  ///
  /// Returns true if there is an error.
  ///
  /// Note: this destroys the cursor's position in the stream. Furthermore,
  /// because it reads from the cursor, it is not possible to reset the cursor
  /// after reading. Nothing should ever follow a DEFAULT_WITNESS_TABLE record.
  bool readDefaultWitnessTable(ProtocolDecl *proto);

  /// Resolves a cross-reference, starting from the given module.
  ///
  /// Note: this destroys the cursor's position in the stream. Furthermore,
  /// because it reads from the cursor, it is not possible to reset the cursor
  /// after reading. Nothing should ever follow an XREF record except
  /// XREF_PATH_PIECE records.
  toolchain::Expected<Decl *> resolveCrossReference(serialization::ModuleID MID,
                                               uint32_t pathLen);

  struct AccessorRecord {
    SmallVector<serialization::DeclID, 8> IDs;
  };

  /// Sets the accessors for \p storage based on \p rawStorageKind.
  void configureStorage(AbstractStorageDecl *storage,
                        uint8_t rawOpaqueReadOwnership,
                        uint8_t rawReadImpl,
                        uint8_t rawWriteImpl,
                        uint8_t rawReadWriteImpl,
                        AccessorRecord &accessors);

public:
  /// Constructs a new module.
  explicit ModuleFile(std::shared_ptr<const ModuleFileSharedCore> core);

  // Out of line to avoid instantiation OnDiskChainedHashTable here.
  ~ModuleFile();

  /// The name of the module.
  StringRef getName() const {
    return Core->Name;
  }

  StringRef getModulePackageName() const {
    return Core->ModulePackageName;
  }

  StringRef getModuleExportAsName() const {
    return Core->ModuleExportAsName;
  }

  StringRef getPublicModuleName() const {
    return Core->PublicModuleName;
  }

  /// The ABI name of the module.
  StringRef getModuleABIName() const {
    return Core->ModuleABIName;
  }

  toolchain::VersionTuple getUserModuleVersion() const {
    return Core->UserModuleVersion;
  }

  version::Version getCodiraInterfaceCompilerVersion() const {
    return Core->CodiraInterfaceCompilerVersion;
  }

  ArrayRef<StringRef> getAllowableClientNames() const {
    return Core->AllowableClientNames;
  }

  /// The Codira compatibility version in use when this module was built.
  const version::Version &getCompatibilityVersion() const {
    return Core->CompatibilityVersion;
  }

  /// Whether this module is compiled with `-enable-private-imports`.
  bool arePrivateImportsEnabled() const {
    return Core->Bits.ArePrivateImportsEnabled;
  }

  /// Is this module file actually a .sib file? .sib files are serialized SIL at
  /// arbitrary granularity and arbitrary stage; unlike serialized Codira
  /// modules, which are assumed to contain canonical SIL for an entire module.
  bool isSIB() const {
    return Core->Bits.IsSIB;
  }

  /// Whether this module file is compiled with '-enable-testing'.
  bool isTestable() const {
    return Core->Bits.IsTestable;
  }

  /// Whether this module is compiled as static library.
  bool isStaticLibrary() const {
    return Core->Bits.IsStaticLibrary;
  }

  /// Whether this module was built with -experimental-hermetic-seal-at-link.
  bool hasHermeticSealAtLink() const {
    return Core->Bits.HasHermeticSealAtLink;
  }

  /// Whether this module was built using embedded Codira.
  bool isEmbeddedCodiraModule() const {
    return Core->Bits.IsEmbeddedCodiraModule;
  }

  /// Whether this module was built with C++ interoperability enabled.
  bool hasCxxInteroperability() const {
    return Core->Bits.HasCxxInteroperability;
  }

  /// The kind of the C++ stdlib that this module was built with.
  CXXStdlibKind getCXXStdlibKind() const {
    return static_cast<CXXStdlibKind>(Core->Bits.CXXStdlibKind);
  }

  /// Whether the module is resilient. ('-enable-library-evolution')
  ResilienceStrategy getResilienceStrategy() const {
    return ResilienceStrategy(Core->Bits.ResilienceStrategy);
  }

  bool isBuiltFromInterface() const {
    return Core->Bits.IsBuiltFromInterface;
  }

  bool allowNonResilientAccess() const {
    return Core->Bits.AllowNonResilientAccess;
  }

  /// Whether this module was built with -experimental-package-cmo.
  bool serializePackageEnabled() const {
    return Core->Bits.SerializePackageEnabled;
  }

  /// Whether this module is compiled with implicit dynamic.
  bool isImplicitDynamicEnabled() const {
    return Core->Bits.IsImplicitDynamicEnabled;
  }

  /// Whether this module is compiled while allowing errors
  /// ('-experimental-allow-module-with-compiler-errors').
  bool compiledAllowingCompilerErrors() const {
    return Core->Bits.IsAllowModuleWithCompilerErrorsEnabled;
  }

  /// Whether currently allowing modules with compiler errors (ie.
  /// '-experimental-allow-module-with-compiler-errors' is currently enabled).
  bool allowCompilerErrors() const;

  /// Allow recovering from errors that could be unsafe when compiling
  /// the binary. Useful for the debugger and IDE support tools.
  bool enableExtendedDeserializationRecovery() const;

  /// \c true if this module has incremental dependency information.
  bool hasIncrementalInfo() const { return Core->hasIncrementalInfo(); }

  /// \c true if this module has a corresponding .codesourceinfo file.
  bool hasSourceInfoFile() const { return Core->hasSourceInfoFile(); }

  /// \c true if this module has information from a corresponding
  /// .codesourceinfo file (ie. the file exists and has been read).
  bool hasSourceInfo() const { return Core->hasSourceInfo(); }

  /// \c true if this module was built with complete checking for concurrency.
  bool isConcurrencyChecked() const { return Core->isConcurrencyChecked(); }

  /// \c true if this module was built with strict memory safety.
  bool strictMemorySafety() const { return Core->strictMemorySafety(); }

  /// Associates this module file with the AST node representing it.
  ///
  /// Checks that the file is compatible with the AST module it's being loaded
  /// into, loads any dependencies needed to understand the module, and updates
  /// the ASTContext and ClangImporter with search paths and other information
  /// from the module.
  ///
  /// \param file The FileUnit that represents this file's place in the AST.
  /// \param diagLoc A location used for diagnostics that occur during loading.
  /// This does not include diagnostics about \e this file failing to load,
  /// but rather other things that might be imported as part of bringing the
  /// file into the AST.
  /// \param recoverFromIncompatibility Whether to associate the file
  /// regardless of the compatibility with the AST module. Still returns the
  /// underlying error for diagnostic purposes but does not set the error bit.
  ///
  /// \returns any error that occurred during association, such as being
  /// compiled for a different OS.
  Status associateWithFileContext(FileUnit *file, SourceLoc diagLoc,
                                  bool recoverFromIncompatibility);

  /// Load dependencies of this module.
  ///
  /// \param file The FileUnit that represents this file's place in the AST.
  /// \param diagLoc A location used for diagnostics that occur during loading.
  /// This does not include diagnostics about \e this file failing to load,
  /// but rather other things that might be imported as part of bringing the
  /// file into the AST.
  ///
  /// \returns any error that occurred during loading dependencies.
  Status
  loadDependenciesForFileContext(const FileUnit *file, SourceLoc diagLoc,
                               bool forTestable);

  /// How should \p dependency be loaded for a transitive import via \c this?
  ModuleLoadingBehavior
  getTransitiveLoadingBehavior(const Dependency &dependency,
                               bool forTestable) const;

  /// Generate a \c SourceLoc pointing at the loaded languagemodule file.
  SourceLoc getSourceLoc() const;

  /// Returns `true` if there is a buffer that might contain source code where
  /// other parts of the compiler could have emitted diagnostics, to indicate
  /// that the object must be kept alive as long as the diagnostics exist.
  ///
  /// Should only be called when a failure has been reported from
  /// ModuleFile::load or ModuleFile::associateWithFileContext.
  bool mayHaveDiagnosticsPointingAtBuffer() const;

  /// Returns the list of modules this module depends on.
  ArrayRef<Dependency> getDependencies() const {
    return Dependencies;
  }

  /// The module that this module is an overlay for, if any.
  ModuleDecl *getUnderlyingModule() const { return UnderlyingModule; }

  /// Searches the module's top-level decls for the given identifier.
  void lookupValue(DeclName name, OptionSet<ModuleLookupFlags> flags,
                   SmallVectorImpl<ValueDecl*> &results);

  /// Searches the module's local type decls for the given mangled name.
  TypeDecl *lookupLocalType(StringRef MangledName);
      
  /// Search the module's opaque return type decls for the one corresponding to
  /// the given mangled name.
  OpaqueTypeDecl *lookupOpaqueResultType(StringRef MangledName);

  /// Searches the module's nested type decls table for the given member of
  /// the given type.
  TypeDecl *lookupNestedType(Identifier name, const NominalTypeDecl *parent);

  /// Searches the module's operators for one with the given name and fixity.
  ///
  /// If none is found, returns null.
  OperatorDecl *lookupOperator(Identifier name, OperatorFixity fixity);

  /// Searches the module's precedence groups for one with the given
  /// name and fixity.
  ///
  /// If none is found, returns null.
  PrecedenceGroupDecl *lookupPrecedenceGroup(Identifier name);

  /// Adds any imported modules to the given vector.
  void getImportedModules(SmallVectorImpl<ImportedModule> &results,
                          ModuleDecl::ImportFilter filter);

  void getExternalMacros(SmallVectorImpl<ExternalMacroPlugin> &macros);

  void getImportDecls(SmallVectorImpl<Decl *> &Results);

  /// Reports all visible top-level members in this module.
  void lookupVisibleDecls(ImportPath::Access accessPath,
                          VisibleDeclConsumer &consumer,
                          NLKind lookupKind);

  /// Loads extensions for the given decl.
  ///
  /// Note that this may cause other decls to load as well.
  void loadExtensions(NominalTypeDecl *nominal);

  /// Load the methods within the given nominal type that produce
  /// Objective-C class or instance methods with the given selector.
  ///
  /// \param typeDecl The nominal in which we are searching for @objc methods.
  /// The search only considers this type and its extensions; not any
  /// superclasses.
  ///
  /// \param selector The selector to search for.
  ///
  /// \param isInstanceMethod Whether we are looking for an instance method
  /// (vs. a class method).
  ///
  /// \param methods The list of @objc methods in this class that have this
  /// selector and are instance/class methods as requested.
  void loadObjCMethods(NominalTypeDecl *typeDecl,
                       ObjCSelector selector,
                       bool isInstanceMethod,
                       toolchain::TinyPtrVector<AbstractFunctionDecl *> &methods);

  /// Loads all derivative function configurations for the given
  /// AbstractFunctionDecl.
  void loadDerivativeFunctionConfigurations(
      AbstractFunctionDecl *originalAFD,
      toolchain::SetVector<AutoDiffConfig> &results);

  /// Reports all class members in the module to the given consumer.
  ///
  /// This is intended for use with id-style lookup and code completion.
  void lookupClassMembers(ImportPath::Access accessPath,
                          VisibleDeclConsumer &consumer);

  /// Adds class members in the module with the given name to the given vector.
  ///
  /// This is intended for use with id-style lookup.
  void lookupClassMember(ImportPath::Access accessPath,
                         DeclName name,
                         SmallVectorImpl<ValueDecl*> &results);

  /// Find all Objective-C methods with the given selector.
  void lookupObjCMethods(
         ObjCSelector selector,
         SmallVectorImpl<AbstractFunctionDecl *> &results);

  /// Reports all link-time dependencies.
  void collectLinkLibraries(ModuleDecl::LinkLibraryCallback callback) const;

  /// Adds all top-level decls to the given vector.
  ///
  /// \param Results Vector collecting the decls.
  ///
  /// \param matchAttributes Optional check on the attributes of a decl to
  /// filter which decls to fully deserialize. Only decls with accepted
  /// attributes are deserialized and added to Results.
  void getTopLevelDecls(
         SmallVectorImpl<Decl*> &Results,
         toolchain::function_ref<bool(DeclAttributes)> matchAttributes = nullptr);

  void getExportedPrespecializations(SmallVectorImpl<Decl *> &results);

  /// Adds all operators to the given vector.
  void getOperatorDecls(SmallVectorImpl<OperatorDecl *> &Results);

  /// Adds all precedence groups to the given vector.
  void getPrecedenceGroups(SmallVectorImpl<PrecedenceGroupDecl*> &Results);

  /// Adds all local type decls to the given vector.
  void getLocalTypeDecls(SmallVectorImpl<TypeDecl*> &Results);
      
  /// Add all opaque return type decls in the module to the given vector.
  void getOpaqueReturnTypeDecls(SmallVectorImpl<OpaqueTypeDecl*> &Results);

  /// Adds all top-level decls to the given vector.
  ///
  /// This includes all decls that should be displayed to clients of the module.
  /// This can differ from \c getTopLevelDecls, e.g. it returns decls from a
  /// shadowed clang module.
  void getDisplayDecls(SmallVectorImpl<Decl*> &results, bool recursive = false);

  StringRef getModuleFilename() const {
    if (ResolvedModuleDefiningFilename)
      return ResolvedModuleDefiningFilename.value();
    if (!Core->ModuleInterfacePath.empty())
      return Core->ModuleInterfacePath;
    return getModuleLoadedFilename();
  }

  StringRef getModuleLoadedFilename() const {
    // FIXME: This seems fragile, maybe store the filename separately?
    return Core->ModuleInputBuffer->getBufferIdentifier();
  }

  StringRef getModuleSourceFilename() const {
    if (!Core->CorrespondingInterfacePath.empty())
      return Core->CorrespondingInterfacePath;
    return getModuleFilename();
  }

  StringRef getTargetTriple() const {
    return Core->TargetTriple;
  }

  /// AST-verify imported decls.
  ///
  /// Has no effect in NDEBUG builds.
  void verify() const;

  virtual void loadAllMembers(Decl *D,
                              uint64_t contextData) override;

  virtual TinyPtrVector<ValueDecl *>
  loadNamedMembers(const IterableDeclContext *IDC, DeclBaseName N,
                   uint64_t contextData) override;

  virtual void
  loadAllConformances(const Decl *D, uint64_t contextData,
                    SmallVectorImpl<ProtocolConformance*> &Conforms) override;

  virtual Type loadAssociatedTypeDefault(const AssociatedTypeDecl *ATD,
                                         uint64_t contextData) override;

  virtual ValueDecl *
  loadDynamicallyReplacedFunctionDecl(const DynamicReplacementAttr *DRA,
                                      uint64_t contextData) override;

  virtual ValueDecl *loadTargetFunctionDecl(const AbstractSpecializeAttr *attr,
                                            uint64_t contextData) override;
  virtual AbstractFunctionDecl *
  loadReferencedFunctionDecl(const DerivativeAttr *DA,
                             uint64_t contextData) override;

  virtual Type loadTypeEraserType(const TypeEraserAttr *TRA,
                                  uint64_t contextData) override;

  virtual void finishNormalConformance(NormalProtocolConformance *conformance,
                                       uint64_t contextData) override;

  void
  loadRequirementSignature(const ProtocolDecl *proto, uint64_t contextData,
                           SmallVectorImpl<Requirement> &requirements,
                           SmallVectorImpl<ProtocolTypeAlias> &typeAliases) override;

  void
  loadAssociatedTypes(
      const ProtocolDecl *proto, uint64_t contextData,
      SmallVectorImpl<AssociatedTypeDecl *> &assocTypes) override;

  void
  loadPrimaryAssociatedTypes(
      const ProtocolDecl *proto, uint64_t contextData,
      SmallVectorImpl<AssociatedTypeDecl *> &assocTypes) override;

  std::optional<StringRef> getGroupNameById(unsigned Id) const;
  std::optional<StringRef> getSourceFileNameById(unsigned Id) const;
  std::optional<StringRef> getGroupNameForDecl(const Decl *D) const;
  std::optional<StringRef> getSourceFileNameForDecl(const Decl *D) const;
  std::optional<unsigned> getSourceOrderForDecl(const Decl *D) const;
  void collectAllGroups(SmallVectorImpl<StringRef> &Names) const;
  std::optional<CommentInfo> getCommentForDecl(const Decl *D) const;
  bool hasLoadedCodiraDoc() const;
  std::optional<CommentInfo> getCommentForDeclByUSR(StringRef USR) const;
  std::optional<StringRef> getGroupNameByUSR(StringRef USR) const;
  std::optional<ExternalSourceLocs::RawLocs>
  getExternalRawLocsForDecl(const Decl *D) const;
  Identifier getDiscriminatorForPrivateDecl(const Decl *D);
  std::optional<Fingerprint>
  loadFingerprint(const IterableDeclContext *IDC) const;
  void collectBasicSourceFileInfo(
      toolchain::function_ref<void(const BasicSourceFileInfo &)> callback) const;
  void collectSerializedSearchPath(
      toolchain::function_ref<void(StringRef)> callback) const;

  // MARK: Deserialization interface

  toolchain::BitstreamCursor getSILCursor() const {
    return SILCursor;
  }
  toolchain::BitstreamCursor getSILIndexCursor() const {
    return SILIndexCursor;
  }

  /// Returns the type with the given ID, deserializing it if needed.
  ///
  /// \sa getTypeChecked
  Type getType(serialization::TypeID TID);

  /// Returns the type with the given ID, deserializing it if needed.
  toolchain::Expected<Type> getTypeChecked(serialization::TypeID TID);

  /// Returns the Clang type with the given ID, deserializing it if needed.
  toolchain::Expected<const clang::Type *>
  getClangType(serialization::ClangTypeID TID);

  /// Returns the base name with the given ID, deserializing it if needed.
  DeclBaseName getDeclBaseName(serialization::IdentifierID IID);

  /// Convenience method to retrieve the identifier backing the name with
  /// given ID. Asserts that the name with this ID is not special.
  Identifier getIdentifier(serialization::IdentifierID IID);

  /// Convenience method to retrieve the text of the name with the given ID.
  /// This can be used if the result doesn't need to be uniqued in the
  /// ASTContext. Asserts that the name with this ID is not special.
  StringRef getIdentifierText(serialization::IdentifierID IID);

  /// Returns the decl with the given ID, deserializing it if needed.
  ///
  /// \param DID The ID for the decl within this module.
  /// \sa getDeclChecked
  Decl *getDecl(serialization::DeclID DID);

  /// Returns the decl with the given ID, deserializing it if needed.
  ///
  /// \param DID The ID for the decl within this module.
  ///
  /// \param matchAttributes Optional check on the attributes of the decl to
  /// determine if it should be fully deserialized and returned. If the
  /// attributes fail the check, the decl is not deserialized and
  /// \c DeclAttributesDidNotMatch is returned.
  toolchain::Expected<Decl *>
  getDeclChecked(
    serialization::DeclID DID,
    toolchain::function_ref<bool(DeclAttributes)> matchAttributes = nullptr);

  /// Returns the decl context with the given ID, deserializing it if needed.
  DeclContext *getDeclContext(serialization::DeclContextID DID);

  /// Returns the decl context with the given ID, deserializing it if needed,
  /// or the first error.
  toolchain::Expected<DeclContext *>
  getDeclContextChecked(serialization::DeclContextID DCID);

  /// Returns the local decl context with the given ID, deserializing it if needed.
  toolchain::Expected<DeclContext *>
  getLocalDeclContext(serialization::LocalDeclContextID DID);

  /// Returns the appropriate module for the given ID.
  ModuleDecl *getModule(serialization::ModuleID MID);

  /// Returns the appropriate module for the given name.
  ///
  /// If the name matches the name of the current module, a shadowed module
  /// is loaded instead.
  ModuleDecl *getModule(ImportPath::Module name, bool allowLoading = false);

  /// Returns the generic signature for the given ID.
  GenericSignature getGenericSignature(serialization::GenericSignatureID ID);

  /// Returns the generic signature for the given ID or the first error.
  toolchain::Expected<GenericSignature>
  getGenericSignatureChecked(serialization::GenericSignatureID ID);

  /// Returns the generic environment for the given ID or the first error.
  toolchain::Expected<GenericEnvironment *>
  getGenericEnvironmentChecked(serialization::GenericEnvironmentID ID);

  /// Returns the substitution map for the given ID, deserializing it if
  /// needed.
  SubstitutionMap getSubstitutionMap(serialization::SubstitutionMapID id);

  /// Returns the substitution map for the given ID, deserializing it if
  /// needed, or the first error.
  toolchain::Expected<SubstitutionMap>
  getSubstitutionMapChecked(serialization::SubstitutionMapID id);

  /// Returns the protocol conformance for the given ID.
  ProtocolConformanceRef
  getConformance(serialization::ProtocolConformanceID id);

  /// Returns the protocol conformance for the given ID.
  toolchain::Expected<ProtocolConformanceRef>
  getConformanceChecked(serialization::ProtocolConformanceID id);

  /// Read a SILLayout from the given cursor.
  SILLayout *readSILLayout(toolchain::BitstreamCursor &Cursor);

  /// Reads a foreign error convention from \c DeclTypeCursor, if present.
  std::optional<ForeignErrorConvention> maybeReadForeignErrorConvention();

  /// Reads a foreign async convention from \c DeclTypeCursor, if present.
  std::optional<ForeignAsyncConvention> maybeReadForeignAsyncConvention();

  bool maybeReadLifetimeDependenceRecord(SmallVectorImpl<uint64_t> &scratch);

  // Reads lifetime dependence info from type if present.
  std::optional<LifetimeDependenceInfo> maybeReadLifetimeDependence();

  // Reads lifetime dependence specifier from decl if present
  bool maybeReadLifetimeEntry(SmallVectorImpl<LifetimeEntry> &specifierList,
                              unsigned numDeclParams, bool hasSelf);

  /// Reads inlinable body text from \c DeclTypeCursor, if present.
  std::optional<StringRef> maybeReadInlinableBodyText();

  /// Reads pattern initializer text from \c DeclTypeCursor, if present.
  std::optional<StringRef> maybeReadPatternInitializerText();
};

template <typename T, typename RawData>
void ModuleFile::allocateBuffer(MutableArrayRef<T> &buffer,
                                const RawData &rawData) {
  assert(buffer.empty() && "reallocating deserialized buffer");
  if (rawData.empty())
    return;

  void *rawBuffer = Allocator.Allocate(sizeof(T) * rawData.size(), alignof(T));
  buffer = toolchain::MutableArrayRef(static_cast<T *>(rawBuffer), rawData.size());
  std::uninitialized_copy(rawData.begin(), rawData.end(), buffer.begin());
}

} // end namespace language

#endif
