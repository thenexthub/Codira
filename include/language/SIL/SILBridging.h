//===--- SILBridging.h - header for the language SILBridging module ----------===//
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

#ifndef LANGUAGE_SIL_SILBRIDGING_H
#define LANGUAGE_SIL_SILBRIDGING_H

// Do not add other C++/toolchain/language header files here!
// Function implementations should be placed into SILBridgingImpl.h or SILBridging.cpp and
// required header files should be added there.
//
// Pure bridging mode does not permit including any C++/toolchain/language headers.
// See also the comments for `BRIDGING_MODE` in the top-level CMakeLists.txt file.
//
#include "language/AST/ASTBridging.h"

#ifdef USED_IN_CPP_SOURCE
#include "toolchain/ADT/ArrayRef.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILWitnessTable.h"
#endif

LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS

struct BridgedInstruction;
struct OptionalBridgedInstruction;
struct OptionalBridgedOperand;
struct OptionalBridgedSuccessor;
struct BridgedFunction;
struct BridgedBasicBlock;
struct BridgedSuccessorArray;
struct OptionalBridgedBasicBlock;
struct BridgedDeclRef;

namespace language {
class ValueBase;
class Operand;
struct SILDebugVariable;
class ForwardingInstruction;
class SILType;
class SILFunction;
class SILBasicBlock;
class SILSuccessor;
class SILGlobalVariable;
class SILInstruction;
class SILArgument;
class MultipleValueInstructionResult;
struct ValueOwnershipKind;
class SILVTableEntry;
class SILVTable;
class ConstExprFunctionState;
class SymbolicValueBumpAllocator;
class ConstExprEvaluator;
class SILWitnessTable;
class SILDefaultWitnessTable;
class SILDebugLocation;
class NominalTypeDecl;
class VarDecl;
class CodiraPassInvocation;
class GenericSpecializationInformation;
class LifetimeDependenceInfo;
class IndexSubset;
enum class ResultConvention : uint8_t;
class SILResultInfo;
class SILParameterInfo;
struct SILDeclRef;
class SILBuilder;
class SILLocation;
}

bool languageModulesInitialized();
void registerBridgedClass(BridgedStringRef className, CodiraMetatype metatype);

enum class BridgedResultConvention {
  Indirect,
  Owned,
  Unowned,
  UnownedInnerPointer,
  Autoreleased,
  Pack
};

struct BridgedResultInfo {
  language::TypeBase * _Nonnull type;
  BridgedResultConvention convention;

  BRIDGED_INLINE static BridgedResultConvention castToResultConvention(language::ResultConvention convention);
  BRIDGED_INLINE BridgedResultInfo(language::SILResultInfo resultInfo);
};

struct OptionalBridgedResultInfo {
  language::TypeBase * _Nullable type;
  BridgedResultConvention convention;
};

struct BridgedResultInfoArray {
  BridgedArrayRef resultInfoArray;

  BRIDGED_INLINE CodiraInt count() const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  BridgedResultInfo at(CodiraInt resultIndex) const;
};

// Unfortunately we need to take a detour over this enum.
// Currently it's not possible to switch over `SILArgumentConvention::ConventionType`,
// because it's not a class enum.
enum class BridgedArgumentConvention {
  Indirect_In,
  Indirect_In_Guaranteed,
  Indirect_Inout,
  Indirect_InoutAliasable,
  Indirect_In_CXX,
  Indirect_Out,
  Direct_Owned,
  Direct_Unowned,
  Direct_Guaranteed,
  Pack_Owned,
  Pack_Inout,
  Pack_Guaranteed,
  Pack_Out
};

struct BridgedParameterInfo {
  BridgedCanType type;
  BridgedArgumentConvention convention;
  uint8_t options;

  BridgedParameterInfo(BridgedCanType type, BridgedArgumentConvention convention, uint8_t options) :
    type(type), convention(convention), options(options) {}

  BRIDGED_INLINE BridgedParameterInfo(language::SILParameterInfo parameterInfo);
  BRIDGED_INLINE language::SILParameterInfo unbridged() const;
};

struct BridgedParameterInfoArray {
  BridgedArrayRef parameterInfoArray;

  BRIDGED_INLINE CodiraInt count() const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  BridgedParameterInfo at(CodiraInt parameterIndex) const;
};

struct BridgedYieldInfoArray {
  BridgedArrayRef yieldInfoArray;

  BRIDGED_INLINE CodiraInt count() const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  BridgedParameterInfo at(CodiraInt resultIndex) const;
};
struct BridgedLifetimeDependenceInfo {
  language::IndexSubset *_Nullable inheritLifetimeParamIndices;
  language::IndexSubset *_Nullable scopeLifetimeParamIndices;
  language::IndexSubset *_Nullable addressableParamIndices;
  language::IndexSubset *_Nullable conditionallyAddressableParamIndices;
  CodiraUInt targetIndex;
  bool immortal;

  BRIDGED_INLINE BridgedLifetimeDependenceInfo(language::LifetimeDependenceInfo info);

  BRIDGED_INLINE bool empty() const;
  BRIDGED_INLINE bool checkInherit(CodiraInt index) const;
  BRIDGED_INLINE bool checkScope(CodiraInt index) const;
  BRIDGED_INLINE bool checkAddressable(CodiraInt index) const;
  BRIDGED_INLINE bool checkConditionallyAddressable(CodiraInt index) const;
  BRIDGED_INLINE CodiraInt getTargetIndex() const;

  BRIDGED_INLINE BridgedOwnedString getDebugDescription() const;
};

struct BridgedLifetimeDependenceInfoArray {
  BridgedArrayRef lifetimeDependenceInfoArray;

  BRIDGED_INLINE CodiraInt count() const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedLifetimeDependenceInfo
  at(CodiraInt index) const;
};

enum class BridgedLinkage {
  Public,
  PublicNonABI,
  Package,
  PackageNonABI,
  Hidden,
  Shared,
  Private,
  PublicExternal,
  PackageExternal,
  HiddenExternal
};

// =========================================================================//
//                              SILFunctionType
// =========================================================================//

LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
BridgedResultInfoArray SILFunctionType_getResultsWithError(BridgedCanType);

BRIDGED_INLINE CodiraInt
SILFunctionType_getNumIndirectFormalResultsWithError(BridgedCanType);

BRIDGED_INLINE CodiraInt SILFunctionType_getNumPackResults(BridgedCanType);

LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedResultInfo SILFunctionType_getErrorResult(BridgedCanType);

LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
BridgedParameterInfoArray SILFunctionType_getParameters(BridgedCanType);

BRIDGED_INLINE bool SILFunctionType_hasSelfParam(BridgedCanType);

BRIDGED_INLINE bool SILFunctionType_isTrivialNoescape(BridgedCanType);

LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
BridgedYieldInfoArray SILFunctionType_getYields(BridgedCanType);

LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedLifetimeDependenceInfoArray
SILFunctionType_getLifetimeDependencies(BridgedCanType);


struct BridgedType {
  void * _Nullable opaqueValue;

  struct EnumElementIterator {
    uint64_t storage[4];

    LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE EnumElementIterator getNext() const;
  };

  BRIDGED_INLINE BridgedType(); // for Optional<Type>.nil
  BRIDGED_INLINE BridgedType(language::SILType t);
  BRIDGED_INLINE language::SILType unbridged() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType getCanType() const;

  static LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType createSILType(BridgedCanType canTy);
  BRIDGED_INLINE BridgedOwnedString getDebugDescription() const;
  BRIDGED_INLINE bool isNull() const;
  BRIDGED_INLINE bool isAddress() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getAddressType() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getObjectType() const;
  BRIDGED_INLINE bool isTrivial(BridgedFunction f) const;
  BRIDGED_INLINE bool isNonTrivialOrContainsRawPointer(BridgedFunction f) const;
  BRIDGED_INLINE bool isLoadable(BridgedFunction f) const;
  BRIDGED_INLINE bool isReferenceCounted(BridgedFunction f) const;
  BRIDGED_INLINE bool containsNoEscapeFunction() const;
  BRIDGED_INLINE bool isEmpty(BridgedFunction f) const;
  BRIDGED_INLINE bool isMoveOnly() const;
  BRIDGED_INLINE bool isEscapable(BridgedFunction f) const;
  BRIDGED_INLINE bool isExactSuperclassOf(BridgedType t) const;
  BRIDGED_INLINE bool isMarkedAsImmortal() const;
  BRIDGED_INLINE bool isAddressableForDeps(BridgedFunction f) const;
  BRIDGED_INLINE CodiraInt getCaseIdxOfEnumType(BridgedStringRef name) const;
  static BRIDGED_INLINE CodiraInt getNumBoxFields(BridgedCanType boxTy);
  static LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getBoxFieldType(BridgedCanType boxTy,
                                                                        CodiraInt idx, BridgedFunction f);
  static BRIDGED_INLINE bool getBoxFieldIsMutable(BridgedCanType boxTy, CodiraInt idx);
  BRIDGED_INLINE CodiraInt getNumNominalFields() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getFieldType(CodiraInt idx, BridgedFunction f) const;
  BRIDGED_INLINE CodiraInt getFieldIdxOfNominalType(BridgedStringRef name) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedStringRef getFieldName(CodiraInt idx) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE EnumElementIterator getFirstEnumCaseIterator() const;
  BRIDGED_INLINE bool isEndCaseIterator(EnumElementIterator i) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getEnumCasePayload(EnumElementIterator i, BridgedFunction f) const;
  BRIDGED_INLINE CodiraInt getNumTupleElements() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType
  getTupleElementType(CodiraInt idx) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getFunctionTypeWithNoEscape(bool withNoEscape) const;
  BRIDGED_INLINE BridgedArgumentConvention getCalleeConvention() const;
};

// SIL Bridging

struct BridgedValue {
  CodiraObject obj;

  enum class Kind {
    SingleValueInstruction,
    Argument,
    MultipleValueInstructionResult,
    Undef
  };

  // Unfortunately we need to take a detour over this enum.
  // Currently it's not possible to switch over `OwnershipKind::inntery`, because it's not a class enum.
  enum class Ownership {
    Unowned,
    Owned,
    Guaranteed,
    None
  };

  BRIDGED_INLINE static language::ValueOwnershipKind unbridge(Ownership ownership);

  Kind getKind() const;
  BRIDGED_INLINE language::ValueBase * _Nonnull getSILValue() const;
  BridgedOwnedString getDebugDescription() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedOperand getFirstUse() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getType() const;
  BRIDGED_INLINE Ownership getOwnership() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedFunction SILUndef_getParentFunction() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedFunction PlaceholderValue_getParentFunction() const;

  bool findPointerEscape() const;
};

struct OptionalBridgedValue {
  OptionalCodiraObject obj;

  BRIDGED_INLINE language::ValueBase * _Nullable getSILValue() const;
};

// This is the layout of a class existential.
struct BridgeValueExistential {
  BridgedValue value;
  void * _Nonnull conformance;
};

struct BridgedValueArray {
  const BridgeValueExistential * _Nullable base;
  size_t count;

#ifdef USED_IN_CPP_SOURCE
  toolchain::ArrayRef<language::SILValue> getValues(toolchain::SmallVectorImpl<language::SILValue> &storage);
#endif
};

struct BridgedOperand {
  language::Operand * _Nonnull op;

  enum class OperandOwnership {
    NonUse,
    TrivialUse,
    InstantaneousUse,
    UnownedInstantaneousUse,
    ForwardingUnowned,
    PointerEscape,
    BitwiseEscape,
    Borrow,
    DestroyingConsume,
    ForwardingConsume,
    InteriorPointer,
    AnyInteriorPointer,
    GuaranteedForwarding,
    EndBorrow,
    Reborrow
  };

  BRIDGED_INLINE bool isTypeDependent() const;
  BRIDGED_INLINE bool isLifetimeEnding() const;
  BRIDGED_INLINE bool canAcceptOwnership(BridgedValue::Ownership ownership) const;
  BRIDGED_INLINE bool isDeleted() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedOperand getNextUse() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedValue getValue() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction getUser() const;
  BRIDGED_INLINE OperandOwnership getOperandOwnership() const;
  void changeOwnership(BridgedValue::Ownership from, BridgedValue::Ownership to) const;
};

struct OptionalBridgedOperand {
  language::Operand * _Nullable op;

  // Assumes that `op` is not null.
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  BridgedOperand advancedBy(CodiraInt index) const;

  // Assumes that `op` is not null.
  BRIDGED_INLINE CodiraInt distanceTo(BridgedOperand element) const;
};

struct BridgedOperandArray {
  OptionalBridgedOperand base;
  CodiraInt count;
};

enum class BridgedMemoryBehavior {
  None,
  MayRead,
  MayWrite,
  MayReadWrite,
  MayHaveSideEffects
};

struct BridgedLocation {
  uint64_t storage[3];

  struct FilenameAndLocation {
    BridgedStringRef path;
    CodiraInt line;
    CodiraInt column;
  };

  BRIDGED_INLINE BridgedLocation(const language::SILDebugLocation &loc);
  BRIDGED_INLINE const language::SILDebugLocation &getLoc() const;

  BridgedOwnedString getDebugDescription() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedLocation getAutogeneratedLocation() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedLocation getCleanupLocation() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedLocation withScopeOf(BridgedLocation other) const;
  BRIDGED_INLINE bool hasValidLineNumber() const;
  BRIDGED_INLINE bool isAutoGenerated() const;
  BRIDGED_INLINE bool isInlined() const;
  BRIDGED_INLINE bool isEqualTo(BridgedLocation rhs) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedSourceLoc getSourceLocation() const;
  BRIDGED_INLINE bool isFilenameAndLocation() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE FilenameAndLocation getFilenameAndLocation() const;
  BRIDGED_INLINE bool hasSameSourceLocation(BridgedLocation rhs) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedDeclObj getDecl() const;
  static BRIDGED_INLINE BridgedLocation fromNominalTypeDecl(BridgedDeclObj decl);
  static BRIDGED_INLINE BridgedLocation getArtificialUnreachableLocation();
};

struct BridgedFunction {
  CodiraObject obj;

  enum class EffectsKind {
    ReadNone,
    ReadOnly,
    ReleaseNone,
    ReadWrite,
    Unspecified,
    Custom
  };

  enum class PerformanceConstraints {
    None = 0,
    NoAllocation = 1,
    NoLocks = 2,
    NoRuntime = 3,
    NoExistentials = 4,
    NoObjCBridging = 5
  };

  enum class InlineStrategy {
    InlineDefault,
    NoInline,
    AlwaysInline
  };

  enum class ThunkKind {
    IsNotThunk,
    IsThunk,
    IsReabstractionThunk,
    IsSignatureOptimizedThunk
  };

  enum class SerializedKind {
    IsNotSerialized,
    IsSerialized,
    IsSerializedForPackage
  };

  enum class OptionalSourceFileKind {
    Library,
    Main,
    SIL,
    Interface,
    MacroExpansion,
    DefaultArgument, // must match language::SourceFileKind::DefaultArgument
    None
  };

  LANGUAGE_NAME("init(obj:)")
  BridgedFunction(CodiraObject obj) : obj(obj) {}
  BridgedFunction() {}
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE language::SILFunction * _Nonnull getFunction() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedStringRef getName() const;
  BridgedOwnedString getDebugDescription() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedLocation getLocation() const;
  BRIDGED_INLINE bool isAccessor() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedStringRef getAccessorName() const;
  BRIDGED_INLINE bool hasOwnership() const;
  BRIDGED_INLINE bool hasLoweredAddresses() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType getLoweredFunctionTypeInContext() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedGenericSignature getGenericSignature() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedSubstitutionMap getForwardingSubstitutionMap() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedASTType mapTypeIntoContext(BridgedASTType ty) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedBasicBlock getFirstBlock() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedBasicBlock getLastBlock() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclRef getDeclRef() const;
  BRIDGED_INLINE CodiraInt getNumIndirectFormalResults() const;
  BRIDGED_INLINE bool hasIndirectErrorResult() const;
  BRIDGED_INLINE CodiraInt getNumSILArguments() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getSILArgumentType(CodiraInt idx) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getSILResultType() const;
  BRIDGED_INLINE bool isCodira51RuntimeAvailable() const;
  BRIDGED_INLINE bool isPossiblyUsedExternally() const;
  BRIDGED_INLINE bool isTransparent() const;
  BRIDGED_INLINE bool isAsync() const;
  BRIDGED_INLINE bool isGlobalInitFunction() const;
  BRIDGED_INLINE bool isGlobalInitOnceFunction() const;
  BRIDGED_INLINE bool isDestructor() const;
  BRIDGED_INLINE bool isGeneric() const;
  BRIDGED_INLINE bool hasSemanticsAttr(BridgedStringRef attrName) const;
  BRIDGED_INLINE bool hasUnsafeNonEscapableResult() const;
  BRIDGED_INLINE bool hasDynamicSelfMetadata() const;
  BRIDGED_INLINE EffectsKind getEffectAttribute() const;
  BRIDGED_INLINE PerformanceConstraints getPerformanceConstraints() const;
  BRIDGED_INLINE InlineStrategy getInlineStrategy() const;
  BRIDGED_INLINE SerializedKind getSerializedKind() const;
  BRIDGED_INLINE bool canBeInlinedIntoCaller(SerializedKind) const;
  BRIDGED_INLINE bool hasValidLinkageForFragileRef(SerializedKind) const;
  BRIDGED_INLINE ThunkKind isThunk() const;
  BRIDGED_INLINE void setThunk(ThunkKind) const;
  BRIDGED_INLINE bool needsStackProtection() const;
  BRIDGED_INLINE bool shouldOptimize() const;
  BRIDGED_INLINE bool isReferencedInModule() const;
  BRIDGED_INLINE bool wasDeserializedCanonical() const;
  BRIDGED_INLINE void setNeedStackProtection(bool needSP) const;
  BRIDGED_INLINE void setIsPerformanceConstraint(bool isPerfConstraint) const;
  BRIDGED_INLINE bool isResilientNominalDecl(BridgedDeclObj decl) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getLoweredType(BridgedASTType type, bool maximallyAbstracted) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getLoweredType(BridgedType type) const;
  BRIDGED_INLINE BridgedLinkage getLinkage() const;
  BRIDGED_INLINE void setLinkage(BridgedLinkage linkage) const;
  BRIDGED_INLINE void setIsSerialized(bool isSerialized) const;
  BRIDGED_INLINE bool conformanceMatchesActorIsolation(BridgedConformance conformance) const;
  BRIDGED_INLINE bool isSpecialization() const;
  bool isTrapNoReturn() const;
  bool isConvertPointerToPointerArgument() const;
  bool isAutodiffVJP() const;
  CodiraInt specializationLevel() const;
  LANGUAGE_IMPORT_UNSAFE BridgedSubstitutionMap getMethodSubstitutions(BridgedSubstitutionMap contextSubs,
                                                                    BridgedCanType selfType) const;
  BRIDGED_INLINE OptionalSourceFileKind getSourceFileKind() const;

  enum class ParseEffectsMode {
    argumentEffectsFromSource,
    argumentEffectsFromSIL,
    globalEffectsFromSIL,
    multipleEffectsFromSIL
  };

  struct ParsingError {
    const unsigned char * _Nullable message;
    CodiraInt position;
  };

  struct EffectInfo {
    CodiraInt argumentIndex;
    bool isDerived;
    bool isEmpty;
    bool isValid;
  };

  typedef void (* _Nonnull RegisterFn)(BridgedFunction f, void * _Nonnull data, CodiraInt size);
  typedef void (* _Nonnull WriteFn)(BridgedFunction, BridgedOStream, CodiraInt);
  typedef ParsingError (*_Nonnull ParseFn)(BridgedFunction,
                                           BridgedStringRef,
                                           ParseEffectsMode, CodiraInt,
                                           BridgedArrayRef);
  typedef CodiraInt (* _Nonnull CopyEffectsFn)(BridgedFunction, BridgedFunction);
  typedef EffectInfo (* _Nonnull GetEffectInfoFn)(BridgedFunction, CodiraInt);
  typedef BridgedMemoryBehavior (* _Nonnull GetMemBehaviorFn)(BridgedFunction, bool);
  typedef bool (* _Nonnull ArgumentMayReadFn)(BridgedFunction, BridgedOperand, BridgedValue);
  typedef bool (* _Nonnull IsDeinitBarrierFn)(BridgedFunction);

  static void registerBridging(CodiraMetatype metatype,
              RegisterFn initFn, RegisterFn destroyFn,
              WriteFn writeFn, ParseFn parseFn,
              CopyEffectsFn copyEffectsFn,
              GetEffectInfoFn effectInfoFn,
              GetMemBehaviorFn memBehaviorFn,
              ArgumentMayReadFn argumentMayReadFn,
              IsDeinitBarrierFn isDeinitBarrierFn);
};

struct OptionalBridgedFunction {
  OptionalCodiraObject obj;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE language::SILFunction * _Nullable getFunction() const;
};

struct BridgedGlobalVar {
  CodiraObject obj;

  BridgedGlobalVar(CodiraObject obj) : obj(obj) {}
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE language::SILGlobalVariable * _Nonnull getGlobal() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedDeclObj getDecl() const;
  BridgedOwnedString getDebugDescription() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedStringRef getName() const;
  BRIDGED_INLINE bool isLet() const;
  BRIDGED_INLINE void setLet(bool value) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getType() const;
  BRIDGED_INLINE BridgedLinkage getLinkage() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedSourceLoc getSourceLocation() const;
  BRIDGED_INLINE bool isPossiblyUsedExternally() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedInstruction getFirstStaticInitInst() const;
  bool canBeInitializedStatically() const;
  bool mustBeInitializedStatically() const;
  bool isConstValue() const;
};

struct OptionalBridgedGlobalVar {
  OptionalCodiraObject obj;
};

struct BridgedMultiValueResult {
  CodiraObject obj;

  BRIDGED_INLINE language::MultipleValueInstructionResult * _Nonnull unbridged() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction getParent() const;
  BRIDGED_INLINE CodiraInt getIndex() const;
};

struct BridgedSILTypeArray {
  BridgedArrayRef typeArray;

  CodiraInt getCount() const { return CodiraInt(typeArray.Length); }

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  BridgedType getAt(CodiraInt index) const;
};

struct BridgedGenericSpecializationInformation {
  const language::GenericSpecializationInformation * _Nullable data = nullptr;
};

struct BridgedSILDebugVariable {
  uint64_t storage[16];

  BRIDGED_INLINE BridgedSILDebugVariable() {}
  BRIDGED_INLINE BridgedSILDebugVariable(const language::SILDebugVariable &var);
  BRIDGED_INLINE BridgedSILDebugVariable(const BridgedSILDebugVariable &rhs);
  BRIDGED_INLINE ~BridgedSILDebugVariable();
  BRIDGED_INLINE BridgedSILDebugVariable &operator=(const BridgedSILDebugVariable &rhs);
  BRIDGED_INLINE language::SILDebugVariable unbridge() const;
};

struct BridgedInstruction {
  CodiraObject obj;

#ifdef USED_IN_CPP_SOURCE
  template <class I> I *_Nonnull getAs() const {
    return toolchain::cast<I>(static_cast<language::SILNode *>(obj)->castToInstruction());
  }
  language::SILInstruction * _Nonnull unbridged() const {
    return getAs<language::SILInstruction>();
  }
#endif

  BridgedInstruction(CodiraObject obj) : obj(obj) {}
  BridgedOwnedString getDebugDescription() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedInstruction getNext() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedInstruction getPrevious() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlock getParent() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction getLastInstOfParent() const;
  BRIDGED_INLINE bool isDeleted() const;
  BRIDGED_INLINE bool isInStaticInitializer() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedOperandArray getOperands() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedOperandArray getTypeDependentOperands() const;
  BRIDGED_INLINE void setOperand(CodiraInt index, BridgedValue value) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedLocation getLocation() const;
  BRIDGED_INLINE BridgedMemoryBehavior getMemBehavior() const;
  BRIDGED_INLINE bool mayRelease() const;
  BRIDGED_INLINE bool mayHaveSideEffects() const;
  BRIDGED_INLINE bool maySuspend() const;
  bool mayAccessPointer() const;
  bool mayLoadWeakOrUnowned() const;
  bool maySynchronize() const;
  bool mayBeDeinitBarrierNotConsideringSideEffects() const;
  BRIDGED_INLINE bool shouldBeForwarding() const;

  // =========================================================================//
  //                   Generalized instruction subclasses
  // =========================================================================//
  
  BRIDGED_INLINE CodiraInt MultipleValueInstruction_getNumResults() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedMultiValueResult MultipleValueInstruction_getResult(CodiraInt index) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedSuccessorArray TermInst_getSuccessors() const;

  // =========================================================================//
  //                         Instruction protocols
  // =========================================================================//

  BRIDGED_INLINE language::ForwardingInstruction * _Nonnull getAsForwardingInstruction() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedOperand ForwardingInst_singleForwardedOperand() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedOperandArray ForwardingInst_forwardedOperands() const;
  BRIDGED_INLINE BridgedValue::Ownership ForwardingInst_forwardingOwnership() const;
  BRIDGED_INLINE void ForwardingInst_setForwardingOwnership(BridgedValue::Ownership ownership) const;
  BRIDGED_INLINE bool ForwardingInst_preservesOwnership() const;

  // =========================================================================//
  //                    Specific instruction subclasses
  // =========================================================================//

  enum class BuiltinValueKind {
    None = 0,
#define BUILTIN(Id, Name, Attrs) Id,
#include "language/AST/Builtins.def"
  };

  enum class IntrinsicID {
    memcpy, memmove,
    unknown
  };

  enum class MarkDependenceKind {
    Unresolved, Escaping, NonEscaping
  };

  struct KeyPathFunctionResults {
    enum { maxFunctions = 5 };
    BridgedFunction functions[maxFunctions];
    CodiraInt numFunctions;
  };

  enum class CastConsumptionKind {
    TakeAlways,
    TakeOnSuccess,
    CopyOnSuccess
  };

  struct CheckedCastInstOptions {
    uint8_t storage;
  };

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedStringRef CondFailInst_getMessage() const;
  BRIDGED_INLINE CodiraInt LoadInst_getLoadOwnership() const ;
  BRIDGED_INLINE bool LoadBorrowInst_isUnchecked() const ;
  BRIDGED_INLINE BuiltinValueKind BuiltinInst_getID() const;
  BRIDGED_INLINE IntrinsicID BuiltinInst_getIntrinsicID() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedStringRef BuiltinInst_getName() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedSubstitutionMap BuiltinInst_getSubstitutionMap() const;
  BRIDGED_INLINE bool PointerToAddressInst_isStrict() const;
  BRIDGED_INLINE bool PointerToAddressInst_isInvariant() const;
  BRIDGED_INLINE uint64_t PointerToAddressInst_getAlignment() const;
  BRIDGED_INLINE void PointerToAddressInst_setAlignment(uint64_t alignment) const;
  BRIDGED_INLINE bool AddressToPointerInst_needsStackProtection() const;
  BRIDGED_INLINE bool IndexAddrInst_needsStackProtection() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedConformanceArray InitExistentialRefInst_getConformances() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType InitExistentialRefInst_getFormalConcreteType() const;
  BRIDGED_INLINE bool OpenExistentialAddr_isImmutable() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedGlobalVar GlobalAccessInst_getGlobal() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedGlobalVar AllocGlobalInst_getGlobal() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedFunction FunctionRefBaseInst_getReferencedFunction() const;
  BRIDGED_INLINE BridgedOptionalInt IntegerLiteralInst_getValue() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedStringRef StringLiteralInst_getValue() const;
  BRIDGED_INLINE int StringLiteralInst_getEncoding() const;
  BRIDGED_INLINE CodiraInt TupleExtractInst_fieldIndex() const;
  BRIDGED_INLINE CodiraInt TupleElementAddrInst_fieldIndex() const;
  BRIDGED_INLINE CodiraInt StructExtractInst_fieldIndex() const;
  BRIDGED_INLINE OptionalBridgedValue StructInst_getUniqueNonTrivialFieldValue() const;
  BRIDGED_INLINE CodiraInt StructElementAddrInst_fieldIndex() const;
  BRIDGED_INLINE bool BeginBorrow_isLexical() const;
  BRIDGED_INLINE bool BeginBorrow_hasPointerEscape() const;
  BRIDGED_INLINE bool BeginBorrow_isFromVarDecl() const;
  BRIDGED_INLINE bool MoveValue_isLexical() const;
  BRIDGED_INLINE bool MoveValue_hasPointerEscape() const;
  BRIDGED_INLINE bool MoveValue_isFromVarDecl() const;

  BRIDGED_INLINE CodiraInt ProjectBoxInst_fieldIndex() const;
  BRIDGED_INLINE bool EndCOWMutationInst_doKeepUnique() const;
  BRIDGED_INLINE bool DestroyValueInst_isDeadEnd() const;
  BRIDGED_INLINE CodiraInt EnumInst_caseIndex() const;
  BRIDGED_INLINE CodiraInt UncheckedEnumDataInst_caseIndex() const;
  BRIDGED_INLINE CodiraInt InitEnumDataAddrInst_caseIndex() const;
  BRIDGED_INLINE CodiraInt UncheckedTakeEnumDataAddrInst_caseIndex() const;
  BRIDGED_INLINE CodiraInt InjectEnumAddrInst_caseIndex() const;
  BRIDGED_INLINE CodiraInt RefElementAddrInst_fieldIndex() const;
  BRIDGED_INLINE bool RefElementAddrInst_fieldIsLet() const;
  BRIDGED_INLINE bool RefElementAddrInst_isImmutable() const;
  BRIDGED_INLINE void RefElementAddrInst_setImmutable(bool isImmutable) const;
  BRIDGED_INLINE bool RefTailAddrInst_isImmutable() const;
  BRIDGED_INLINE CodiraInt PartialApplyInst_numArguments() const;
  BRIDGED_INLINE CodiraInt ApplyInst_numArguments() const;
  BRIDGED_INLINE bool ApplyInst_getNonThrowing() const;
  BRIDGED_INLINE bool ApplyInst_getNonAsync() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedGenericSpecializationInformation ApplyInst_getSpecializationInfo() const;
  BRIDGED_INLINE bool TryApplyInst_getNonAsync() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedGenericSpecializationInformation TryApplyInst_getSpecializationInfo() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclRef ClassMethodInst_getMember() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclRef WitnessMethodInst_getMember() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType WitnessMethodInst_getLookupType() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclObj WitnessMethodInst_getLookupProtocol() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedConformance WitnessMethodInst_getConformance() const;
  BRIDGED_INLINE CodiraInt ObjectInst_getNumBaseElements() const;
  BRIDGED_INLINE CodiraInt PartialApply_getCalleeArgIndexOfFirstAppliedArg() const;
  BRIDGED_INLINE bool PartialApplyInst_isOnStack() const;
  BRIDGED_INLINE bool PartialApplyInst_hasUnknownResultIsolation() const;
  BRIDGED_INLINE bool AllocStackInst_hasDynamicLifetime() const;
  BRIDGED_INLINE bool AllocStackInst_isFromVarDecl() const;
  BRIDGED_INLINE bool AllocStackInst_usesMoveableValueDebugInfo() const;
  BRIDGED_INLINE bool AllocStackInst_isLexical() const;
  BRIDGED_INLINE bool AllocBoxInst_hasDynamicLifetime() const;
  BRIDGED_INLINE bool AllocRefInstBase_isObjc() const;
  BRIDGED_INLINE bool AllocRefInstBase_canAllocOnStack() const;
  BRIDGED_INLINE CodiraInt AllocRefInstBase_getNumTailTypes() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedSILTypeArray AllocRefInstBase_getTailAllocatedTypes() const;
  BRIDGED_INLINE bool AllocRefDynamicInst_isDynamicTypeDeinitAndSizeKnownEquivalentToBaseType() const;
  BRIDGED_INLINE CodiraInt BeginApplyInst_numArguments() const;
  BRIDGED_INLINE bool BeginApplyInst_isCalleeAllocated() const;
  BRIDGED_INLINE CodiraInt TryApplyInst_numArguments() const;
  BRIDGED_INLINE BridgedArgumentConvention YieldInst_getConvention(BridgedOperand forOperand) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlock BranchInst_getTargetBlock() const;
  BRIDGED_INLINE CodiraInt SwitchEnumInst_getNumCases() const;
  BRIDGED_INLINE CodiraInt SwitchEnumInst_getCaseIndex(CodiraInt idx) const;
  BRIDGED_INLINE CodiraInt StoreInst_getStoreOwnership() const;
  BRIDGED_INLINE CodiraInt AssignInst_getAssignOwnership() const;
  BRIDGED_INLINE MarkDependenceKind MarkDependenceInst_dependenceKind() const;
  BRIDGED_INLINE void MarkDependenceInstruction_resolveToNonEscaping() const;
  BRIDGED_INLINE void MarkDependenceInstruction_settleToEscaping() const;
  BRIDGED_INLINE MarkDependenceKind MarkDependenceAddrInst_dependenceKind() const;
  BRIDGED_INLINE CodiraInt BeginAccessInst_getAccessKind() const;
  BRIDGED_INLINE bool BeginAccessInst_isStatic() const;
  BRIDGED_INLINE bool BeginAccessInst_isUnsafe() const;
  BRIDGED_INLINE void BeginAccess_setAccessKind(CodiraInt accessKind) const;
  BRIDGED_INLINE bool CopyAddrInst_isTakeOfSrc() const;
  BRIDGED_INLINE bool CopyAddrInst_isInitializationOfDest() const;
  BRIDGED_INLINE void CopyAddrInst_setIsTakeOfSrc(bool isTakeOfSrc) const;
  BRIDGED_INLINE void CopyAddrInst_setIsInitializationOfDest(bool isInitializationOfDest) const;
  BRIDGED_INLINE bool ExplicitCopyAddrInst_isTakeOfSrc() const;
  BRIDGED_INLINE bool ExplicitCopyAddrInst_isInitializationOfDest() const;
  BRIDGED_INLINE CodiraInt MarkUninitializedInst_getKind() const;
  BRIDGED_INLINE CodiraInt MarkUnresolvedNonCopyableValue_getCheckKind() const;
  BRIDGED_INLINE bool MarkUnresolvedNonCopyableValue_isStrict() const;
  BRIDGED_INLINE void RefCountingInst_setIsAtomic(bool isAtomic) const;
  BRIDGED_INLINE bool RefCountingInst_getIsAtomic() const;
  BRIDGED_INLINE CodiraInt CondBranchInst_getNumTrueArgs() const;
  BRIDGED_INLINE void AllocRefInstBase_setIsStackAllocatable() const;
  BRIDGED_INLINE bool AllocRefInst_isBare() const;
  BRIDGED_INLINE void AllocRefInst_setIsBare() const;
  BRIDGED_INLINE void TermInst_replaceBranchTarget(BridgedBasicBlock from, BridgedBasicBlock to) const;
  BRIDGED_INLINE CodiraInt KeyPathInst_getNumComponents() const;
  BRIDGED_INLINE void KeyPathInst_getReferencedFunctions(CodiraInt componentIdx, KeyPathFunctionResults * _Nonnull results) const;
  BRIDGED_INLINE void GlobalAddrInst_clearToken() const;
  BRIDGED_INLINE bool GlobalValueInst_isBare() const;
  BRIDGED_INLINE void GlobalValueInst_setIsBare() const;
  BRIDGED_INLINE void LoadInst_setOwnership(CodiraInt ownership) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType UnconditionalCheckedCast_getSourceFormalType() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType UnconditionalCheckedCast_getTargetFormalType() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE CheckedCastInstOptions
      UnconditionalCheckedCast_getCheckedCastOptions() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType UnconditionalCheckedCastAddr_getSourceFormalType() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType UnconditionalCheckedCastAddr_getTargetFormalType() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE CheckedCastInstOptions
      UnconditionalCheckedCastAddr_getCheckedCastOptions() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlock CheckedCastBranch_getSuccessBlock() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlock CheckedCastBranch_getFailureBlock() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE CheckedCastInstOptions
      CheckedCastBranch_getCheckedCastOptions() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType CheckedCastAddrBranch_getSourceFormalType() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType CheckedCastAddrBranch_getTargetFormalType() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlock CheckedCastAddrBranch_getSuccessBlock() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlock CheckedCastAddrBranch_getFailureBlock() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE CheckedCastInstOptions
      CheckedCastAddrBranch_getCheckedCastOptions() const;
  BRIDGED_INLINE void CheckedCastBranch_updateSourceFormalTypeFromOperandLoweredType() const;
  BRIDGED_INLINE CastConsumptionKind CheckedCastAddrBranch_getConsumptionKind() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedSubstitutionMap ApplySite_getSubstitutionMap() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType ApplySite_getSubstitutedCalleeType() const;
  BRIDGED_INLINE CodiraInt ApplySite_getNumArguments() const;
  BRIDGED_INLINE bool ApplySite_isCalleeNoReturn() const;
  BRIDGED_INLINE CodiraInt FullApplySite_numIndirectResultArguments() const;
  BRIDGED_INLINE bool ConvertFunctionInst_withoutActuallyEscaping() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType TypeValueInst_getParamType() const;

  // =========================================================================//
  //                   VarDeclInst and DebugVariableInst
  // =========================================================================//

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedDeclObj
  DebugValue_getDecl() const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedDeclObj
  AllocStack_getDecl() const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedDeclObj
  AllocBox_getDecl() const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedDeclObj
  GlobalAddr_getDecl() const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedDeclObj
  RefElementAddr_getDecl() const;

  BRIDGED_INLINE bool DebugValue_hasVarInfo() const;
  BRIDGED_INLINE BridgedSILDebugVariable DebugValue_getVarInfo() const;

  BRIDGED_INLINE bool AllocStack_hasVarInfo() const;
  BRIDGED_INLINE BridgedSILDebugVariable AllocStack_getVarInfo() const;

  BRIDGED_INLINE bool AllocBox_hasVarInfo() const;
  BRIDGED_INLINE BridgedSILDebugVariable AllocBox_getVarInfo() const;
};

struct OptionalBridgedInstruction {
  OptionalCodiraObject obj;

  BRIDGED_INLINE language::SILInstruction * _Nullable unbridged() const;

  OptionalBridgedInstruction() : obj(nullptr) {}
  OptionalBridgedInstruction(OptionalCodiraObject obj) : obj(obj) {}
};

struct BridgedArgument {
  CodiraObject obj;

  BRIDGED_INLINE language::SILArgument * _Nonnull getArgument() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlock getParent() const;
  BRIDGED_INLINE bool isReborrow() const;
  BRIDGED_INLINE bool FunctionArgument_isLexical() const;
  BRIDGED_INLINE bool FunctionArgument_isClosureCapture() const;
  BRIDGED_INLINE void setReborrow(bool reborrow) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedDeclObj getDecl() const;
  BRIDGED_INLINE void copyFlags(BridgedArgument fromArgument) const;
};

struct OptionalBridgedArgument {
  OptionalCodiraObject obj;

  BRIDGED_INLINE language::SILArgument * _Nullable unbridged() const;
};

struct OptionalBridgedBasicBlock {
  OptionalCodiraObject obj;

  BRIDGED_INLINE language::SILBasicBlock * _Nullable unbridged() const;
};

struct BridgedBasicBlock {
  CodiraObject obj;

  BridgedBasicBlock(CodiraObject obj) : obj(obj) {
  }

  BRIDGED_INLINE BridgedBasicBlock(language::SILBasicBlock * _Nonnull block);
  BRIDGED_INLINE language::SILBasicBlock * _Nonnull unbridged() const;

  BridgedOwnedString getDebugDescription() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedBasicBlock getNext() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedBasicBlock getPrevious() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedFunction getFunction() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedInstruction getFirstInst() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedInstruction getLastInst() const;
  BRIDGED_INLINE CodiraInt getNumArguments() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedArgument getArgument(CodiraInt index) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedArgument addBlockArgument(BridgedType type, BridgedValue::Ownership ownership) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedArgument addFunctionArgument(BridgedType type) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedArgument insertFunctionArgument(CodiraInt atPosition, BridgedType type,
                                                                            BridgedValue::Ownership ownership,
                                                                            OptionalBridgedDeclObj decl) const;
  BRIDGED_INLINE void eraseArgument(CodiraInt index) const;
  BRIDGED_INLINE void moveAllInstructionsToBegin(BridgedBasicBlock dest) const;
  BRIDGED_INLINE void moveAllInstructionsToEnd(BridgedBasicBlock dest) const;
  BRIDGED_INLINE void moveArgumentsTo(BridgedBasicBlock dest) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedSuccessor getFirstPred() const;
};

struct BridgedSuccessor {
  const language::SILSuccessor * _Nonnull succ;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedSuccessor getNext() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedBasicBlock getTargetBlock() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE  BridgedInstruction getContainingInst() const;
};

struct OptionalBridgedSuccessor {
  const language::SILSuccessor * _Nullable succ;

  // Assumes that `succ` is not null.
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedSuccessor advancedBy(CodiraInt index) const;
};

struct BridgedSuccessorArray {
  OptionalBridgedSuccessor base;
  CodiraInt count;
};

struct BridgedDeclRef {
  uint64_t storage[3];

  BRIDGED_INLINE BridgedDeclRef(language::SILDeclRef declRef);
  BRIDGED_INLINE language::SILDeclRef unbridged() const;

  BRIDGED_INLINE bool isEqualTo(BridgedDeclRef rhs) const;
  BridgedOwnedString getDebugDescription() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedLocation getLocation() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclObj getDecl() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDiagnosticArgument asDiagnosticArgument() const;
};

struct BridgedVTableEntry {
  uint64_t storage[5];

  enum class Kind {
    Normal,
    Inherited,
    Override
  };

  BridgedVTableEntry() : storage{0, 0, 0, 0, 0} {};
  BRIDGED_INLINE BridgedVTableEntry(const language::SILVTableEntry &entry);
  BRIDGED_INLINE const language::SILVTableEntry &unbridged() const;

  BridgedOwnedString getDebugDescription() const;
  BRIDGED_INLINE Kind getKind() const;
  BRIDGED_INLINE bool isNonOverridden() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclRef getMethodDecl() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedFunction getImplementation() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  static BridgedVTableEntry create(Kind kind, bool nonOverridden,
                                   BridgedDeclRef methodDecl, BridgedFunction implementation);
};

struct OptionalBridgedVTableEntry {
  BridgedVTableEntry entry;
  bool hasEntry = false;

  OptionalBridgedVTableEntry() {}
  OptionalBridgedVTableEntry(BridgedVTableEntry e) : entry(e), hasEntry(true) {}
};

struct BridgedVTableEntryArray {
  BridgedVTableEntry base;
  CodiraInt count;
};

struct BridgedVTable {
  language::SILVTable * _Nonnull vTable;

  BridgedOwnedString getDebugDescription() const;
  BRIDGED_INLINE CodiraInt getNumEntries() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedVTableEntry getEntry(CodiraInt index) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclObj getClass() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedVTableEntry lookupMethod(BridgedDeclRef member) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedType getSpecializedClassType() const;
  BRIDGED_INLINE void replaceEntries(BridgedArrayRef bridgedEntries) const;
};

struct OptionalBridgedVTable {
  language::SILVTable * _Nullable table;
};

struct BridgedConstExprFunctionState {
  language::ConstExprFunctionState * _Nonnull state;
  language::SymbolicValueBumpAllocator * _Nonnull allocator;
  language::ConstExprEvaluator * _Nonnull constantEvaluator;
  unsigned int * _Nonnull numEvaluatedSILInstructions;
  
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  static BridgedConstExprFunctionState create();
  
  BRIDGED_INLINE
  bool isConstantValue(BridgedValue value);
  
  BRIDGED_INLINE
  void deinitialize();
};

struct BridgedWitnessTableEntry {
  uint64_t storage[5];

  enum class Kind {
    invalid,
    method,
    associatedType,
    associatedConformance,
    baseProtocol
  };

#ifdef USED_IN_CPP_SOURCE
  static BridgedWitnessTableEntry bridge(const language::SILWitnessTable::Entry &entry) {
    BridgedWitnessTableEntry bridgedEntry;
    *reinterpret_cast<language::SILWitnessTable::Entry *>(&bridgedEntry.storage) = entry;
    return bridgedEntry;
  }

  const language::SILWitnessTable::Entry &unbridged() const {
    return *reinterpret_cast<const language::SILWitnessTable::Entry *>(&storage);
  }
#endif

  BridgedOwnedString getDebugDescription() const;
  BRIDGED_INLINE Kind getKind() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclRef getMethodRequirement() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE OptionalBridgedFunction getMethodWitness() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclObj getAssociatedTypeRequirement() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType getAssociatedTypeWitness() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedCanType getAssociatedConformanceRequirement() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedConformance getAssociatedConformanceWitness() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedDeclObj getBaseProtocolRequirement() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedConformance getBaseProtocolWitness() const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  static BridgedWitnessTableEntry createInvalid();
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  static BridgedWitnessTableEntry createMethod(BridgedDeclRef requirement,
                                               OptionalBridgedFunction witness);
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  static BridgedWitnessTableEntry createAssociatedType(BridgedDeclObj requirement,
                                                       BridgedCanType witness);
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  static BridgedWitnessTableEntry createAssociatedConformance(BridgedCanType requirement,
                                                              BridgedConformance witness);
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE
  static BridgedWitnessTableEntry createBaseProtocol(BridgedDeclObj requirement,
                                                     BridgedConformance witness);
};

struct BridgedWitnessTable {
  const language::SILWitnessTable * _Nonnull table;

  BridgedOwnedString getDebugDescription() const;
  BRIDGED_INLINE CodiraInt getNumEntries() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedWitnessTableEntry getEntry(CodiraInt index) const;
  BRIDGED_INLINE bool isDeclaration() const;
  BRIDGED_INLINE bool isSpecialized() const;
};

struct OptionalBridgedWitnessTable {
  const language::SILWitnessTable * _Nullable table;
};

struct BridgedDefaultWitnessTable {
  const language::SILDefaultWitnessTable * _Nonnull table;

  BridgedOwnedString getDebugDescription() const;
  BRIDGED_INLINE CodiraInt getNumEntries() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedWitnessTableEntry getEntry(CodiraInt index) const;
};

struct OptionalBridgedDefaultWitnessTable {
  const language::SILDefaultWitnessTable * _Nullable table;
};

struct BridgedBuilder{

  enum class InsertAt {
    beforeInst, endOfBlock, startOfFunction, intoGlobal
  } insertAt;

  CodiraObject insertionObj;
  BridgedLocation loc;

  BRIDGED_INLINE language::SILBuilder unbridged() const;
  BRIDGED_INLINE language::SILLocation regularLoc() const;
  BRIDGED_INLINE language::SILLocation returnLoc() const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createBuiltin(BridgedStringRef name,
                                                                      BridgedType type,
                                                                      BridgedSubstitutionMap subs,
                                                                      BridgedValueArray arguments) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createBuiltinBinaryFunction(BridgedStringRef name,
                                          BridgedType operandType, BridgedType resultType,
                                          BridgedValueArray arguments) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createCondFail(BridgedValue condition,
                                                                       BridgedStringRef message) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createIntegerLiteral(BridgedType type, CodiraInt value) const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createAllocRef(BridgedType type,
    bool objc, bool canAllocOnStack, bool isBare,
    BridgedSILTypeArray elementTypes, BridgedValueArray elementCountOperands) const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction
  createAllocStack(BridgedType type, BridgedSILDebugVariable debugVar,
                   bool hasDynamicLifetime, bool isLexical, bool isFromVarDecl, bool wasMoved) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction
  createAllocStack(BridgedType type, bool hasDynamicLifetime, bool isLexical, bool isFromVarDecl, bool wasMoved) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createAllocVector(BridgedValue capacity,
                                                                          BridgedType type) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createDeallocStack(BridgedValue operand) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createDeallocStackRef(BridgedValue operand) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createAddressToPointer(BridgedValue address,
                                                                               BridgedType pointerTy,
                                                                               bool needsStackProtection) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createPointerToAddress(BridgedValue pointer,
                                                                               BridgedType addressTy,
                                                                               bool isStrict,
                                                                               bool isInvariant,
                                                                               uint64_t alignment) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createIndexAddr(BridgedValue base,
                                                                        BridgedValue index,
                                                                        bool needsStackProtection) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createUncheckedRefCast(BridgedValue op,
                                                                               BridgedType type) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createUncheckedAddrCast(BridgedValue op,
                                                                                BridgedType type) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createUpcast(BridgedValue op, BridgedType type) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createCheckedCastAddrBranch(
      BridgedValue source, BridgedCanType sourceFormalType,
      BridgedValue destination, BridgedCanType targetFormalType,
      BridgedInstruction::CheckedCastInstOptions options,
      BridgedInstruction::CastConsumptionKind consumptionKind,
      BridgedBasicBlock successBlock, BridgedBasicBlock failureBlock) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createUnconditionalCheckedCastAddr(
        BridgedInstruction::CheckedCastInstOptions options,
        BridgedValue source, BridgedCanType sourceFormalType,
        BridgedValue destination, BridgedCanType targetFormalType) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createUncheckedOwnershipConversion(
        BridgedValue op, BridgedValue::Ownership ownership) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createLoad(BridgedValue op, CodiraInt ownership) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createLoadBorrow(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createBeginDeallocRef(BridgedValue reference,
                                                                              BridgedValue allocation) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createEndInitLetRef(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction
  createRetainValue(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction
  createReleaseValue(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createStrongRetain(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createStrongRelease(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createUnownedRetain(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createUnownedRelease(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createFunctionRef(BridgedFunction function) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createCopyValue(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createBeginBorrow(BridgedValue op,
                                                                          bool isLexical,
                                                                          bool hasPointerEscape,
                                                                          bool isFromVarDecl) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createBorrowedFrom(BridgedValue borrowedValue,
                                                                           BridgedValueArray enclosingValues) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createEndBorrow(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createCopyAddr(BridgedValue from, BridgedValue to,
                                          bool takeSource, bool initializeDest) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createDestroyValue(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createDestroyAddr(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createEndLifetime(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createDebugValue(BridgedValue src,
                                                                         BridgedSILDebugVariable var) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createDebugStep() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createApply(BridgedValue function,
                                          BridgedSubstitutionMap subMap,
                                          BridgedValueArray arguments, bool isNonThrowing, bool isNonAsync,
                                          BridgedGenericSpecializationInformation specInfo) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createTryApply(BridgedValue function,
                                          BridgedSubstitutionMap subMap,
                                          BridgedValueArray arguments,
                                          BridgedBasicBlock normalBB, BridgedBasicBlock errorBB,
                                          bool isNonAsync,
                                          BridgedGenericSpecializationInformation specInfo) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createBeginApply(BridgedValue function,
                                          BridgedSubstitutionMap subMap,
                                          BridgedValueArray arguments, bool isNonThrowing, bool isNonAsync,
                                          BridgedGenericSpecializationInformation specInfo) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createWitnessMethod(BridgedCanType lookupType,
                                          BridgedConformance conformance,
                                          BridgedDeclRef member, BridgedType methodType) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createReturn(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createThrow(BridgedValue op) const;
  LANGUAGE_IMPORT_UNSAFE BridgedInstruction createSwitchEnumInst(BridgedValue enumVal,
                                          OptionalBridgedBasicBlock defaultBlock,
                                          const void * _Nullable enumCases, CodiraInt numEnumCases) const;
  LANGUAGE_IMPORT_UNSAFE BridgedInstruction createSwitchEnumAddrInst(BridgedValue enumAddr,
                                          OptionalBridgedBasicBlock defaultBlock,
                                          const void * _Nullable enumCases, CodiraInt numEnumCases) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createUncheckedEnumData(BridgedValue enumVal,
                                          CodiraInt caseIdx, BridgedType resultType) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createUncheckedTakeEnumDataAddr(BridgedValue enumAddr,
                                                                                        CodiraInt caseIdx) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createInitEnumDataAddr(BridgedValue enumAddr,
                                                                               CodiraInt caseIdx,
                                                                               BridgedType type) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createEnum(CodiraInt caseIdx, OptionalBridgedValue payload,
                                          BridgedType resultType) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createThinToThickFunction(BridgedValue fn,
                                                                                  BridgedType resultType) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createPartialApply(BridgedValue fn,
                                                                           BridgedValueArray bridgedCapturedArgs,
                                                                           BridgedArgumentConvention calleeConvention,
                                                                           BridgedSubstitutionMap bridgedSubstitutionMap = BridgedSubstitutionMap(),
                                                                           bool hasUnknownIsolation = true,
                                                                           bool isOnStack = false) const;                                                                                  
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createBranch(BridgedBasicBlock destBlock,
                                                                     BridgedValueArray arguments) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createUnreachable() const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createObject(BridgedType type, BridgedValueArray arguments,
                                                                     CodiraInt numBaseElements) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createVector(BridgedValueArray arguments) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createVectorBaseAddr(BridgedValue vector) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createGlobalAddr(BridgedGlobalVar global,
                                                                         OptionalBridgedValue dependencyToken) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createGlobalValue(BridgedGlobalVar global,
                                                                          bool isBare) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createStruct(BridgedType type,
                                                                     BridgedValueArray elements) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createStructExtract(BridgedValue str,
                                                                            CodiraInt fieldIndex) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createStructElementAddr(BridgedValue addr,
                                                                                CodiraInt fieldIndex) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createDestructureStruct(BridgedValue str) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createTuple(BridgedType type,
                                                                    BridgedValueArray elements) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createTupleExtract(BridgedValue str,
                                                                           CodiraInt elementIndex) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createTupleElementAddr(BridgedValue addr,
                                                                               CodiraInt elementIndex) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createDestructureTuple(BridgedValue str) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createProjectBox(BridgedValue box, CodiraInt fieldIdx) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createStore(BridgedValue src, BridgedValue dst,
                                          CodiraInt ownership) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createInitExistentialRef(BridgedValue instance,
                                          BridgedType type,
                                          BridgedCanType formalConcreteType,
                                          BridgedConformanceArray conformances) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createInitExistentialMetatype(BridgedValue metatype,
                                          BridgedType existentialType,
                                          BridgedConformanceArray conformances) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createMetatype(BridgedCanType instanceType,
                                          BridgedASTType::MetatypeRepresentation representation) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createEndCOWMutation(BridgedValue instance,
                                                                             bool keepUnique) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createEndCOWMutationAddr(BridgedValue instance) const;                                                                             
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createMarkDependence(
    BridgedValue value, BridgedValue base, BridgedInstruction::MarkDependenceKind dependenceKind) const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createMarkDependenceAddr(
    BridgedValue value, BridgedValue base, BridgedInstruction::MarkDependenceKind dependenceKind) const;
    
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createMarkUninitialized(
    BridgedValue value, CodiraInt kind) const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createMarkUnresolvedNonCopyableValue(
    BridgedValue value, CodiraInt checkKind, bool isStrict) const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createEndAccess(BridgedValue value) const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createEndApply(BridgedValue value) const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createAbortApply(BridgedValue value) const;

  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createConvertFunction(BridgedValue originalFunction, BridgedType resultType, bool withoutActuallyEscaping) const;
  LANGUAGE_IMPORT_UNSAFE BRIDGED_INLINE BridgedInstruction createConvertEscapeToNoEscape(BridgedValue originalFunction, BridgedType resultType, bool isLifetimeGuaranteed) const;

  LANGUAGE_IMPORT_UNSAFE void destroyCapturedArgs(BridgedInstruction partialApply) const;
};

// Passmanager and Context

namespace language {
  class CodiraPassInvocation;
}

struct BridgedChangeNotificationHandler {
  language::CodiraPassInvocation * _Nonnull invocation;

  enum class Kind {
    instructionsChanged,
    callsChanged,
    branchesChanged,
    effectsChanged,
    functionTablesChanged
  };

  void notifyChanges(Kind changeKind) const;
};

namespace language::test {
struct Arguments;
class FunctionTest;
} // namespace language::test

struct BridgedFunctionTest {
  language::test::FunctionTest *_Nonnull test;
};

struct BridgedTestArguments {
  language::test::Arguments *_Nonnull arguments;

  bool hasUntaken() const;
  LANGUAGE_IMPORT_UNSAFE BridgedStringRef takeString() const;
  bool takeBool() const;
  CodiraInt takeInt() const;
  LANGUAGE_IMPORT_UNSAFE BridgedOperand takeOperand() const;
  LANGUAGE_IMPORT_UNSAFE BridgedValue takeValue() const;
  LANGUAGE_IMPORT_UNSAFE BridgedInstruction takeInstruction() const;
  LANGUAGE_IMPORT_UNSAFE BridgedArgument takeArgument() const;
  LANGUAGE_IMPORT_UNSAFE BridgedBasicBlock takeBlock() const;
  LANGUAGE_IMPORT_UNSAFE BridgedFunction takeFunction() const;
};

struct BridgedCodiraPassInvocation {
  language::CodiraPassInvocation *_Nonnull invocation;
};

using CodiraNativeFunctionTestThunk =
    void (*_Nonnull)(void *_Nonnull, BridgedFunction, BridgedTestArguments,
                     BridgedCodiraPassInvocation);

void registerFunctionTestThunk(CodiraNativeFunctionTestThunk);

void registerFunctionTest(BridgedStringRef,
                          void *_Nonnull nativeCodiraInvocation);

LANGUAGE_END_NULLABILITY_ANNOTATIONS

#ifndef PURE_BRIDGING_MODE
// In _not_ PURE_BRIDGING_MODE, bridging functions are inlined and therefore
// included in the header file. This is because they rely on C++ headers that
// we don't want to pull in when using "pure bridging mode".
#include "SILBridgingImpl.h"
#endif

#endif
