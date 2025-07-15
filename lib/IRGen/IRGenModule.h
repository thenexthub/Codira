//===--- IRGenModule.h - Codira Global IR Generation Module ------*- C++ -*-===//
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
// This file defines the interface used 
// the AST into LLVM IR.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_IRGENMODULE_H
#define LANGUAGE_IRGEN_IRGENMODULE_H

#include "Callee.h"
#include "IRGen.h"
#include "CodiraTargetInfo.h"
#include "TypeLayout.h"
#include "language/AST/Decl.h"
#include "language/AST/IRGenOptions.h"
#include "language/AST/LinkLibrary.h"
#include "language/AST/Module.h"
#include "language/AST/ReferenceCounting.h"
#include "language/AST/SourceFile.h"
#include "language/AST/SynthesizedFileUnit.h"
#include "language/Basic/ClusteredBitVector.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/OptimizationMode.h"
#include "language/Basic/SuccessorMap.h"
#include "language/IRGen/ValueWitness.h"
#include "language/SIL/RuntimeEffect.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILModule.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/DenseSet.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/MapVector.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/IR/Attributes.h"
#include "toolchain/IR/CallingConv.h"
#include "toolchain/IR/Constant.h"
#include "toolchain/IR/Instructions.h"
#include "toolchain/IR/ValueHandle.h"
#include "toolchain/Target/TargetMachine.h"

#include <atomic>

namespace toolchain {
  class Constant;
  class ConstantInt;
  class DataLayout;
  class Function;
  class FunctionType;
  class GlobalVariable;
  class InlineAsm;
  class IntegerType;
  class LLVMContext;
  class MDNode;
  class Metadata;
  class Module;
  class PointerType;
  class StructType;
  class StringRef;
  class Type;
  class AttributeList;
}
namespace clang {
  class ASTContext;
  template <class> class CanQual;
  class CodeGenerator;
  class CXXDestructorDecl;
  class Decl;
  class GlobalDecl;
  class Type;
  class ObjCProtocolDecl;
  class PointerAuthSchema;
  namespace CodeGen {
    class CGFunctionInfo;
    class CodeGenModule;
  }
}

namespace language {
  class GenericSignature;
  class AssociatedConformance;
  class ASTContext;
  class BaseConformance;
  class BraceStmt;
  class CanType;
  class GeneratedModule;
  class GenericRequirement;
  class LinkLibrary;
  class SILFunction;
  class IRGenOptions;
  class NormalProtocolConformance;
  class ProtocolConformance;
  class ProtocolCompositionType;
  class RootProtocolConformance;
  class SILCoverageMap;
  struct SILDeclRef;
  class SILDefaultWitnessTable;
  class SILDifferentiabilityWitness;
  class SILGlobalVariable;
  class SILModule;
  class SILDefaultOverrideTable;
  class SILProperty;
  class SILType;
  class SILVTable;
  class SILWitnessTable;
  class SourceLoc;
  class SourceFile;
  class Type;
  enum class TypeReferenceKind : unsigned;

namespace Lowering {
  class TypeConverter;
}

namespace irgen {
  class Address;
  class ClangTypeConverter;
  class ClassMetadataLayout;
  class ConformanceDescription;
  class ConformanceInfo;
  class ConstantInitBuilder;
  struct ConstantIntegerLiteral;
  class ConstantIntegerLiteralMap;
  class DebugTypeInfo;
  class EnumImplStrategy;
  class EnumMetadataLayout;
  class ExplosionSchema;
  class FixedTypeInfo;
  class ForeignClassMetadataLayout;
  class ForeignFunctionInfo;
  class FormalType;
  class FunctionPointerKind;
  class HeapLayout;
  class StructLayout;
  class IRGenDebugInfo;
  class IRGenFunction;
  struct IRLinkage;
  class LinkEntity;
  class LoadableTypeInfo;
  class MetadataLayout;
  class NecessaryBindings;
  class NominalMetadataLayout;
  class OutliningMetadataCollector;
  class PointerAuthEntity;
  class ProtocolInfo;
  enum class ProtocolInfoKind : uint8_t;
  class Signature;
  class StructMetadataLayout;
  struct SymbolicMangling;
  class TypeConverter;
  class TypeInfo;
  enum class TypeMetadataAddress;
  enum class ValueWitness : unsigned;
  enum class ClassMetadataStrategy;

class IRGenModule;

/// A type descriptor for a field type accessor.
class FieldTypeInfo {
  toolchain::PointerIntPair<CanType, 2, unsigned> Info;
  /// Bits in the "int" part of the Info pair.
  enum : unsigned {
    /// Flag indicates that the case is indirectly stored in a box.
    Indirect = 1,
    /// Indicates a weak optional reference
    Weak = 2,
  };

  static unsigned getFlags(bool indirect, bool weak) {
    return (indirect ? Indirect : 0)
         | (weak ? Weak : 0);
    //   | (blah ? Blah : 0) ...
  }

public:
  FieldTypeInfo(CanType type, bool indirect, bool weak)
    : Info(type, getFlags(indirect, weak))
  {}

  CanType getType() const { return Info.getPointer(); }
  bool isIndirect() const { return Info.getInt() & Indirect; }
  bool isWeak() const { return Info.getInt() & Weak; }
  bool hasFlags() const { return Info.getInt() != 0; }
};

enum RequireMetadata_t : bool {
  DontRequireMetadata = false,
  RequireMetadata = true
};

enum class TypeMetadataCanonicality : bool {
  Noncanonical,
  Canonical,
};

/// The principal singleton which manages all of IR generation.
///
/// The IRGenerator delegates the emission of different top-level entities
/// to different instances of IRGenModule, each of which creates a different
/// toolchain::Module.
///
/// In single-threaded compilation, the IRGenerator creates only a single
/// IRGenModule. In multi-threaded compilation, it contains multiple
/// IRGenModules - one for each LLVM module (= one for each input/output file).
class IRGenerator {
public:
  const IRGenOptions &Opts;

  SILModule &SIL;

private:
  toolchain::DenseMap<SourceFile *, IRGenModule *> GenModules;
  
  // Stores the IGM from which a function is referenced the first time.
  // It is used if a function has no source-file association.
  toolchain::DenseMap<SILFunction *, IRGenModule *> DefaultIGMForFunction;

  // The IGMs where specializations of functions are emitted. The key is the
  // non-specialized function.
  // Storing all specializations of a function in the same IGM increases the
  // chances of function merging.
  toolchain::DenseMap<const SILFunction *, IRGenModule *> IGMForSpecializations;

  // The IGM of the first source file.
  IRGenModule *PrimaryIGM = nullptr;

  // The current IGM for which IR is generated.
  IRGenModule *CurrentIGM = nullptr;

  /// If this is true, adding anything to the below queues is an error.
  bool FinishedEmittingLazyDefinitions = false;

  /// A map recording if metadata can be emitted lazily for each nominal type.
  toolchain::DenseMap<TypeDecl *, bool> HasLazyMetadata;

  struct LazyTypeGlobalsInfo {
    /// Is there a use of the type metadata?
    bool IsMetadataUsed = false;

    /// Is there a use of the nominal type descriptor?
    bool IsDescriptorUsed = false;

    /// Have we already emitted type metadata?
    bool IsMetadataEmitted = false;

    /// Have we already emitted a type context descriptor?
    bool IsDescriptorEmitted = false;
  };

  /// The set of type metadata that have been enqueued for lazy emission.
  toolchain::DenseMap<NominalTypeDecl *, LazyTypeGlobalsInfo> LazyTypeGlobals;

  /// The queue of lazy type metadata to emit.
  toolchain::SmallVector<NominalTypeDecl *, 4> LazyTypeMetadata;

  /// The queue of lazy type context descriptors to emit.
  toolchain::SmallVector<NominalTypeDecl *, 4> LazyTypeContextDescriptors;

  /// Field metadata records that have already been lazily emitted, or are
  /// queued up.
  toolchain::SmallPtrSet<NominalTypeDecl *, 4> LazilyEmittedFieldMetadata;

  /// Maps every generic type whose metadata is specialized within the module
  /// to its specializations.
  toolchain::DenseMap<
      NominalTypeDecl *,
      toolchain::SmallVector<std::pair<CanType, TypeMetadataCanonicality>, 4>>
      MetadataPrespecializationsForGenericTypes;

  toolchain::DenseMap<NominalTypeDecl *, toolchain::SmallVector<CanType, 4>>
      CanonicalSpecializedAccessorsForGenericTypes;

  /// The queue of specialized generic types whose prespecialized metadata to
  /// emit.
  toolchain::SmallVector<std::pair<CanType, TypeMetadataCanonicality>, 4>
      LazySpecializedTypeMetadataRecords;

  toolchain::SmallPtrSet<NominalTypeDecl *, 4> LazilyReemittedTypeContextDescriptors;

  /// The queue of metadata accessors to emit.
  ///
  /// The accessors must be emitted after everything else which might result in
  /// a statically-known-canonical prespecialization.
  toolchain::SmallSetVector<NominalTypeDecl *, 4> LazyMetadataAccessors;

  /// The queue of prespecialized metadata accessors to emit.
  toolchain::SmallSetVector<CanType, 4> LazyCanonicalSpecializedMetadataAccessors;

  struct LazyOpaqueInfo {
    bool IsDescriptorUsed = false;
    bool IsDescriptorEmitted = false;
  };
  /// The set of opaque types enqueued for lazy emission.
  toolchain::DenseMap<OpaqueTypeDecl*, LazyOpaqueInfo> LazyOpaqueTypes;
  /// The queue of opaque type descriptors to emit.
  toolchain::SmallVector<OpaqueTypeDecl*, 4> LazyOpaqueTypeDescriptors;
  /// The set of extension contenxts enqueued for lazy emission.
  toolchain::DenseMap<ExtensionDecl*, LazyOpaqueInfo> LazyExtensions;
  /// The queue of opaque type descriptors to emit.
  toolchain::TinyPtrVector<ExtensionDecl*> LazyExtensionDescriptors;
public:
  /// The set of eagerly emitted opaque types.
  toolchain::SmallPtrSet<OpaqueTypeDecl *, 4> EmittedNonLazyOpaqueTypeDecls;
private:

  /// The queue of lazy field metadata records to emit.
  toolchain::SmallVector<NominalTypeDecl *, 4> LazyFieldDescriptors;

  toolchain::SetVector<SILFunction *> DynamicReplacements;

  /// SIL functions that have already been lazily emitted, or are queued up.
  toolchain::SmallPtrSet<SILFunction *, 4> LazilyEmittedFunctions;

  /// The queue of SIL functions to emit.
  toolchain::SmallVector<SILFunction *, 4> LazyFunctionDefinitions;

  /// Witness tables that have already been lazily emitted, or are queued up.
  toolchain::SmallPtrSet<SILWitnessTable *, 4> LazilyEmittedWitnessTables;

  /// The queue of lazy witness tables to emit.
  toolchain::SmallVector<SILWitnessTable *, 4> LazyWitnessTables;

  toolchain::SmallVector<CanType, 4> LazyClassMetadata;

  toolchain::SmallPtrSet<TypeBase *, 4> LazilyEmittedClassMetadata;

  toolchain::SmallVector<CanType, 4> LazySpecializedClassMetadata;

  toolchain::SmallPtrSet<TypeBase *, 4> LazilyEmittedSpecializedClassMetadata;

  toolchain::SmallVector<ClassDecl *, 4> ClassesForEagerInitialization;

  toolchain::SmallVector<ClassDecl *, 4> ObjCActorsNeedingSuperclassSwizzle;

  /// The order in which all the SIL function definitions should
  /// appear in the translation unit.
  toolchain::DenseMap<SILFunction*, unsigned> FunctionOrder;

  /// The queue of IRGenModules for multi-threaded compilation.
  SmallVector<IRGenModule *, 8> Queue;
  
  std::atomic<int> QueueIndex;
  
  friend class CurrentIGMPtr;
public:
  explicit IRGenerator(const IRGenOptions &opts, SILModule &module);

  /// Attempt to create an toolchain::TargetMachine for the current target.
  std::unique_ptr<toolchain::TargetMachine> createTargetMachine();

  /// Add an IRGenModule for a source file.
  /// Should only be called from IRGenModule's constructor.
  void addGenModule(SourceFile *SF, IRGenModule *IGM);
  
  /// Get an IRGenModule for a source file.
  IRGenModule *getGenModule(SourceFile *SF);

  SourceFile *getSourceFile(IRGenModule *module) {
    for (auto pair : GenModules) {
      if (pair.second == module) {
        return pair.first;
      }
    }
    return nullptr;
  }

  /// Get an IRGenModule for a declaration context.
  /// Returns the IRGenModule of the containing source file, or if this cannot
  /// be determined, returns the primary IRGenModule.
  IRGenModule *getGenModule(DeclContext *ctxt);

  /// Get an IRGenModule for a function.
  /// Returns the IRGenModule of the containing source file, or if this cannot
  /// be determined, returns the IGM from which the function is referenced the
  /// first time.
  IRGenModule *getGenModule(SILFunction *f);

  /// Returns the primary IRGenModule. This is the first added IRGenModule.
  /// It is used for everything which cannot be correlated to a specific source
  /// file. And of course, in single-threaded compilation there is only the
  /// primary IRGenModule.
  IRGenModule *getPrimaryIGM() const {
    assert(PrimaryIGM);
    return PrimaryIGM;
  }
  
  bool hasMultipleIGMs() const { return GenModules.size() >= 2; }
  
  toolchain::DenseMap<SourceFile *, IRGenModule *>::iterator begin() {
    return GenModules.begin();
  }
  
  toolchain::DenseMap<SourceFile *, IRGenModule *>::iterator end() {
    return GenModules.end();
  }
  
  /// Emit functions, variables and tables which are needed anyway, e.g. because
  /// they are externally visible.
  void emitGlobalTopLevel(const std::vector<std::string> &LinkerDirectives);

  /// Emit references to each of the protocol descriptors defined in this
  /// IR module.
  void emitCodiraProtocols();

  /// Emit the protocol conformance records needed by each IR module.
  void emitProtocolConformances();

  /// Emit type metadata records for types without explicit protocol conformance.
  void emitTypeMetadataRecords();

  /// Emit type metadata records for functions that can be looked up by name at
  /// runtime.
  void emitAccessibleFunctions();

  /// Emit reflection metadata records for builtin and imported types referenced
  /// from this module.
  void emitBuiltinReflectionMetadata();

  /// Emit a symbol identifying the reflection metadata version.
  void emitReflectionMetadataVersion();

  void emitEagerClassInitialization();
  void emitObjCActorsNeedingSuperclassSwizzle();

  // Emit the code to replace dynamicReplacement(for:) functions.
  void emitDynamicReplacements();

  // Emit info that describes the entry point to the module, if it has one.
  void emitEntryPointInfo();

  /// Emit coverage mapping info.
  void emitCoverageMapping();

  /// Checks if metadata for this type can be emitted lazily. This is true for
  /// non-public types as well as imported types, except for classes and
  /// protocols which are always emitted eagerly.
  bool hasLazyMetadata(TypeDecl *type);

  /// Emit everything which is reachable from already emitted IR.
  void emitLazyDefinitions();

  void addLazyFunction(SILFunction *f);

  void addDynamicReplacement(SILFunction *f) { DynamicReplacements.insert(f); }

  void forceLocalEmitOfLazyFunction(SILFunction *f) {
    DefaultIGMForFunction[f] = CurrentIGM;
  }

  void ensureRelativeSymbolCollocation(SILWitnessTable &wt);

  void ensureRelativeSymbolCollocation(SILDefaultWitnessTable &wt);

  void ensureRelativeSymbolCollocation(SILDefaultOverrideTable &ot);

  toolchain::SmallVector<std::pair<CanType, TypeMetadataCanonicality>, 4>
  metadataPrespecializationsForType(NominalTypeDecl *type) {
    return MetadataPrespecializationsForGenericTypes.lookup(type);
  }

  void noteLazyReemissionOfNominalTypeDescriptor(NominalTypeDecl *decl) {
    assert(decl->isAvailableDuringLowering());
    LazilyReemittedTypeContextDescriptors.insert(decl);
  }

  bool isLazilyReemittingNominalTypeDescriptor(NominalTypeDecl *decl) {
    return LazilyReemittedTypeContextDescriptors.find(decl) !=
           std::end(LazilyReemittedTypeContextDescriptors);
  }

  void noteUseOfMetadataAccessor(NominalTypeDecl *decl) {
    assert(decl->isAvailableDuringLowering());
    if (LazyMetadataAccessors.count(decl) == 0) {
      LazyMetadataAccessors.insert(decl);
    }
  }

  void noteUseOfClassMetadata(CanType classType);
  void noteUseOfSpecializedClassMetadata(CanType classType);

  void noteUseOfTypeMetadata(NominalTypeDecl *type) {
    noteUseOfTypeGlobals(type, true, RequireMetadata);
  }

  void noteUseOfSpecializedGenericTypeMetadata(
      IRGenModule &IGM, CanType theType, TypeMetadataCanonicality canonicality);
  void noteUseOfCanonicalSpecializedMetadataAccessor(CanType forType);

  void noteUseOfTypeMetadata(CanType type) {
    type.visit([&](Type t) {
      if (auto *nominal = t->getAnyNominal())
        noteUseOfTypeMetadata(nominal);
    });
  }

  void noteUseOfTypeContextDescriptor(NominalTypeDecl *type,
                                      RequireMetadata_t requireMetadata) {
    noteUseOfTypeGlobals(type, false, requireMetadata);
  }
  
  void noteUseOfOpaqueTypeDescriptor(OpaqueTypeDecl *opaque);
  void noteUseOfExtensionDescriptor(ExtensionDecl *ext);
  void noteUseOfFieldDescriptor(NominalTypeDecl *type);

  void noteUseOfFieldDescriptors(CanType type) {
    type.visit([&](Type t) {
      if (auto *nominal = t->getAnyNominal())
        noteUseOfFieldDescriptor(nominal);
    });
  }

private:
  void noteUseOfTypeGlobals(NominalTypeDecl *type,
                            bool isUseOfMetadata,
                            RequireMetadata_t requireMetadata);
public:

  /// Return true if \p wt can be emitted lazily.
  bool canEmitWitnessTableLazily(SILWitnessTable *wt);

  /// Adds \p Conf to LazyWitnessTables if it has not been added yet.
  void addLazyWitnessTable(const ProtocolConformance *Conf);


  void addClassForEagerInitialization(ClassDecl *ClassDecl);
  void addBackDeployedObjCActorInitialization(ClassDecl *ClassDecl);

  unsigned getFunctionOrder(SILFunction *F) {
    auto it = FunctionOrder.find(F);
    assert(it != FunctionOrder.end() &&
           "no order number for SIL function definition?");
    return it->second;
  }
  
  /// In multi-threaded compilation fetch the next IRGenModule from the queue.
  IRGenModule *fetchFromQueue() {
    int idx = QueueIndex++;
    if (idx < (int)Queue.size()) {
      return Queue[idx];
    }
    return nullptr;
  }

  /// Return the effective triple used by clang.
  toolchain::Triple getEffectiveClangTriple();

  /// Return the effective variant triple used by clang.
  toolchain::Triple getEffectiveClangVariantTriple();

  const toolchain::StringRef getClangDataLayoutString();
};

class ConstantReference {
public:
  enum Directness : bool { Direct, Indirect };
private:
  toolchain::PointerIntPair<toolchain::Constant *, 1, Directness> ValueAndIsIndirect;
public:
  ConstantReference() {}
  ConstantReference(toolchain::Constant *value, Directness isIndirect)
    : ValueAndIsIndirect(value, isIndirect) {}

  Directness isIndirect() const { return ValueAndIsIndirect.getInt(); }
  toolchain::Constant *getValue() const { return ValueAndIsIndirect.getPointer(); }

  toolchain::Constant *getDirectValue() const {
    assert(!isIndirect());
    return getValue();
  }

  explicit operator bool() const {
    return ValueAndIsIndirect.getPointer() != nullptr;
  }
};

/// A reference to a declared type entity.
class TypeEntityReference {
  TypeReferenceKind Kind;
  toolchain::Constant *Value;
public:
  TypeEntityReference(TypeReferenceKind kind, toolchain::Constant *value)
    : Kind(kind), Value(value) {}

  TypeReferenceKind getKind() const { return Kind; }
  toolchain::Constant *getValue() const { return Value; }
};

/// Describes the role of a mangled type reference string.
enum class MangledTypeRefRole {
  /// The mangled type reference is used for field metadata, which is used
  /// by both in-process and out-of-process reflection, so still requires
  /// referenced types' metadata to be fully emitted.
  FieldMetadata,
  /// The mangled type reference is used for normal metadata.
  Metadata,
  /// The mangled type reference is used for reflection metadata.
  Reflection,
  /// The mangled type reference is used for a default associated type
  /// witness.
  DefaultAssociatedTypeWitness,
  /// The mangled type reference must be a flat string (i.e. no
  /// symbolic references) and unique for the target type.
  FlatUnique,
};


struct AccessibleFunction {
private:
  std::string RecordName;
  std::string FunctionName;

  bool IsDistributed: 1;

  CanSILFunctionType Type;
  toolchain::Constant *Address;

  explicit AccessibleFunction(std::string recordName, std::string funcName,
                              bool isDistributed, CanSILFunctionType type,
                              toolchain::Constant *addr)
      : RecordName(recordName), FunctionName(funcName),
        IsDistributed(isDistributed), Type(type), Address(addr) {}

public:
  StringRef getRecordName() const { return RecordName; }
  StringRef getFunctionName() const { return FunctionName; }

  bool isDistributed() const { return IsDistributed; }

  CanSILFunctionType getType() const { return Type; }

  toolchain::Constant *getAddress() const { return Address; }

  static AccessibleFunction forSILFunction(IRGenModule &IGM, SILFunction *fn);
  static AccessibleFunction forDistributed(std::string recordName,
                                           std::string accessorName,
                                           CanSILFunctionType type,
                                           toolchain::Constant *address);
};

/// IRGenModule - Primary class for emitting IR for global declarations.
/// 
class IRGenModule {
public:
  // The ABI version of the Codira data generated by this file.
  static const uint32_t languageVersion = 7;

  std::unique_ptr<toolchain::LLVMContext> LLVMContext;
  IRGenerator &IRGen;
  ASTContext &Context;
  std::unique_ptr<clang::CodeGenerator> ClangCodeGen;
  toolchain::Module &Module;
  const toolchain::DataLayout DataLayout;
  const toolchain::Triple Triple;
  const toolchain::Triple VariantTriple;
  std::unique_ptr<toolchain::TargetMachine> TargetMachine;
  ModuleDecl *getCodiraModule() const;
  AvailabilityRange getAvailabilityRange() const;
  Lowering::TypeConverter &getSILTypes() const;
  SILModule &getSILModule() const { return IRGen.SIL; }
  const IRGenOptions &getOptions() const { return IRGen.Opts; }
  SILModuleConventions silConv;
  ModuleDecl *ObjCModule = nullptr;
  ModuleDecl *ClangImporterModule = nullptr;
  std::unique_ptr<toolchain::raw_fd_ostream> RemarkStream;

  toolchain::StringMap<ModuleDecl*> OriginalModules;
  toolchain::SmallString<128> OutputFilename;
  toolchain::SmallString<128> MainInputFilenameForDebugInfo;

  /// Order dependency -- TargetInfo must be initialized after Opts.
  const CodiraTargetInfo TargetInfo;
  /// Holds lexical scope info, etc. Is a nullptr if we compile without -g.
  std::unique_ptr<IRGenDebugInfo> DebugInfo;

  /// A global variable which stores the hash of the module. Used for
  /// incremental compilation.
  toolchain::GlobalVariable *ModuleHash;

  TypeLayoutCache typeLayoutCache;

  /// Does the current target require Objective-C interoperation?
  bool ObjCInterop = true;

  /// Is the current target using the Darwin pre-stable ABI's class marker bit?
  bool UseDarwinPreStableABIBit = true;

  /// Should we add value names to local IR values?
  bool EnableValueNames = false;

  // Should `languageerror` attribute be explicitly added for the target ABI.
  bool ShouldUseCodiraError;

  toolchain::Type *VoidTy;                  /// void (usually {})
  toolchain::PointerType *PtrTy;            /// ptr
  toolchain::IntegerType *Int1Ty;           /// i1
  toolchain::IntegerType *Int8Ty;           /// i8
  toolchain::IntegerType *Int16Ty;          /// i16
  toolchain::IntegerType *Int32Ty;          /// i32
  toolchain::PointerType *Int32PtrTy;       /// i32 *
  toolchain::IntegerType *RelativeAddressTy;
  toolchain::PointerType *RelativeAddressPtrTy;
  toolchain::IntegerType *Int64Ty;          /// i64
  union {
    toolchain::IntegerType *SizeTy;         /// usually i32 or i64
    toolchain::IntegerType *IntPtrTy;
    toolchain::IntegerType *MetadataKindTy;
    toolchain::IntegerType *OnceTy;
    toolchain::IntegerType *FarRelativeAddressTy;
    toolchain::IntegerType *ProtocolDescriptorRefTy;
  };
  toolchain::IntegerType *ObjCBoolTy;       /// i8 or i1
  union {
    toolchain::PointerType *Int8PtrTy;      /// i8*
    toolchain::PointerType *WitnessTableTy;
    toolchain::PointerType *ObjCSELTy;
    toolchain::PointerType *FunctionPtrTy;
    toolchain::PointerType *CaptureDescriptorPtrTy;
  };
  union {
    toolchain::PointerType *Int8PtrPtrTy;   /// i8**
    toolchain::PointerType *WitnessTablePtrTy;
  };
  toolchain::StructType *RefCountedStructTy;/// %language.refcounted = type { ... }
  Size RefCountedStructSize;           /// sizeof(%language.refcounted)
  toolchain::PointerType *RefCountedPtrTy;  /// %language.refcounted*
#define CHECKED_REF_STORAGE(Name, ...)                                         \
  toolchain::StructType *Name##ReferenceStructTy;                                   \
  toolchain::PointerType *Name##ReferencePtrTy; /// %language. #name _reference*
#include "language/AST/ReferenceStorage.def"
  toolchain::Constant *RefCountedNull;      /// %language.refcounted* null
  toolchain::StructType *FunctionPairTy;    /// { i8*, %language.refcounted* }
  toolchain::StructType *NoEscapeFunctionPairTy;    /// { i8*, %language.opaque* }
  toolchain::FunctionType *DeallocatingDtorTy; /// void (%language.refcounted*)
  toolchain::StructType *TypeMetadataStructTy; /// %language.type = type { ... }
  toolchain::PointerType *TypeMetadataPtrTy;/// %language.type*
  toolchain::PointerType *TypeMetadataPtrPtrTy; /// %language.type**
  union {
    toolchain::StructType *TypeMetadataResponseTy;   /// { %language.type*, iSize }
    toolchain::StructType *TypeMetadataDependencyTy; /// { %language.type*, iSize }
  };
  toolchain::StructType *OffsetPairTy;      /// { iSize, iSize }
  toolchain::StructType *FullTypeLayoutTy;  /// %language.full_type_layout = { ... }
  toolchain::StructType *TypeLayoutTy;  /// %language.type_layout = { ... }
  toolchain::StructType *TupleTypeMetadataTy;     /// %language.tuple_type
  toolchain::PointerType *TupleTypeMetadataPtrTy; /// %language.tuple_type*
  toolchain::StructType *FullHeapMetadataStructTy; /// %language.full_heapmetadata = type { ... }
  toolchain::PointerType *FullHeapMetadataPtrTy;/// %language.full_heapmetadata*
  toolchain::StructType *FullBoxMetadataStructTy; /// %language.full_boxmetadata = type { ... }
  toolchain::PointerType *FullBoxMetadataPtrTy;/// %language.full_boxmetadata*
  toolchain::StructType *FullTypeMetadataStructTy; /// %language.full_type = type { ... }
  toolchain::StructType *FullExistentialTypeMetadataStructTy; /// %language.full_existential_type = type { ... }
  toolchain::PointerType *FullTypeMetadataPtrTy;/// %language.full_type*
  toolchain::StructType *FullForeignTypeMetadataStructTy; /// %language.full_foreign_type = type { ... }
  toolchain::StructType *ProtocolDescriptorStructTy; /// %language.protocol = type { ... }
  toolchain::PointerType *ProtocolDescriptorPtrTy; /// %language.protocol*
  toolchain::StructType *ProtocolRequirementStructTy; /// %language.protocol_requirement
  union {
    toolchain::PointerType *ObjCPtrTy;      /// %objc_object*
    toolchain::PointerType *UnknownRefCountedPtrTy;
  };
  toolchain::PointerType *BridgeObjectPtrTy;/// %language.bridge*
  toolchain::StructType *OpaqueTy;          /// %language.opaque
  toolchain::PointerType *OpaquePtrTy;      /// %language.opaque*
  toolchain::StructType *ObjCClassStructTy; /// %objc_class
  toolchain::PointerType *ObjCClassPtrTy;   /// %objc_class*
  toolchain::StructType *ObjCSuperStructTy; /// %objc_super
  toolchain::PointerType *ObjCSuperPtrTy;   /// %objc_super*
  toolchain::StructType *ObjCBlockStructTy; /// %objc_block
  toolchain::PointerType *ObjCBlockPtrTy;   /// %objc_block*
  toolchain::FunctionType *ObjCUpdateCallbackTy;
  toolchain::StructType *ObjCFullResilientClassStubTy;   /// %objc_full_class_stub
  toolchain::StructType *ObjCResilientClassStubTy;   /// %objc_class_stub
  toolchain::StructType *ProtocolRecordTy;
  toolchain::PointerType *ProtocolRecordPtrTy;
  toolchain::StructType *ProtocolConformanceDescriptorTy;
  toolchain::PointerType *ProtocolConformanceDescriptorPtrTy;
  toolchain::StructType *TypeContextDescriptorTy;
  toolchain::PointerType *TypeContextDescriptorPtrTy;
  toolchain::StructType *ClassContextDescriptorTy;
  toolchain::StructType *MethodDescriptorStructTy; /// %language.method_descriptor
  toolchain::StructType *MethodOverrideDescriptorStructTy; /// %language.method_override_descriptor
  toolchain::StructType *MethodDefaultOverrideDescriptorStructTy; /// %language.method_default_override_descriptor
  toolchain::StructType *TypeMetadataRecordTy;
  toolchain::PointerType *TypeMetadataRecordPtrTy;
  toolchain::StructType *FieldDescriptorTy;
  toolchain::PointerType *FieldDescriptorPtrTy;
  toolchain::PointerType *FieldDescriptorPtrPtrTy;
  toolchain::PointerType *ErrorPtrTy;       /// %language.error*
  toolchain::StructType *OpenedErrorTripleTy; /// { %language.opaque*, %language.type*, i8** }
  toolchain::PointerType *OpenedErrorTriplePtrTy; /// { %language.opaque*, %language.type*, i8** }*
  toolchain::PointerType *WitnessTablePtrPtrTy;   /// i8***
  toolchain::StructType *OpaqueTypeDescriptorTy;
  toolchain::PointerType *OpaqueTypeDescriptorPtrTy;
  toolchain::Type *FloatTy;
  toolchain::Type *DoubleTy;
  toolchain::StructType *DynamicReplacementsTy; // { i8**, i8* }
  toolchain::PointerType *DynamicReplacementsPtrTy;

  toolchain::StructType *DynamicReplacementLinkEntryTy; // %link_entry = { i8*, %link_entry*}
  toolchain::PointerType
      *DynamicReplacementLinkEntryPtrTy; // %link_entry*
  toolchain::StructType *DynamicReplacementKeyTy; // { i32, i32}

  toolchain::StructType *AccessibleFunctionRecordTy; // { i32*, i32*, i32*, i32}

  // clang-format off
  toolchain::StructType *AsyncFunctionPointerTy; // { i32, i32 }
  toolchain::StructType *CodiraContextTy;
  toolchain::StructType *CodiraTaskTy;
  toolchain::StructType *CodiraJobTy;
  toolchain::StructType *CodiraExecutorTy;
  toolchain::PointerType *AsyncFunctionPointerPtrTy;
  toolchain::PointerType *CodiraContextPtrTy;
  toolchain::PointerType *CodiraTaskPtrTy;
  toolchain::PointerType *CodiraAsyncLetPtrTy;
  toolchain::PointerType *CodiraTaskOptionRecordPtrTy;
  toolchain::PointerType *CodiraTaskGroupPtrTy;
  toolchain::StructType  *CodiraTaskOptionRecordTy;
  toolchain::StructType  *CodiraInitialSerialExecutorTaskOptionRecordTy;
  toolchain::StructType  *CodiraTaskGroupTaskOptionRecordTy;
  toolchain::StructType  *CodiraInitialTaskExecutorUnownedPreferenceTaskOptionRecordTy;
  toolchain::StructType  *CodiraInitialTaskExecutorOwnedPreferenceTaskOptionRecordTy;
  toolchain::StructType  *CodiraInitialTaskNameTaskOptionRecordTy;
  toolchain::StructType  *CodiraResultTypeInfoTaskOptionRecordTy;
  toolchain::PointerType *CodiraJobPtrTy;
  toolchain::IntegerType *ExecutorFirstTy;
  toolchain::IntegerType *ExecutorSecondTy;
  toolchain::FunctionType *TaskContinuationFunctionTy;
  toolchain::PointerType *TaskContinuationFunctionPtrTy;
  toolchain::StructType *AsyncTaskAndContextTy;
  toolchain::StructType *ContinuationAsyncContextTy;
  toolchain::PointerType *ContinuationAsyncContextPtrTy;
  toolchain::StructType *ClassMetadataBaseOffsetTy;
  toolchain::StructType *DifferentiabilityWitnessTy; // { i8*, i8* }
  // clang-format on

  toolchain::StructType *CoroFunctionPointerTy; // { i32, i32 }
  toolchain::PointerType *CoroFunctionPointerPtrTy;
  toolchain::PointerType *CoroAllocationTy;
  toolchain::FunctionType *CoroAllocateFnTy;
  toolchain::FunctionType *CoroDeallocateFnTy;
  toolchain::IntegerType *CoroAllocatorFlagsTy;
  toolchain::StructType *CoroAllocatorTy;
  toolchain::PointerType *CoroAllocatorPtrTy;

  toolchain::GlobalVariable *TheTrivialPropertyDescriptor = nullptr;

  toolchain::Constant *languageImmortalRefCount = nullptr;
  toolchain::Constant *languageStaticArrayMetadata = nullptr;

  toolchain::StructType *
  createTransientStructType(StringRef name,
                            std::initializer_list<toolchain::Type *> types,
                            bool packed = false);

  /// Used to create unique names for class layout types with tail allocated
  /// elements.
  unsigned TailElemTypeID = 0;

  unsigned InvariantMetadataID; /// !invariant.load
  unsigned DereferenceableID;   /// !dereferenceable
  toolchain::MDNode *InvariantNode;

#ifdef CHECK_RUNTIME_EFFECT_ANALYSIS
  RuntimeEffect effectOfRuntimeFuncs = RuntimeEffect::NoEffect;
  SmallVector<const char *> emittedRuntimeFuncs;

  void registerRuntimeEffect(ArrayRef<RuntimeEffect> realtime,
                             const char *funcName);
#endif

  toolchain::CallingConv::ID C_CC;          /// standard C calling convention
  toolchain::CallingConv::ID DefaultCC;     /// default calling convention
  toolchain::CallingConv::ID CodiraCC;       /// language calling convention
  toolchain::CallingConv::ID CodiraAsyncCC;  /// language calling convention for async
  toolchain::CallingConv::ID CodiraCoroCC;   /// language calling convention for callee-allocated coroutines

  /// What kind of tail call should be used for async->async calls.
  toolchain::CallInst::TailCallKind AsyncTailCallKind;

  Signature getAssociatedTypeWitnessTableAccessFunctionSignature();

  /// Get the bit width of an integer type for the target platform.
  unsigned getBuiltinIntegerWidth(BuiltinIntegerType *t);
  unsigned getBuiltinIntegerWidth(BuiltinIntegerWidth w);

  Size getPointerSize() const { return PtrSize; }
  Alignment getPointerAlignment() const {
    // We always use the pointer's width as its language ABI alignment.
    return Alignment(PtrSize.getValue());
  }
  Alignment getWitnessTableAlignment() const {
    return getPointerAlignment();
  }
  Alignment getTypeMetadataAlignment() const {
    return getPointerAlignment();
  }
  Alignment getAsyncContextAlignment() const;
  Alignment getCoroStaticFrameAlignment() const;

  /// Return the offset, relative to the address point, of the start of the
  /// type-specific members of an enum metadata.
  Size getOffsetOfEnumTypeSpecificMetadataMembers() {
    return getPointerSize() * 2;
  }

  /// Return the offset, relative to the address point, of the start of the
  /// type-specific members of a struct metadata.
  Size getOffsetOfStructTypeSpecificMetadataMembers() {
    return getPointerSize() * 2;
  }

  Size::int_type getOffsetInWords(Size offset) {
    assert(offset.isMultipleOf(getPointerSize()));
    return offset / getPointerSize();
  }

  toolchain::Type *getReferenceType(ReferenceCounting style);

  static bool isLoadableReferenceAddressOnly(ReferenceCounting style) {
    switch (style) {
    case ReferenceCounting::Native:
      return false;

    case ReferenceCounting::Unknown:
    case ReferenceCounting::ObjC:
    case ReferenceCounting::Block:
    case ReferenceCounting::None:
    case ReferenceCounting::Custom:
      return true;

    case ReferenceCounting::Bridge:
    case ReferenceCounting::Error:
      toolchain_unreachable("loadable references to this type are not supported");
    }

    toolchain_unreachable("Not a valid ReferenceCounting.");
  }
  
  /// Return the spare bit mask to use for types that comprise heap object
  /// pointers.
  const SpareBitVector &getHeapObjectSpareBits() const;

  const SpareBitVector &getFunctionPointerSpareBits() const;
  const SpareBitVector &getWitnessTablePtrSpareBits() const;

  /// Return runtime specific extra inhabitant and spare bits policies.
  unsigned getReferenceStorageExtraInhabitantCount(ReferenceOwnership ownership,
                                                 ReferenceCounting style) const;
  SpareBitVector getReferenceStorageSpareBits(ReferenceOwnership ownership,
                                              ReferenceCounting style) const;
  APInt getReferenceStorageExtraInhabitantValue(unsigned bits, unsigned index,
                                                ReferenceOwnership ownership,
                                                ReferenceCounting style) const;
  APInt getReferenceStorageExtraInhabitantMask(ReferenceOwnership ownership,
                                               ReferenceCounting style) const;

  toolchain::Type *getFixedBufferTy();
  toolchain::PointerType *getExistentialPtrTy(unsigned numTables);
  toolchain::Type *getExistentialType(unsigned numTables);
  toolchain::Type *getValueWitnessTy(ValueWitness index);
  Signature getValueWitnessSignature(ValueWitness index);

  toolchain::StructType *getIntegerLiteralTy();

  toolchain::StructType *getValueWitnessTableTy();
  toolchain::StructType *getEnumValueWitnessTableTy();
  toolchain::PointerType *getValueWitnessTablePtrTy();
  toolchain::PointerType *getEnumValueWitnessTablePtrTy();

  toolchain::IntegerType *getTypeMetadataRequestParamTy();
  toolchain::StructType *getTypeMetadataResponseTy();

  void unimplemented(SourceLoc, StringRef Message);
  [[noreturn]]
  void fatal_unimplemented(SourceLoc, StringRef Message);
  void error(SourceLoc loc, const Twine &message);

  bool shouldPrespecializeGenericMetadata();
  
  bool canMakeStaticObjectReadOnly(SILType objectType);

  ClassDecl *getStaticArrayStorageDecl();

#define FEATURE(N, V)                                           \
  bool is##N##FeatureAvailable(const ASTContext &context);      \
  inline bool is##N##FeatureAvailable() {                       \
    return is##N##FeatureAvailable(Context);                    \
  }
  #include "language/AST/FeatureAvailability.def"

  bool canUseObjCSymbolicReferences();

  Size getAtomicBoolSize() const { return AtomicBoolSize; }
  Alignment getAtomicBoolAlignment() const { return AtomicBoolAlign; }

  enum class ObjCLabelType {
    ClassName,
    MethodVarName,
    MethodVarType,
    PropertyName,
  };

  std::string GetObjCSectionName(StringRef Section, StringRef MachOAttributes);
  void SetCStringLiteralSection(toolchain::GlobalVariable *GV, ObjCLabelType Type);

  void addAsyncCoroIDMapping(toolchain::GlobalVariable *asyncFunctionPointer,
                             toolchain::CallInst *coro_id_builtin);
  
  toolchain::CallInst *getAsyncCoroIDMapping(
                                    toolchain::GlobalVariable *asyncFunctionPointer);
  
  void markAsyncFunctionPointerForPadding(
                                    toolchain::GlobalVariable *asyncFunctionPointer);
  
  bool isAsyncFunctionPointerMarkedForPadding(
                                    toolchain::GlobalVariable *asyncFunctionPointer);
  
private:
  Size PtrSize;
  Size AtomicBoolSize;
  Alignment AtomicBoolAlign;
  toolchain::Type *FixedBufferTy;          /// [N x i8], where N == 3 * sizeof(void*)

  toolchain::Type *ValueWitnessTys[MaxNumValueWitnesses];
  toolchain::FunctionType *AssociatedTypeWitnessTableAccessFunctionTy = nullptr;
  toolchain::StructType *GenericWitnessTableCacheTy = nullptr;
  toolchain::StructType *IntegerLiteralTy = nullptr;
  toolchain::StructType *ValueWitnessTableTy = nullptr;
  toolchain::StructType *EnumValueWitnessTableTy = nullptr;

  toolchain::DenseMap<toolchain::Type *, SpareBitVector> SpareBitsForTypes;

  // Mapping of AsyncFunctionPointer records to their corresponding
  // `@toolchain.coro.id.async` intrinsic tag in the function implementation.
  // This is used for a runtime bug workaround where we need to pad the initial
  // context size for tasks used as `async let` entry points.
  //
  // An entry in the map may have a null value, to indicate that a not-yet-
  // emitted async function pointer should get the padding applied when it is
  // emitted.
  toolchain::DenseMap<toolchain::GlobalVariable*, toolchain::CallInst*> AsyncCoroIDsForPadding;

  /// The personality function used for foreign exception handling in this
  /// module.
  toolchain::Function *foreignExceptionHandlingPersonalityFunc = nullptr;

public:
  /// The set of emitted foreign function thunks that trap on exception in the
  /// underlying call that the thunk dispatches.
  toolchain::SmallPtrSet<toolchain::Function *, 4>
      emittedForeignFunctionThunksWithExceptionTraps;

//--- Types -----------------------------------------------------------------
  const ProtocolInfo &getProtocolInfo(ProtocolDecl *D, ProtocolInfoKind kind);

  // Not strictly a type operation, but similar.
  const ConformanceInfo &
  getConformanceInfo(const ProtocolDecl *protocol,
                     const ProtocolConformance *conformance);

  SILType getLoweredType(AbstractionPattern orig, Type subst) const;
  SILType getLoweredType(Type subst) const;
  const Lowering::TypeLowering &getTypeLowering(SILType type) const;
  bool isTypeABIAccessible(SILType type) const;

  const TypeInfo &getTypeInfoForUnlowered(AbstractionPattern orig,
                                          CanType subst);
  const TypeInfo &getTypeInfoForUnlowered(AbstractionPattern orig,
                                          Type subst);
  const TypeInfo &getTypeInfoForUnlowered(Type subst);
  const TypeInfo &getTypeInfoForLowered(CanType T);
  const TypeInfo &getTypeInfo(SILType T);
  const TypeInfo &getWitnessTablePtrTypeInfo();
  const TypeInfo &getTypeMetadataPtrTypeInfo();
  const TypeInfo &getCodiraContextPtrTypeInfo();
  const TypeInfo &getTaskContinuationFunctionPtrTypeInfo();
  const LoadableTypeInfo &getExecutorTypeInfo();
  const TypeInfo &getObjCClassPtrTypeInfo();
  const LoadableTypeInfo &getOpaqueStorageTypeInfo(Size size, Alignment align);
  const LoadableTypeInfo &
  getReferenceObjectTypeInfo(ReferenceCounting refcounting);
  const LoadableTypeInfo &getNativeObjectTypeInfo();
  const LoadableTypeInfo &getUnknownObjectTypeInfo();
  const LoadableTypeInfo &getBridgeObjectTypeInfo();
  const LoadableTypeInfo &getRawPointerTypeInfo();
  const LoadableTypeInfo &getRawUnsafeContinuationTypeInfo();
  toolchain::Type *getStorageTypeForUnlowered(Type T);
  toolchain::Type *getStorageTypeForLowered(CanType T);
  toolchain::Type *getStorageType(SILType T);
  toolchain::PointerType *getStoragePointerTypeForUnlowered(Type T);
  toolchain::PointerType *getStoragePointerTypeForLowered(CanType T);
  toolchain::PointerType *getStoragePointerType(SILType T);
  toolchain::StructType *createNominalType(CanType type);
  toolchain::StructType *createNominalType(ProtocolCompositionType *T);
  clang::CanQual<clang::Type> getClangType(CanType type);
  clang::CanQual<clang::Type> getClangType(SILType type);
  clang::CanQual<clang::Type> getClangType(SILParameterInfo param,
                                           CanSILFunctionType funcTy);

  const TypeLayoutEntry
  &getTypeLayoutEntry(SILType T, bool useStructLayouts);

  const clang::ASTContext &getClangASTContext() {
    assert(ClangASTContext &&
           "requesting clang AST context without clang importer!");
    return *ClangASTContext;
  }

  clang::CodeGen::CodeGenModule &getClangCGM() const;
  
  CanType getRuntimeReifiedType(CanType type);
  Type substOpaqueTypesWithUnderlyingTypes(Type type);
  CanType substOpaqueTypesWithUnderlyingTypes(CanType type);
  SILType substOpaqueTypesWithUnderlyingTypes(SILType type, CanGenericSignature genericSig);
  std::pair<CanType, ProtocolConformanceRef>
  substOpaqueTypesWithUnderlyingTypes(CanType type,
                                      ProtocolConformanceRef conformance);

  bool isResilient(NominalTypeDecl *decl, ResilienceExpansion expansion,
                   ClassDecl *asViewedFromRootClass = nullptr);
  bool hasResilientMetadata(ClassDecl *decl, ResilienceExpansion expansion,
                            ClassDecl *asViewedFromRootClass = nullptr);
  ResilienceExpansion getResilienceExpansionForAccess(NominalTypeDecl *decl);
  ResilienceExpansion getResilienceExpansionForLayout(NominalTypeDecl *decl);
  ResilienceExpansion getResilienceExpansionForLayout(SILGlobalVariable *var);

  TypeExpansionContext getMaximalTypeExpansionContext() const;

  bool isResilientConformance(const NormalProtocolConformance *conformance,
                              bool disableOptimizations = false);
  bool isResilientConformance(const RootProtocolConformance *root,
                              bool disableOptimizations = false);
  bool isDependentConformance(const RootProtocolConformance *conformance);

  Alignment getCappedAlignment(Alignment alignment);

  SpareBitVector getSpareBitsForType(toolchain::Type *scalarTy, Size size);

  MetadataLayout &getMetadataLayout(NominalTypeDecl *decl);
  NominalMetadataLayout &getNominalMetadataLayout(NominalTypeDecl *decl);
  StructMetadataLayout &getMetadataLayout(StructDecl *decl);
  ClassMetadataLayout &getClassMetadataLayout(ClassDecl *decl);
  EnumMetadataLayout &getMetadataLayout(EnumDecl *decl);
  ForeignClassMetadataLayout &getForeignMetadataLayout(ClassDecl *decl);

  ClassMetadataStrategy getClassMetadataStrategy(const ClassDecl *theClass);

  bool IsWellKnownBuiltinOrStructralType(CanType type) const;

private:
  TypeConverter &Types;
  friend TypeConverter;

  const clang::ASTContext *ClangASTContext;

  toolchain::DenseMap<Decl*, MetadataLayout*> MetadataLayouts;
  void destroyMetadataLayoutMap();

  toolchain::DenseMap<const ProtocolConformance *,
                 std::unique_ptr<const ConformanceInfo>> Conformances;

  friend class GenericContextScope;
  friend class LoweringModeScope;
  
//--- Globals ---------------------------------------------------------------
public:
  std::pair<toolchain::GlobalVariable *, toolchain::Constant *> createStringConstant(
      StringRef Str, bool willBeRelativelyAddressed = false,
      StringRef sectionName = "", StringRef name = "");
  toolchain::Constant *getAddrOfGlobalString(StringRef utf8,
                                        bool willBeRelativelyAddressed = false,
                                        bool useOSLogSection = false);
  toolchain::Constant *getAddrOfGlobalUTF16String(StringRef utf8);
  toolchain::Constant *
  getAddrOfGlobalIdentifierString(StringRef utf8,
                                  bool willBeRelativelyAddressed = false);
  toolchain::Constant *getAddrOfObjCSelectorRef(StringRef selector);
  toolchain::Constant *getAddrOfObjCSelectorRef(SILDeclRef method);
  std::string getObjCSelectorName(SILDeclRef method);
  toolchain::Constant *getAddrOfObjCMethodName(StringRef methodName);
  toolchain::Constant *getAddrOfObjCProtocolRecord(ProtocolDecl *proto,
                                              ForDefinition_t forDefinition);
  Address getAddrOfObjCProtocolRef(ProtocolDecl *proto,
                                   ForDefinition_t forDefinition);
  toolchain::Constant *getAddrOfKeyPathPattern(KeyPathPattern *pattern,
                                          SILLocation diagLoc);
  toolchain::Constant *getAddrOfOpaqueTypeDescriptor(OpaqueTypeDecl *opaqueType,
                                                ConstantInit forDefinition);
  toolchain::Constant *
  emitClangProtocolObject(const clang::ObjCProtocolDecl *objcProtocol);

  ConstantReference getConstantReferenceForProtocolDescriptor(ProtocolDecl *proto);

  ConstantIntegerLiteral getConstantIntegerLiteral(APInt value);

  toolchain::Constant *getOrCreateLazyGlobalVariable(LinkEntity entity,
      toolchain::function_ref<ConstantInitFuture(ConstantInitBuilder &)> build,
      toolchain::function_ref<void(toolchain::GlobalVariable *)> finish);

  void addUsedGlobal(toolchain::GlobalValue *global);
  void addCompilerUsedGlobal(toolchain::GlobalValue *global);
  void addGenericROData(toolchain::Constant *addr);
  void addObjCClass(toolchain::Constant *addr, bool nonlazy);
  void addObjCClassStub(toolchain::Constant *addr);
  void addProtocolConformance(ConformanceDescription &&conformance);
  void addAccessibleFunction(AccessibleFunction fn);

  toolchain::Constant *emitCodiraProtocols(bool asContiguousArray);
  toolchain::Constant *emitProtocolConformances(bool asContiguousArray);
  toolchain::Constant *emitTypeMetadataRecords(bool asContiguousArray);

  void emitAccessibleFunctions();
  void emitAccessibleFunction(StringRef sectionName,
                              const AccessibleFunction &fn);

  toolchain::Constant *getConstantSignedFunctionPointer(toolchain::Constant *fn,
                                                   CanSILFunctionType fnType);

  toolchain::Constant *getConstantSignedCFunctionPointer(toolchain::Constant *fn);

  toolchain::Constant *
  getConstantSignedPointer(toolchain::Constant *pointer, unsigned key,
                           toolchain::Constant *storageAddress,
                           toolchain::ConstantInt *otherDiscriminator);
  toolchain::Constant *getConstantSignedPointer(toolchain::Constant *pointer,
                                           const clang::PointerAuthSchema &schema,
                                           const PointerAuthEntity &entity,
                                           toolchain::Constant *storageAddress);

  toolchain::Constant *getOrCreateHelperFunction(
      StringRef name, toolchain::Type *resultType, ArrayRef<toolchain::Type *> paramTypes,
      toolchain::function_ref<void(IRGenFunction &IGF)> generate,
      bool setIsNoInline = false, bool forPrologue = false,
      bool isPerformanceConstraint = false,
      IRLinkage *optionalLinkage = nullptr,
      std::optional<toolchain::CallingConv::ID> specialCallingConv = std::nullopt,
      std::optional<toolchain::function_ref<void(toolchain::AttributeList &)>>
          transformAttrs = std::nullopt);

  toolchain::Constant *getOrCreateRetainFunction(const TypeInfo &objectTI, SILType t,
                              toolchain::Type *toolchainType, Atomicity atomicity);

  toolchain::Constant *
  getOrCreateReleaseFunction(const TypeInfo &objectTI, SILType t,
                             toolchain::Type *toolchainType, Atomicity atomicity,
                             const OutliningMetadataCollector &collector);

  toolchain::Constant *getOrCreateOutlinedInitializeWithTakeFunction(
                              SILType objectType, const TypeInfo &objectTI,
                              const OutliningMetadataCollector &collector);

  toolchain::Constant *getOrCreateOutlinedInitializeWithCopyFunction(
                              SILType objectType, const TypeInfo &objectTI,
                              const OutliningMetadataCollector &collector);

  toolchain::Constant *getOrCreateOutlinedAssignWithTakeFunction(
                              SILType objectType, const TypeInfo &objectTI,
                              const OutliningMetadataCollector &collector);

  toolchain::Constant *getOrCreateOutlinedAssignWithCopyFunction(
                              SILType objectType, const TypeInfo &objectTI,
                              const OutliningMetadataCollector &collector);

  toolchain::Constant *getOrCreateOutlinedDestroyFunction(
                              SILType objectType, const TypeInfo &objectTI,
                              const OutliningMetadataCollector &collector);

  toolchain::Constant *getOrCreateOutlinedEnumTagStoreFunction(
    SILType T, const TypeInfo &ti, EnumElementDecl *theCase, unsigned caseIdx);
  toolchain::Constant *getOrCreateOutlinedDestructiveProjectDataForLoad(
    SILType T, const TypeInfo &ti, EnumElementDecl *theCase, unsigned caseIdx);

  toolchain::Constant *getAddrOfClangGlobalDecl(clang::GlobalDecl global,
                                           ForDefinition_t forDefinition);

private:
  using CopyAddrHelperGenerator =
    toolchain::function_ref<void(IRGenFunction &IGF, Address dest, Address src,
                            SILType objectType, const TypeInfo &objectTI)>;

  toolchain::Constant *getOrCreateOutlinedCopyAddrHelperFunction(
                              SILType objectType, const TypeInfo &objectTI,
                              const OutliningMetadataCollector &collector,
                              StringRef funcName,
                              CopyAddrHelperGenerator generator);

  toolchain::Constant *getOrCreateGOTEquivalent(toolchain::Constant *global,
                                           LinkEntity entity);
                                           
  toolchain::DenseMap<LinkEntity, toolchain::Constant*> GlobalVars;
  toolchain::DenseMap<LinkEntity, toolchain::Constant*> IndirectAsyncFunctionPointers;
  toolchain::DenseMap<LinkEntity, toolchain::Constant *> IndirectCoroFunctionPointers;
  toolchain::DenseMap<LinkEntity, toolchain::Constant*> GlobalGOTEquivalents;
  toolchain::DenseMap<LinkEntity, toolchain::Function*> GlobalFuncs;
  toolchain::DenseSet<const clang::Decl *> GlobalClangDecls;
  toolchain::StringMap<std::pair<toolchain::GlobalVariable*, toolchain::Constant*>>
    GlobalStrings;
  toolchain::StringMap<std::pair<toolchain::GlobalVariable*, toolchain::Constant*>>
    GlobalOSLogStrings;
  toolchain::StringMap<toolchain::Constant*> GlobalUTF16Strings;
  toolchain::StringMap<std::pair<toolchain::GlobalVariable*, toolchain::Constant*>>
    StringsForTypeRef;
  toolchain::StringMap<std::pair<toolchain::GlobalVariable*, toolchain::Constant*>> FieldNames;
  toolchain::StringMap<toolchain::Constant*> ObjCSelectorRefs;
  toolchain::StringMap<toolchain::Constant*> ObjCMethodNames;

  std::unique_ptr<ConstantIntegerLiteralMap> ConstantIntegerLiterals;

  /// Maps to constant language 'String's.
  toolchain::StringMap<toolchain::Constant*> GlobalConstantStrings;
  toolchain::StringMap<toolchain::Constant*> GlobalConstantUTF16Strings;

  struct PointerAuthCachesType;
  PointerAuthCachesType *PointerAuthCaches = nullptr;
  PointerAuthCachesType &getPointerAuthCaches();
  void destroyPointerAuthCaches();
  friend class PointerAuthEntity;

  /// LLVMUsed - List of global values which are required to be
  /// present in the object file; bitcast to i8*. This is used for
  /// forcing visibility of symbols which may otherwise be optimized
  /// out.
  SmallVector<toolchain::WeakTrackingVH, 4> LLVMUsed;

  /// LLVMCompilerUsed - List of global values which are required to be
  /// present in the object file; bitcast to i8*. This is used for
  /// forcing visibility of symbols which may otherwise be optimized
  /// out.
  ///
  /// Similar to LLVMUsed, but emitted as toolchain.compiler.used.
  SmallVector<toolchain::WeakTrackingVH, 4> LLVMCompilerUsed;

  /// Metadata nodes for autolinking info.
  SmallVector<LinkLibrary, 32> AutolinkEntries;

  // List of ro_data_t referenced from generic class patterns.
  SmallVector<toolchain::WeakTrackingVH, 4> GenericRODatas;
  /// List of Objective-C classes, bitcast to i8*.
  SmallVector<toolchain::WeakTrackingVH, 4> ObjCClasses;
  /// List of Objective-C classes that require nonlazy realization, bitcast to
  /// i8*.
  SmallVector<toolchain::WeakTrackingVH, 4> ObjCNonLazyClasses;
  /// List of Objective-C resilient class stubs, bitcast to i8*.
  SmallVector<toolchain::WeakTrackingVH, 4> ObjCClassStubs;
  /// List of Objective-C categories, bitcast to i8*.
  SmallVector<toolchain::WeakTrackingVH, 4> ObjCCategories;
  /// List of Objective-C categories on class stubs, bitcast to i8*.
  SmallVector<toolchain::WeakTrackingVH, 4> ObjCCategoriesOnStubs;
  /// List of non-ObjC protocols described by this module.
  SmallVector<ProtocolDecl *, 4> CodiraProtocols;
  /// List of protocol conformances to generate descriptors for.
  std::vector<ConformanceDescription> ProtocolConformances;
  /// List of types to generate runtime-resolvable metadata records for that
  /// are consumable for any Codira runtime.
  SmallVector<GenericTypeDecl *, 4> RuntimeResolvableTypes;
  /// List of types to generate runtime-resolvable metadata records for that
  /// are consumable for any Codira runtime aware of noncopyable types.
  SmallVector<GenericTypeDecl *, 4> RuntimeResolvableTypes2;
  /// List of ExtensionDecls corresponding to the generated
  /// categories.
  SmallVector<ExtensionDecl*, 4> ObjCCategoryDecls;

  /// List of all of the functions, which can be lookup by name
  /// up at runtime.
  SmallVector<AccessibleFunction, 4> AccessibleFunctions;

  /// Map of Objective-C protocols and protocol references, bitcast to i8*.
  /// The interesting global variables relating to an ObjC protocol.
  struct ObjCProtocolPair {
    /// The global variable that contains the protocol record.
    toolchain::WeakTrackingVH record;
    /// The global variable that contains the indirect reference to the
    /// protocol record.
    toolchain::WeakTrackingVH ref;
  };

  toolchain::DenseMap<ProtocolDecl*, ObjCProtocolPair> ObjCProtocols;
  toolchain::DenseMap<ProtocolDecl *, toolchain::Constant *> ObjCProtocolSymRefs;
  toolchain::SmallVector<ProtocolDecl*, 4> LazyObjCProtocolDefinitions;
  toolchain::DenseMap<KeyPathPattern*, toolchain::GlobalVariable*> KeyPathPatterns;

  /// Uniquing key for a fixed type layout record.
  struct FixedLayoutKey {
    unsigned size;
    unsigned numExtraInhabitants;
    unsigned align: 16;
    unsigned pod: 1;
    unsigned bitwiseTakable: 2;
  };
  friend struct ::toolchain::DenseMapInfo<language::irgen::IRGenModule::FixedLayoutKey>;
  toolchain::DenseMap<FixedLayoutKey, toolchain::Constant *> PrivateFixedLayouts;

  /// A cache for layouts of statically initialized objects.
  toolchain::DenseMap<SILGlobalVariable *, std::unique_ptr<StructLayout>>
    StaticObjectLayouts;

  /// A mapping from order numbers to the LLVM functions which we
  /// created for the SIL functions with those orders.
  SuccessorMap<unsigned, toolchain::Function*> EmittedFunctionsByOrder;

  ObjCProtocolPair getObjCProtocolGlobalVars(ProtocolDecl *proto);
  toolchain::Constant *getObjCProtocolRefSymRefDescriptor(ProtocolDecl *protocol);
  void emitLazyObjCProtocolDefinitions();
  void emitLazyObjCProtocolDefinition(ProtocolDecl *proto);

  SmallVector<toolchain::MDNode *> UsedConditionals;
  void emitUsedConditionals();

  void emitGlobalLists();
  void emitAutolinkInfo();
  void cleanupClangCodeGenMetadata();

//--- Remote reflection metadata --------------------------------------------
public:
  /// Section names.
  std::string FieldTypeSection;
  std::string BuiltinTypeSection;
  std::string AssociatedTypeSection;
  std::string CaptureDescriptorSection;
  std::string ReflectionStringsSection;
  std::string ReflectionTypeRefSection;
  std::string MultiPayloadEnumDescriptorSection;

  /// Builtin types referenced by types in this module when emitting
  /// reflection metadata.
  toolchain::SetVector<CanType> BuiltinTypes;

  std::pair<toolchain::Constant *, unsigned>
  getTypeRef(Type type, GenericSignature genericSig, MangledTypeRefRole role);
  
  std::pair<toolchain::Constant *, unsigned>
  getTypeRef(CanType type, CanGenericSignature sig, MangledTypeRefRole role);

  std::pair<toolchain::Constant *, unsigned>
  getLoweredTypeRef(SILType loweredType, CanGenericSignature genericSig,
                    MangledTypeRefRole role);

  toolchain::Constant *emitWitnessTableRefString(CanType type,
                                            ProtocolConformanceRef conformance,
                                            GenericSignature genericSig,
                                            bool shouldSetLowBit);
  toolchain::Constant *getMangledAssociatedConformance(
                                  const NormalProtocolConformance *conformance,
                                  const AssociatedConformance &requirement);
  toolchain::Constant *getAddrOfStringForTypeRef(StringRef mangling,
                                            MangledTypeRefRole role);
  toolchain::Constant *getAddrOfStringForTypeRef(const SymbolicMangling &mangling,
                                            MangledTypeRefRole role);

  /// Retrieve the address of a mangled string used for some kind of metadata
  /// reference.
  ///
  /// \param symbolName The name of the symbol that describes the metadata
  /// being referenced.
  /// \param alignment If non-zero, the alignment of the requested variable.
  /// \param shouldSetLowBit Whether to set the low bit of the result
  /// constant, which is used by some clients to indicate that the result is
  /// a mangled name.
  /// \param body The body of a function that will create the metadata value
  /// itself, given a constant building and producing a future for the
  /// initializer.
  /// \returns the address of the global variable describing this metadata.
  toolchain::Constant *getAddrOfStringForMetadataRef(
      StringRef symbolName,
      unsigned alignment,
      bool shouldSetLowBit,
      toolchain::function_ref<ConstantInitFuture(ConstantInitBuilder &)> body);

  toolchain::Constant *getAddrOfFieldName(StringRef Name);
  toolchain::Constant *getAddrOfCaptureDescriptor(SILFunction &caller,
                                             CanSILFunctionType origCalleeType,
                                             CanSILFunctionType substCalleeType,
                                             SubstitutionMap subs,
                                             const HeapLayout &layout);
  toolchain::Constant *getAddrOfBoxDescriptor(SILType boxedType,
                                         CanGenericSignature genericSig);

  /// Produce an associated type witness that refers to the given type.
  toolchain::Constant *getAssociatedTypeWitness(Type type, GenericSignature sig,
                                           bool inProtocolContext);

  void emitAssociatedTypeMetadataRecord(const RootProtocolConformance *C);
  void emitFieldDescriptor(const NominalTypeDecl *Decl);

  /// Emit a reflection metadata record for a builtin type referenced
  /// from this module.
  void emitBuiltinTypeMetadataRecord(CanType builtinType);

  /// Emit reflection metadata records for builtin and imported types referenced
  /// from this module.
  void emitBuiltinReflectionMetadata();

  /// Emit a symbol identifying the reflection metadata version.
  void emitReflectionMetadataVersion();

  const char *getBuiltinTypeMetadataSectionName();
  const char *getFieldTypeMetadataSectionName();
  const char *getAssociatedTypeMetadataSectionName();
  const char *getCaptureDescriptorMetadataSectionName();
  const char *getReflectionStringsSectionName();
  const char *getReflectionTypeRefSectionName();
  const char *getMultiPayloadEnumDescriptorSectionName();

  /// Returns the special builtin types that should be emitted in the stdlib
  /// module.
  toolchain::ArrayRef<CanType> getOrCreateSpecialStlibBuiltinTypes();

private:
  /// The special builtin types that should be emitted in the stdlib module.
  toolchain::SmallVector<CanType, 7> SpecialStdlibBuiltinTypes;

//--- Runtime ---------------------------------------------------------------
public:
  toolchain::Constant *getEmptyTupleMetadata();
  toolchain::Constant *getAnyExistentialMetadata();
  toolchain::Constant *getAnyObjectExistentialMetadata();
  toolchain::Constant *getObjCEmptyCachePtr();
  toolchain::Constant *getObjCEmptyVTablePtr();
  toolchain::InlineAsm *getObjCRetainAutoreleasedReturnValueMarker();
  ClassDecl *getObjCRuntimeBaseForCodiraRootClass(ClassDecl *theClass);
  ClassDecl *getObjCRuntimeBaseClass(Identifier name, Identifier objcName);
  ClassDecl *getCodiraNativeNSObjectDecl();
  toolchain::Module *getModule() const;
  toolchain::AttributeList getAllocAttrs();
  toolchain::Constant *getDeletedAsyncMethodErrorAsyncFunctionPointer();
  toolchain::Constant *
  getDeletedCalleeAllocatedCoroutineMethodErrorCoroFunctionPointer();

private:
  toolchain::Constant *EmptyTupleMetadata = nullptr;
  toolchain::Constant *AnyExistentialMetadata = nullptr;
  toolchain::Constant *AnyObjectExistentialMetadata = nullptr;
  toolchain::Constant *ObjCEmptyCachePtr = nullptr;
  toolchain::Constant *ObjCEmptyVTablePtr = nullptr;
  toolchain::Constant *ObjCISAMaskPtr = nullptr;
  std::optional<toolchain::InlineAsm *> ObjCRetainAutoreleasedReturnValueMarker;
  toolchain::DenseMap<Identifier, ClassDecl*> CodiraRootClasses;
  toolchain::AttributeList AllocAttrs;

#define FUNCTION_ID(Id)                                                        \
public:                                                                        \
  toolchain::Constant *get##Id##Fn();                                               \
  FunctionPointer get##Id##FunctionPointer();                                  \
  toolchain::FunctionType *get##Id##FnType();                                       \
                                                                               \
private:                                                                       \
  toolchain::Constant *Id##Fn = nullptr;
#include "language/Runtime/RuntimeFunctions.def"

  std::optional<FunctionPointer> FixedClassInitializationFn;

  toolchain::Constant *FixLifetimeFn = nullptr;

  mutable std::optional<SpareBitVector> HeapPointerSpareBits;

  //--- Generic ---------------------------------------------------------------
public:
  toolchain::Constant *getFixLifetimeFn();

  FunctionPointer getFixedClassInitializationFn();
  toolchain::Function *getAwaitAsyncContinuationFn();

  /// The constructor used when generating code.
  ///
  /// The \p SF is the source file for which the toolchain module is generated when
  /// doing multi-threaded whole-module compilation. Otherwise it is null.
  IRGenModule(IRGenerator &irgen, std::unique_ptr<toolchain::TargetMachine> &&target,
              SourceFile *SF,
              StringRef ModuleName, StringRef OutputFilename,
              StringRef MainInputFilenameForDebugInfo,
              StringRef PrivateDiscriminator);

  /// The constructor used when we just need an IRGenModule for type lowering.
  IRGenModule(IRGenerator &irgen, std::unique_ptr<toolchain::TargetMachine> &&target)
    : IRGenModule(irgen, std::move(target), /*SF=*/nullptr,
                  "<fake module name>", "<fake output filename>",
                  "<fake main input filename>", "") {}

  ~IRGenModule();

public:
  GeneratedModule intoGeneratedModule() &&;

public:
  toolchain::LLVMContext &getLLVMContext() const { return *LLVMContext; }

  void emitSourceFile(SourceFile &SF);
  void emitSynthesizedFileUnit(SynthesizedFileUnit &SFU);

  void addLinkLibraries();
  void addLinkLibrary(const LinkLibrary &linkLib);

  /// Attempt to finalize the module.
  ///
  /// This can fail, in which it will return false and the module will be
  /// invalid.
  bool finalize();

  void constructInitialFnAttributes(
      toolchain::AttrBuilder &Attrs,
      OptimizationMode FuncOptMode = OptimizationMode::NotSet,
      StackProtectorMode stackProtect = StackProtectorMode::NoStackProtector);
  void setHasNoFramePointer(toolchain::AttrBuilder &Attrs);
  void setHasNoFramePointer(toolchain::Function *F);
  void setMustHaveFramePointer(toolchain::Function *F);
  toolchain::AttributeList constructInitialAttributes();
  StackProtectorMode shouldEmitStackProtector(SILFunction *f);

  void emitProtocolDecl(ProtocolDecl *D);
  void emitEnumDecl(EnumDecl *D);
  void emitStructDecl(StructDecl *D);
  void emitClassDecl(ClassDecl *D);
  void emitExtension(ExtensionDecl *D);
  void emitOpaqueTypeDecl(OpaqueTypeDecl *D);
  /// This method does additional checking vs. \c emitOpaqueTypeDecl
  /// based on the state and flags associated with the module.
  void maybeEmitOpaqueTypeDecl(OpaqueTypeDecl *D);

  void emitSILGlobalVariable(SILGlobalVariable *gv);
  void emitCoverageMaps(ArrayRef<const SILCoverageMap *> Mappings);
  void emitSILFunction(SILFunction *f);
  void emitSILWitnessTable(SILWitnessTable *wt);
  void emitSILProperty(SILProperty *prop);
  void emitSILDifferentiabilityWitness(SILDifferentiabilityWitness *dw);
  toolchain::Constant *emitFixedTypeLayout(CanType t, const FixedTypeInfo &ti);
  void emitProtocolConformance(const ConformanceDescription &record);
  void emitNestedTypeDecls(DeclRange members);
  void emitClangDecl(const clang::Decl *decl);

  void finalizeClangCodeGen();
  void finishEmitAfterTopLevel();

  Signature
  getSignature(CanSILFunctionType fnType,
               const clang::CXXConstructorDecl *cxxCtorDecl = nullptr);
  Signature
  getSignature(CanSILFunctionType fnType, FunctionPointerKind kind,
               bool forStaticCall = false,
               const clang::CXXConstructorDecl *cxxCtorDecl = nullptr);
  toolchain::FunctionType *getFunctionType(CanSILFunctionType type,
                                      toolchain::AttributeList &attrs,
                                      ForeignFunctionInfo *foreignInfo=nullptr);
  ForeignFunctionInfo getForeignFunctionInfo(CanSILFunctionType type);

  void
  ensureImplicitCXXDestructorBodyIsDefined(clang::CXXDestructorDecl *cxxDtor);

  toolchain::ConstantInt *getInt32(uint32_t value);
  toolchain::ConstantInt *getSize(Size size);
  toolchain::Constant *getAlignment(Alignment align);
  toolchain::Constant *getBool(bool condition);

  /// Cast the given constant to i8*.
  toolchain::Constant *getOpaquePtr(toolchain::Constant *pointer);

  toolchain::Constant *getAddrOfAsyncFunctionPointer(LinkEntity entity);
  toolchain::Constant *getAddrOfAsyncFunctionPointer(SILFunction *function);
  toolchain::Constant *defineAsyncFunctionPointer(LinkEntity entity,
                                             ConstantInit init);
  SILFunction *getSILFunctionForAsyncFunctionPointer(toolchain::Constant *afp);

  toolchain::Constant *getAddrOfCoroFunctionPointer(LinkEntity entity);
  toolchain::Constant *getAddrOfCoroFunctionPointer(SILFunction *function);
  toolchain::Constant *defineCoroFunctionPointer(LinkEntity entity,
                                            ConstantInit init);
  SILFunction *getSILFunctionForCoroFunctionPointer(toolchain::Constant *cfp);

  toolchain::Constant *getAddrOfGlobalCoroMallocAllocator();
  toolchain::Constant *getAddrOfGlobalCoroAsyncTaskAllocator();

  toolchain::Function *getAddrOfDispatchThunk(SILDeclRef declRef,
                                         ForDefinition_t forDefinition);
  void emitDispatchThunk(SILDeclRef declRef);

  toolchain::Function *getAddrOfMethodLookupFunction(ClassDecl *classDecl,
                                                ForDefinition_t forDefinition);
  void emitMethodLookupFunction(ClassDecl *classDecl);

  toolchain::GlobalValue *defineAlias(LinkEntity entity, toolchain::Constant *definition,
                                 toolchain::Type *typeOfDefinitionValue);

  toolchain::GlobalValue *defineMethodDescriptor(SILDeclRef declRef,
                                            NominalTypeDecl *nominalDecl,
                                            toolchain::Constant *definition,
                                            toolchain::Type *typeOfDefinitionValue);
  toolchain::Constant *getAddrOfMethodDescriptor(SILDeclRef declRef,
                                            ForDefinition_t forDefinition);
  void emitNonoverriddenMethodDescriptor(const SILVTable *VTable,
                                         SILDeclRef declRef,
                                         ClassDecl *classDecl);

  Address getAddrOfEnumCase(EnumElementDecl *Case,
                            ForDefinition_t forDefinition);
  Address getAddrOfFieldOffset(VarDecl *D, ForDefinition_t forDefinition);
  toolchain::Function *getOrCreateValueWitnessFunction(ValueWitness index,
                                                  FixedPacking packing,
                                                  CanType abstractType,
                                                  SILType concreteType,
                                                  const TypeInfo &type);
  toolchain::Function *getAddrOfValueWitness(CanType concreteType,
                                        ValueWitness index,
                                        ForDefinition_t forDefinition);
  toolchain::Constant *getAddrOfValueWitnessTable(CanType concreteType,
                                             ConstantInit init = ConstantInit());
  std::optional<toolchain::Function *>
  getAddrOfIVarInitDestroy(ClassDecl *cd, bool isDestroyer, bool isForeign,
                           ForDefinition_t forDefinition);

  void setVCallVisibility(toolchain::GlobalVariable *var,
                          toolchain::GlobalObject::VCallVisibility visibility,
                          std::pair<uint64_t, uint64_t> range);

  void addVTableTypeMetadata(
      ClassDecl *decl, toolchain::GlobalVariable *var,
      SmallVector<std::pair<Size, SILDeclRef>, 8> vtableEntries);

  toolchain::GlobalValue *defineTypeMetadata(
      CanType concreteType, bool isPattern, bool isConstant,
      ConstantInitFuture init, toolchain::StringRef section = {},
      SmallVector<std::pair<Size, SILDeclRef>, 8> vtableEntries = {});

  TypeEntityReference
  getContextDescriptorEntityReference(const LinkEntity &entity);

  TypeEntityReference getTypeEntityReference(GenericTypeDecl *D);

  void appendLLVMUsedConditionalEntry(toolchain::GlobalVariable *var,
                                      toolchain::Constant *dependsOn);
  void appendLLVMUsedConditionalEntry(toolchain::GlobalVariable *var,
                                      const ProtocolConformance *conformance);

  void setColocateMetadataSection(toolchain::Function *f);
  void setColocateTypeDescriptorSection(toolchain::GlobalVariable *v);

  toolchain::Constant *
  getAddrOfTypeMetadata(CanType concreteType,
                        TypeMetadataCanonicality canonicality =
                            TypeMetadataCanonicality::Canonical);
  ConstantReference
  getAddrOfTypeMetadata(CanType concreteType, SymbolReferenceKind kind,
                        TypeMetadataCanonicality canonicality =
                            TypeMetadataCanonicality::Canonical);
  toolchain::Constant *getAddrOfTypeMetadataPattern(NominalTypeDecl *D);
  toolchain::Constant *getAddrOfTypeMetadataPattern(NominalTypeDecl *D,
                                               ConstantInit init,
                                               StringRef section);
  toolchain::Function *getAddrOfTypeMetadataCompletionFunction(NominalTypeDecl *D,
                                             ForDefinition_t forDefinition);
  toolchain::Function *getAddrOfTypeMetadataInstantiationFunction(NominalTypeDecl *D,
                                             ForDefinition_t forDefinition);
  toolchain::Constant *getAddrOfTypeMetadataInstantiationCache(NominalTypeDecl *D,
                                             ForDefinition_t forDefinition);
  toolchain::Constant *getAddrOfTypeMetadataSingletonInitializationCache(
                                             NominalTypeDecl *D,
                                             ForDefinition_t forDefinition);
  toolchain::Function *getAddrOfTypeMetadataAccessFunction(CanType type,
                                               ForDefinition_t forDefinition);
  toolchain::Function *getAddrOfGenericTypeMetadataAccessFunction(
                                             NominalTypeDecl *nominal,
                                             ArrayRef<toolchain::Type *> genericArgs,
                                             ForDefinition_t forDefinition);
  toolchain::Function *
  getAddrOfCanonicalSpecializedGenericTypeMetadataAccessFunction(
      CanType theType, ForDefinition_t forDefinition);
  toolchain::Constant *getAddrOfTypeMetadataLazyCacheVariable(CanType type);
  toolchain::Constant *getAddrOfTypeMetadataDemanglingCacheVariable(CanType type,
                                                       ConstantInit definition);
  toolchain::Constant *getAddrOfCanonicalPrespecializedGenericTypeCachingOnceToken(
      NominalTypeDecl *decl);
  toolchain::Constant *getAddrOfNoncanonicalSpecializedGenericTypeMetadataCacheVariable(CanType type);

  toolchain::Constant *getAddrOfClassMetadataBounds(ClassDecl *D,
                                               ForDefinition_t forDefinition);
  toolchain::Constant *getAddrOfTypeContextDescriptor(NominalTypeDecl *D,
                                      RequireMetadata_t requireMetadata,
                                      ConstantInit definition = ConstantInit());
  toolchain::Constant *getAddrOfAnonymousContextDescriptor(
                          PointerUnion<DeclContext *, VarDecl *> Name,
                          ConstantInit definition = ConstantInit());
  toolchain::Constant *getAddrOfExtensionContextDescriptor(ExtensionDecl *ED,
                                      ConstantInit definition = ConstantInit());
  toolchain::Constant *getAddrOfModuleContextDescriptor(ModuleDecl *D,
                                      ConstantInit definition = ConstantInit());
  toolchain::Constant *getAddrOfReflectionFieldDescriptor(CanType type,
                                      ConstantInit definition = ConstantInit());
  toolchain::Constant *getAddrOfReflectionBuiltinDescriptor(CanType type,
                                      ConstantInit definition = ConstantInit());
  toolchain::Constant *getAddrOfReflectionAssociatedTypeDescriptor(
                                      const ProtocolConformance *c,
                                      ConstantInit definition = ConstantInit());
  toolchain::Constant *getAddrOfObjCModuleContextDescriptor();
  toolchain::Constant *getAddrOfClangImporterModuleContextDescriptor();
  toolchain::Constant *getAddrOfOriginalModuleContextDescriptor(StringRef Name);
  ConstantReference getAddrOfParentContextDescriptor(DeclContext *from,
                                                     bool fromAnonymousContext);
  ConstantReference getAddrOfContextDescriptorForParent(DeclContext *parent,
                                                    DeclContext *ofChild,
                                                    bool fromAnonymousContext);
  toolchain::Constant *getAddrOfGenericEnvironment(CanGenericSignature signature);
  toolchain::Constant *getAddrOfProtocolRequirementsBaseDescriptor(
                                                  ProtocolDecl *proto);
  toolchain::GlobalValue *defineProtocolRequirementsBaseDescriptor(
                                                  ProtocolDecl *proto,
                                                  toolchain::Constant *definition);
  toolchain::Constant *getAddrOfAssociatedTypeDescriptor(
                                                AssociatedTypeDecl *assocType);
  toolchain::GlobalValue *defineAssociatedTypeDescriptor(
                                                  AssociatedTypeDecl *assocType,
                                                  toolchain::Constant *definition);
  toolchain::Constant *getAddrOfAssociatedConformanceDescriptor(
                                            AssociatedConformance conformance);
  toolchain::GlobalValue *defineAssociatedConformanceDescriptor(
                                              AssociatedConformance conformance,
                                              toolchain::Constant *definition);
  toolchain::Constant *getAddrOfBaseConformanceDescriptor(
                                                 BaseConformance conformance);
  toolchain::GlobalValue *defineBaseConformanceDescriptor(
                                              BaseConformance conformance,
                                              toolchain::Constant *definition);

  toolchain::Constant *getAddrOfProtocolDescriptor(ProtocolDecl *D,
                                      ConstantInit definition = ConstantInit());
  toolchain::Constant *getAddrOfProtocolConformanceDescriptor(
                                  const RootProtocolConformance *conformance,
                                  ConstantInit definition = ConstantInit());
  toolchain::Constant *getAddrOfPropertyDescriptor(AbstractStorageDecl *D,
                                      ConstantInit definition = ConstantInit());
  toolchain::Constant *getAddrOfObjCClass(ClassDecl *D,
                                     ForDefinition_t forDefinition);
  Address getAddrOfObjCClassRef(ClassDecl *D);
  toolchain::Constant *getAddrOfMetaclassObject(ClassDecl *D,
                                           ForDefinition_t forDefinition);
  toolchain::Constant *getAddrOfCanonicalSpecializedGenericMetaclassObject(
      CanType concreteType, ForDefinition_t forDefinition);

  toolchain::Function *getAddrOfObjCMetadataUpdateFunction(ClassDecl *D,
                                                      ForDefinition_t forDefinition);

  toolchain::Constant *getAddrOfObjCResilientClassStub(ClassDecl *D,
                                                  ForDefinition_t forDefinition,
                                                  TypeMetadataAddress addr);

  toolchain::Function *
  getAddrOfSILFunction(SILFunction *f, ForDefinition_t forDefinition,
                       bool isDynamicallyReplaceableImplementation = false,
                       bool shouldCallPreviousImplementation = false);

  toolchain::Function *
  getAddrOfWitnessTableProfilingThunk(toolchain::Function *witness,
                                      const NormalProtocolConformance &C);

  toolchain::Function *
  getAddrOfVTableProfilingThunk(toolchain::Function *f, ClassDecl *decl);

  toolchain::Function *getOrCreateProfilingThunk(toolchain::Function *f,
                                            StringRef prefix);

  void emitDynamicReplacementOriginalFunctionThunk(SILFunction *f);

  toolchain::Function *getAddrOfContinuationPrototype(CanSILFunctionType fnType);
  Address getAddrOfSILGlobalVariable(SILGlobalVariable *var,
                                     const TypeInfo &ti,
                                     ForDefinition_t forDefinition);
  toolchain::Constant *getGlobalInitValue(SILGlobalVariable *var,
                                     toolchain::Type *storageType,
                                     Alignment alignment);
  toolchain::Constant *getConstantValue(Explosion &&initExp, Size::int_type paddingBytes);
  toolchain::Function *getAddrOfWitnessTableLazyAccessFunction(
                                               const NormalProtocolConformance *C,
                                               CanType conformingType,
                                               ForDefinition_t forDefinition);
  toolchain::Constant *getAddrOfWitnessTableLazyCacheVariable(
                                               const NormalProtocolConformance *C,
                                               CanType conformingType,
                                               ForDefinition_t forDefinition);
  toolchain::Constant *getAddrOfWitnessTable(const ProtocolConformance *C,
                                      ConstantInit definition = ConstantInit());
  toolchain::Constant *getAddrOfWitnessTablePattern(
                                      const NormalProtocolConformance *C,
                                      ConstantInit definition = ConstantInit());

  toolchain::Function *
  getAddrOfGenericWitnessTableInstantiationFunction(
                                    const NormalProtocolConformance *C);
  toolchain::Function *getAddrOfAssociatedTypeWitnessTableAccessFunction(
                                     const NormalProtocolConformance *C,
                                     const AssociatedConformance &association);
  toolchain::Function *getAddrOfDefaultAssociatedConformanceAccessor(
                                           AssociatedConformance requirement);

  toolchain::Constant *
  getAddrOfDifferentiabilityWitness(const SILDifferentiabilityWitness *witness,
                                    ConstantInit definition = ConstantInit());

  Address getAddrOfObjCISAMask();

  toolchain::Function *
  getAddrOfDistributedTargetAccessor(LinkEntity accessor,
                                     ForDefinition_t forDefinition);

  /// Emit a distributed accessor function for the given distributed thunk or
  /// protocol requirement.
  void emitDistributedTargetAccessor(
      toolchain::PointerUnion<SILFunction *, AbstractFunctionDecl *>
          thunkOrRequirement);

  toolchain::Constant *getAddrOfAccessibleFunctionRecord(SILFunction *accessibleFn);

  /// Retrieve the generic signature for the current generic context, or null if no
  /// generic environment is active.
  CanGenericSignature getCurGenericContext();
  
  /// Retrieve the generic environment for the current generic context.
  ///
  /// Fails if there is no generic context.
  GenericEnvironment *getGenericEnvironment();

  ConstantReference
  getAddrOfLLVMVariableOrGOTEquivalent(LinkEntity entity);

  toolchain::Constant *getAddrOfLLVMVariable(LinkEntity entity,
                                        ConstantInit definition,
                                        DebugTypeInfo debugType,
                                        toolchain::Type *overrideDeclType = nullptr);
  toolchain::Constant *getAddrOfLLVMVariable(LinkEntity entity,
                                        ForDefinition_t forDefinition,
                                        DebugTypeInfo debugType);

  toolchain::Constant *emitRelativeReference(ConstantReference target,
                                        toolchain::GlobalValue *base,
                                        ArrayRef<unsigned> baseIndices);

  toolchain::Constant *emitDirectRelativeReference(toolchain::Constant *target,
                                              toolchain::GlobalValue *base,
                                              ArrayRef<unsigned> baseIndices);

  /// Mark a global variable as true-const by putting it in the text section of
  /// the binary.
  void setTrueConstGlobal(toolchain::GlobalVariable *var);

  /// Add the languageself attribute.
  void addCodiraSelfAttributes(toolchain::AttributeList &attrs, unsigned argIndex);

  void addCodiraCoroAttributes(toolchain::AttributeList &attrs, unsigned argIndex);

  void addCodiraAsyncContextAttributes(toolchain::AttributeList &attrs,
                                      unsigned argIndex);

  /// Add the languageerror attribute.
  void addCodiraErrorAttributes(toolchain::AttributeList &attrs, unsigned argIndex);

  void emitSharedContextDescriptor(DeclContext *dc);

  toolchain::GlobalVariable *
  getGlobalForDynamicallyReplaceableThunk(LinkEntity &entity, toolchain::Type *type,
                                          ForDefinition_t forDefinition);

  FunctionPointer getAddrOfOpaqueTypeDescriptorAccessFunction(
      OpaqueTypeDecl *decl, ForDefinition_t forDefinition, bool implementation);

  void createReplaceableProlog(IRGenFunction &IGF, SILFunction *f);

  void emitOpaqueTypeDescriptorAccessor(OpaqueTypeDecl *);

  /// We emit Objective-C class stubs for non-generic classes with resilient
  /// ancestry. This lets us attach categories to the class even though it
  /// does not have statically-emitted metadata.
  bool hasObjCResilientClassStub(ClassDecl *D);

  /// Emit a resilient class stub.
  toolchain::Constant *emitObjCResilientClassStub(ClassDecl *D, bool isPublic);

  /// Runs additional lowering logic on the given SIL function to ensure that
  /// the SIL function is correctly lowered even if the lowering passes do not
  /// run on the SIL module that owns this function.
  void lowerSILFunction(SILFunction *f);

  toolchain::Function *getForeignExceptionHandlingPersonalityFunc();

  bool isForeignExceptionHandlingEnabled() const;

  /// Returns true if the given Clang function does not throw exceptions.
  bool isCxxNoThrow(clang::FunctionDecl *fd, bool defaultNoThrow = false);

private:
  toolchain::Constant *
  getAddrOfSharedContextDescriptor(LinkEntity entity,
                                   ConstantInit definition,
                                   toolchain::function_ref<void()> emit);

  ConstantReference getAddrOfLLVMVariable(LinkEntity entity,
                                        ConstantInit definition,
                                        DebugTypeInfo debugType,
                                        SymbolReferenceKind refKind,
                                        toolchain::Type *overrideDeclType = nullptr);

  void emitLazyPrivateDefinitions();
  void addRuntimeResolvableType(GenericTypeDecl *nominal);

  /// Add all conformances of the given \c IterableDeclContext
  /// LazyWitnessTables.
  void addLazyConformances(const IterableDeclContext *idc);

//--- Global context emission --------------------------------------------------
  bool hasCodiraAsyncFunctionDef = false;
  toolchain::Value *extendedFramePointerFlagsWeakRef = nullptr;

  /// Emit a weak reference to the `language_async_extendedFramePointerFlags`
  /// symbol needed by Codira async functions.
  void emitCodiraAsyncExtendedFrameInfoWeakRef();
public:
  bool isConcurrencyAvailable();
  void noteCodiraAsyncFunctionDef() {
    hasCodiraAsyncFunctionDef = true;
  }
  void emitRuntimeRegistration();
  void emitVTableStubs();
  void emitTypeVerifier();
  toolchain::Function *emitHasSymbolFunction(ValueDecl *decl);

  /// Create toolchain metadata which encodes the branch weights given by
  /// \p TrueCount and \p FalseCount.
  toolchain::MDNode *createProfileWeights(uint64_t TrueCount,
                                     uint64_t FalseCount) const;

private:
  void emitGlobalDecl(Decl *D);
};

/// Stores a pointer to an IRGenModule.
/// As long as the CurrentIGMPtr is alive, the CurrentIGM in the dispatcher
/// is set to the containing IRGenModule.
class CurrentIGMPtr {
  IRGenModule *IGM;

public:
  CurrentIGMPtr(IRGenModule *IGM) : IGM(IGM) {
    assert(IGM);
    assert(!IGM->IRGen.CurrentIGM && "Another CurrentIGMPtr is alive");
    IGM->IRGen.CurrentIGM = IGM;
  }

  ~CurrentIGMPtr() {
    IGM->IRGen.CurrentIGM = nullptr;
  }
  
  IRGenModule *get() const { return IGM; }
  IRGenModule *operator->() const { return IGM; }
};

/// Workaround to disable thumb-mode until debugger support is there.
bool shouldRemoveTargetFeature(StringRef);

struct CXXBaseRecordLayout {
  const clang::CXXRecordDecl *decl;
  Size offset;
  Size size;
};

/// Retrieves the base classes of a C++ struct/class ordered by their offset in
/// the derived type's memory layout.
SmallVector<CXXBaseRecordLayout, 1>
getBasesAndOffsets(const clang::CXXRecordDecl *decl);

} // end namespace irgen
} // end namespace language

namespace toolchain {

template<>
struct DenseMapInfo<language::irgen::IRGenModule::FixedLayoutKey> {
  using FixedLayoutKey = language::irgen::IRGenModule::FixedLayoutKey;

  static inline FixedLayoutKey getEmptyKey() {
    return {0, 0xFFFFFFFFu, 0, 0, 0};
  }

  static inline FixedLayoutKey getTombstoneKey() {
    return {0, 0xFFFFFFFEu, 0, 0, 0};
  }

  static unsigned getHashValue(const FixedLayoutKey &key) {
    return hash_combine(key.size, key.numExtraInhabitants, key.align,
                        (bool)key.pod, (bool)key.bitwiseTakable);
  }
  static bool isEqual(const FixedLayoutKey &a, const FixedLayoutKey &b) {
    return a.size == b.size
      && a.numExtraInhabitants == b.numExtraInhabitants
      && a.align == b.align
      && a.pod == b.pod
      && a.bitwiseTakable == b.bitwiseTakable;
  }
};

}

#endif
