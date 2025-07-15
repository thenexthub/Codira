//===--- IRGenFunction.h - IR Generation for Codira Functions ----*- C++ -*-===//
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
// This file defines the structure used to generate the IR body of a
// function.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_IRGENFUNCTION_H
#define LANGUAGE_IRGEN_IRGENFUNCTION_H

#include "DominancePoint.h"
#include "GenPack.h"
#include "IRBuilder.h"
#include "LocalTypeDataKind.h"
#include "language/AST/ReferenceCounting.h"
#include "language/AST/Type.h"
#include "language/Basic/Toolchain.h"
#include "language/SIL/SILLocation.h"
#include "language/SIL/SILType.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/IR/CallingConv.h"

namespace toolchain {
  class AllocaInst;
  class CallSite;
  class Constant;
  class Function;
}

namespace language {
  class ArchetypeType;
  class IRGenOptions;
  class SILDebugScope;
  class SILType;
  class SourceLoc;
  enum class MetadataState : size_t;
  enum class CoroAllocatorKind : uint8_t;

  namespace Lowering {
  class TypeConverter;
  }

namespace irgen {
  class DynamicMetadataRequest;
  class Explosion;
  class FunctionRef;
  class HeapLayout;
  class HeapNonFixedOffsets;
  class IRGenModule;
  class LinkEntity;
  class LocalTypeDataCache;
  class MetadataDependencyCollector;
  class MetadataResponse;
  class Scope;
  class TypeInfo;
  enum class ValueWitness : unsigned;

/// IRGenFunction - Primary class for emitting LLVM instructions for a
/// specific function.
class IRGenFunction {
public:
  IRGenModule &IGM;
  IRBuilder Builder;

  /// If != OptimizationMode::NotSet, the optimization mode specified with an
  /// function attribute.
  OptimizationMode OptMode;
  bool isPerformanceConstraint;

  // Destination basic blocks for condfail traps.
  toolchain::SmallVector<toolchain::BasicBlock *, 8> FailBBs;

  toolchain::Function *const CurFn;
  ModuleDecl *getCodiraModule() const;
  SILModule &getSILModule() const;
  Lowering::TypeConverter &getSILTypes() const;
  const IRGenOptions &getOptions() const;

  IRGenFunction(IRGenModule &IGM, toolchain::Function *fn,
                bool isPerformanceConstraint = false,
                OptimizationMode Mode = OptimizationMode::NotSet,
                const SILDebugScope *DbgScope = nullptr,
                std::optional<SILLocation> DbgLoc = std::nullopt);
  ~IRGenFunction();

  void unimplemented(SourceLoc Loc, StringRef Message);

  friend class Scope;

  Address createErrorResultSlot(SILType errorType, bool isAsync, bool setCodiraErrorFlag = true, bool isTypedError = false);

  //--- Function prologue and epilogue
  //-------------------------------------------
public:
  Explosion collectParameters();
  void emitScalarReturn(SILType returnResultType, SILType funcResultType,
                        Explosion &scalars, bool isCodiraCCReturn,
                        bool isOutlined, bool mayPeepholeLoad = false,
                        SILType errorType = {});
  void emitScalarReturn(toolchain::Type *resultTy, Explosion &scalars);
  
  void emitBBForReturn();
  bool emitBranchToReturnBB();

  toolchain::BasicBlock *createExceptionUnwindBlock();

  void setCallsThunksWithForeignExceptionTraps() {
    callsAnyAlwaysInlineThunksWithForeignExceptionTraps = true;
  }

  void createExceptionTrapScope(
      toolchain::function_ref<void(toolchain::BasicBlock *, toolchain::BasicBlock *)>
          invokeEmitter);

  void emitAllExtractValues(toolchain::Value *aggValue, toolchain::StructType *type,
                            Explosion &out);

  /// Return the error result slot to be passed to the callee, given an error
  /// type.  There's always only one error type.
  ///
  /// For async functions, this is different from the caller result slot because
  /// that is a gep into the %language.context.
  Address getCalleeErrorResultSlot(SILType errorType,
                                   bool isTypedError);

  /// Return the error result slot provided by the caller.
  Address getCallerErrorResultSlot();

  /// Set the error result slot for the current function.
  void setCallerErrorResultSlot(Address address);
  /// Set the error result slot for a typed throw for the current function.
  void setCallerTypedErrorResultSlot(Address address);

  Address getCallerTypedErrorResultSlot();
  Address getCalleeTypedErrorResultSlot(SILType errorType);
  void setCalleeTypedErrorResultSlot(Address addr);

  toolchain::ConstantInt* getMallocTypeId();

  /// Are we currently emitting a coroutine?
  bool isCoroutine() {
    return CoroutineHandle != nullptr;
  }
  toolchain::Value *getCoroutineHandle() {
    assert(isCoroutine());
    return CoroutineHandle;
  }
  bool isCalleeAllocatedCoroutine() { return CoroutineAllocator != nullptr; }
  toolchain::Value *getCoroutineAllocator() {
    assert(isCoroutine());
    return CoroutineAllocator;
  }

  void setCoroutineHandle(toolchain::Value *handle) {
    assert(CoroutineHandle == nullptr && "already set handle");
    assert(handle != nullptr && "setting a null handle");
    CoroutineHandle = handle;
  }

  void setCoroutineAllocator(toolchain::Value *allocator) {
    assert(CoroutineAllocator == nullptr && "already set allocator");
    assert(allocator != nullptr && "setting a null allocator");
    CoroutineAllocator = allocator;
  }

  std::optional<CoroAllocatorKind> getDefaultCoroutineAllocatorKind();

  toolchain::BasicBlock *getCoroutineExitBlock() const {
    return CoroutineExitBlock;
  }

  SmallVector<toolchain::Value *, 1> coroutineResults;
  
  void setCoroutineExitBlock(toolchain::BasicBlock *block) {
    assert(CoroutineExitBlock == nullptr && "already set exit BB");
    assert(block != nullptr && "setting a null exit BB");
    CoroutineExitBlock = block;
  }

  toolchain::Value *getAsyncTask();
  toolchain::Value *getAsyncContext();
  void storeCurrentAsyncContext(toolchain::Value *context);

  toolchain::CallInst *emitSuspendAsyncCall(unsigned languageAsyncContextIndex,
                                       toolchain::StructType *resultTy,
                                       ArrayRef<toolchain::Value *> args,
                                       bool restoreCurrentContext = true);

  toolchain::Value *emitAsyncResumeProjectContext(toolchain::Value *callerContextAddr);
  toolchain::Function *getOrCreateResumePrjFn();
  toolchain::Value *popAsyncContext(toolchain::Value *calleeContext);
  toolchain::Function *createAsyncDispatchFn(const FunctionPointer &fnPtr,
                                        ArrayRef<toolchain::Value *> args);
  toolchain::Function *createAsyncDispatchFn(const FunctionPointer &fnPtr,
                                        ArrayRef<toolchain::Type *> argTypes);

  void emitGetAsyncContinuation(SILType resumeTy,
                                StackAddress optionalResultAddr,
                                Explosion &out,
                                bool canThrow);

  void emitAwaitAsyncContinuation(SILType resumeTy,
                                  bool isIndirectResult,
                                  Explosion &outDirectResult,
                                  toolchain::BasicBlock *&normalBB,
                                  toolchain::PHINode *&optionalErrorPhi,
                                  toolchain::BasicBlock *&optionalErrorBB);

  void emitResumeAsyncContinuationReturning(toolchain::Value *continuation,
                                            toolchain::Value *srcPtr,
                                            SILType valueTy,
                                            bool throwing);

  void emitResumeAsyncContinuationThrowing(toolchain::Value *continuation,
                                           toolchain::Value *error);

  void emitClearSensitive(Address address, toolchain::Value *size);

  FunctionPointer
  getFunctionPointerForResumeIntrinsic(toolchain::Value *resumeIntrinsic);

  void emitSuspensionPoint(Explosion &executor, toolchain::Value *asyncResume);
  toolchain::Function *getOrCreateResumeFromSuspensionFn();
  toolchain::Function *createAsyncSuspendFn();

private:
  void emitPrologue();
  void emitEpilogue();

  Address ReturnSlot;
  toolchain::BasicBlock *ReturnBB;
  Address CalleeErrorResultSlot;
  Address AsyncCalleeErrorResultSlot;
  Address CallerErrorResultSlot;
  Address CallerTypedErrorResultSlot;
  Address CalleeTypedErrorResultSlot;
  toolchain::Value *CoroutineHandle = nullptr;
  toolchain::Value *CoroutineAllocator = nullptr;
  toolchain::Value *AsyncCoroutineCurrentResume = nullptr;
  toolchain::Value *AsyncCoroutineCurrentContinuationContext = nullptr;

protected:
  // Whether pack metadata stack promotion is disabled for this function in
  // particular.
  bool packMetadataStackPromotionDisabled = false;

  /// The on-stack pack metadata allocations emitted so far awaiting cleanup.
  toolchain::SmallSetVector<StackPackAlloc, 2> OutstandingStackPackAllocs;

private:
  Address asyncContextLocation;

  /// The unique block that calls @toolchain.coro.end.
  toolchain::BasicBlock *CoroutineExitBlock = nullptr;

  /// The blocks that handle thrown exceptions from all throwing foreign calls
  /// in this function.
  toolchain::SmallVector<toolchain::BasicBlock *, 4> ExceptionUnwindBlocks;

  /// True if this function calls any always inline thunks that have a foreign
  /// exception trap.
  bool callsAnyAlwaysInlineThunksWithForeignExceptionTraps = false;

public:
  void emitCoroutineOrAsyncExit(bool isUnwind);

//--- Helper methods -----------------------------------------------------------
public:

  /// Returns the optimization mode for the function. If no mode is set for the
  /// function, returns the global mode, i.e. the mode in IRGenOptions.
  OptimizationMode getEffectiveOptimizationMode() const;

  /// Returns true if this function should be optimized for size.
  bool optimizeForSize() const {
    return getEffectiveOptimizationMode() == OptimizationMode::ForSize;
  }

  /// Whether metadata/wtable packs allocated on the stack must be eagerly
  /// heapified.
  bool canStackPromotePackMetadata() const;

  bool outliningCanCallValueWitnesses() const;

  void setupAsync(unsigned asyncContextIndex);
  bool isAsync() const { return asyncContextLocation.isValid(); }

  Address createAlloca(toolchain::Type *ty, Alignment align,
                       const toolchain::Twine &name = "");
  Address createAlloca(toolchain::Type *ty, toolchain::Value *arraySize, Alignment align,
                       const toolchain::Twine &name = "");

  StackAddress emitDynamicAlloca(SILType type, const toolchain::Twine &name = "");
  StackAddress emitDynamicAlloca(toolchain::Type *eltTy, toolchain::Value *arraySize,
                                 Alignment align, bool allowTaskAlloc = true,
                                 const toolchain::Twine &name = "");
  void emitDeallocateDynamicAlloca(StackAddress address,
                                   bool allowTaskDealloc = true,
                                   bool useTaskDeallocThrough = false);

  toolchain::BasicBlock *createBasicBlock(const toolchain::Twine &Name);
  const TypeInfo &getTypeInfoForUnlowered(Type subst);
  const TypeInfo &getTypeInfoForUnlowered(AbstractionPattern orig, Type subst);
  const TypeInfo &getTypeInfoForUnlowered(AbstractionPattern orig,
                                          CanType subst);
  const TypeInfo &getTypeInfoForLowered(CanType T);
  const TypeInfo &getTypeInfo(SILType T);
  void emitMemCpy(toolchain::Value *dest, toolchain::Value *src,
                  Size size, Alignment align);
  void emitMemCpy(toolchain::Value *dest, toolchain::Value *src,
                  toolchain::Value *size, Alignment align);
  void emitMemCpy(Address dest, Address src, Size size);
  void emitMemCpy(Address dest, Address src, toolchain::Value *size);

  toolchain::Value *emitByteOffsetGEP(toolchain::Value *base, toolchain::Value *offset,
                                 toolchain::Type *objectType,
                                 const toolchain::Twine &name = "");
  Address emitByteOffsetGEP(toolchain::Value *base, toolchain::Value *offset,
                            const TypeInfo &type,
                            const toolchain::Twine &name = "");
  Address emitAddressAtOffset(toolchain::Value *base, Offset offset,
                              toolchain::Type *objectType,
                              Alignment objectAlignment,
                              const toolchain::Twine &name = "");

  toolchain::Value *emitInvariantLoad(Address address,
                                 const toolchain::Twine &name = "");

  void emitStoreOfRelativeIndirectablePointer(toolchain::Value *value,
                                              Address addr,
                                              bool isFar);

  toolchain::Value *emitLoadOfRelativePointer(Address addr, bool isFar,
                                         toolchain::Type *expectedPointedToType,
                                         const toolchain::Twine &name = "");
  toolchain::Value *
  emitLoadOfCompactFunctionPointer(Address addr, bool isFar,
                                   toolchain::Type *expectedPointedToType,
                                   const toolchain::Twine &name = "");

  toolchain::Value *emitAllocObjectCall(toolchain::Value *metadata, toolchain::Value *size,
                                   toolchain::Value *alignMask,
                                   const toolchain::Twine &name = "");
  toolchain::Value *emitInitStackObjectCall(toolchain::Value *metadata,
                                       toolchain::Value *object,
                                       const toolchain::Twine &name = "");
  toolchain::Value *emitInitStaticObjectCall(toolchain::Value *metadata,
                                        toolchain::Value *object,
                                        const toolchain::Twine &name = "");
  toolchain::Value *emitVerifyEndOfLifetimeCall(toolchain::Value *object,
                                           const toolchain::Twine &name = "");
  toolchain::Value *emitAllocRawCall(toolchain::Value *size, toolchain::Value *alignMask,
                                const toolchain::Twine &name ="");
  void emitDeallocRawCall(toolchain::Value *pointer, toolchain::Value *size,
                          toolchain::Value *alignMask);
  
  void emitAllocBoxCall(toolchain::Value *typeMetadata,
                         toolchain::Value *&box,
                         toolchain::Value *&valueAddress);

  void emitMakeBoxUniqueCall(toolchain::Value *box, toolchain::Value *typeMetadata,
                             toolchain::Value *alignMask, toolchain::Value *&outBox,
                             toolchain::Value *&outValueAddress);

  void emitDeallocBoxCall(toolchain::Value *box, toolchain::Value *typeMetadata);

  void emitTSanInoutAccessCall(toolchain::Value *address);

  toolchain::Value *emitTargetOSVersionAtLeastCall(toolchain::Value *major,
                                              toolchain::Value *minor,
                                              toolchain::Value *patch);

  toolchain::Value *emitTargetVariantOSVersionAtLeastCall(toolchain::Value *major,
                                                     toolchain::Value *minor,
                                                     toolchain::Value *patch);

  toolchain::Value *emitTargetOSVersionOrVariantOSVersionAtLeastCall(
      toolchain::Value *major, toolchain::Value *minor, toolchain::Value *patch,
      toolchain::Value *variantMajor, toolchain::Value *variantMinor,
      toolchain::Value *variantPatch);

  toolchain::Value *emitProjectBoxCall(toolchain::Value *box, toolchain::Value *typeMetadata);

  toolchain::Value *emitAllocEmptyBoxCall();

  // Emit a call to the given generic type metadata access function.
  MetadataResponse emitGenericTypeMetadataAccessFunctionCall(
                                          toolchain::Function *accessFunction,
                                          ArrayRef<toolchain::Value *> args,
                                          DynamicMetadataRequest request);

  // Emit a reference to the canonical type metadata record for the given AST
  // type. This can be used to identify the type at runtime. For types with
  // abstraction difference, the metadata contains the layout information for
  // values in the maximally-abstracted representation of the type; this is
  // correct for all uses of reabstractable values in opaque contexts.
  toolchain::Value *emitTypeMetadataRef(CanType type);

  /// Emit a reference to the canonical type metadata record for the given
  /// formal type.  The metadata is only required to be abstract; that is,
  /// you cannot use the result for layout.
  toolchain::Value *emitAbstractTypeMetadataRef(CanType type);

  MetadataResponse emitTypeMetadataRef(CanType type,
                                       DynamicMetadataRequest request);

  // Emit a reference to a metadata object that can be used for layout, but
  // cannot be used to identify a type. This will produce a layout appropriate
  // to the abstraction level of the given type. It may be able to avoid runtime
  // calls if there is a standard metadata object with the correct layout for
  // the type.
  //
  // TODO: It might be better to return just a value witness table reference
  // here, since for some types it's easier to get a shared reference to one
  // than a metadata reference, and it would be more type-safe.
  toolchain::Value *emitTypeMetadataRefForLayout(SILType type);
  toolchain::Value *emitTypeMetadataRefForLayout(SILType type,
                                            DynamicMetadataRequest request);

  toolchain::Value *emitValueGenericRef(CanType type);

  toolchain::Value *emitValueWitnessTableRef(SILType type,
                                        toolchain::Value **metadataSlot = nullptr);
  toolchain::Value *emitValueWitnessTableRef(SILType type,
                                        DynamicMetadataRequest request,
                                        toolchain::Value **metadataSlot = nullptr);
  toolchain::Value *emitValueWitnessTableRefForMetadata(toolchain::Value *metadata);
  
  toolchain::Value *emitValueWitnessValue(SILType type, ValueWitness index);
  FunctionPointer emitValueWitnessFunctionRef(SILType type,
                                              toolchain::Value *&metadataSlot,
                                              ValueWitness index);

  void emitInitializeFieldOffsetVector(SILType T,
                                       toolchain::Value *metadata,
                                       bool isVWTMutable,
                                       MetadataDependencyCollector *collector);


  toolchain::Value *optionallyLoadFromConditionalProtocolWitnessTable(
    toolchain::Value *wtable);

  toolchain::Value *emitPackShapeExpression(CanType type);

  void recordStackPackMetadataAlloc(StackAddress addr, toolchain::Value *shape);
  void eraseStackPackMetadataAlloc(StackAddress addr, toolchain::Value *shape);

  void recordStackPackWitnessTableAlloc(StackAddress addr, toolchain::Value *shape);
  void eraseStackPackWitnessTableAlloc(StackAddress addr, toolchain::Value *shape);

  void withLocalStackPackAllocs(toolchain::function_ref<void()> fn);

  /// Emit a load of a reference to the given Objective-C selector.
  toolchain::Value *emitObjCSelectorRefLoad(StringRef selector);

  /// Return the SILDebugScope for this function.
  const SILDebugScope *getDebugScope() const { return DbgScope; }
  toolchain::Value *coerceValue(toolchain::Value *value, toolchain::Type *toTy,
                           const toolchain::DataLayout &);
  Explosion coerceValueTo(SILType fromTy, Explosion &from, SILType toTy);

  /// Mark a load as invariant.
  void setInvariantLoad(toolchain::LoadInst *load);
  /// Mark a load as dereferenceable to `size` bytes.
  void setDereferenceableLoad(toolchain::LoadInst *load, unsigned size);

  /// Emit a non-mergeable trap call, optionally followed by a terminator.
  void emitTrap(StringRef failureMessage, bool EmitUnreachable);

  void emitConditionalTrap(toolchain::Value *condition, StringRef failureMessage,
                           const SILDebugScope *debugScope = nullptr);

  /// Given at least a src address to a list of elements, runs body over each
  /// element passing its address. An optional destination address can be
  /// provided which this will run over as well to perform things like
  /// 'initWithTake' from src to dest.
  void emitLoopOverElements(const TypeInfo &elementTypeInfo,
                            SILType elementType, CanType countType,
                            Address dest, Address src,
                          std::function<void (Address dest, Address src)> body);

private:
  toolchain::Instruction *AllocaIP;
  const SILDebugScope *DbgScope;
  /// The insertion point where we should but instructions we would normally put
  /// at the beginning of the function. LLVM's coroutine lowering really does
  /// not like it if we put instructions with side-effectrs before the
  /// coro.begin.
  toolchain::Instruction *EarliestIP;

public:
  void setEarliestInsertionPoint(toolchain::Instruction *inst) { EarliestIP = inst; }
  /// Returns the first insertion point before which we should insert
  /// instructions which have side-effects.
  toolchain::Instruction *getEarliestInsertionPoint() const { return EarliestIP; }

  //--- Reference-counting methods
  //-----------------------------------------------
public:
  // Returns the default atomicity of the module.
  Atomicity getDefaultAtomicity();

  toolchain::Value *emitUnmanagedAlloc(const HeapLayout &layout,
                                  const toolchain::Twine &name,
                                  toolchain::Constant *captureDescriptor,
                                  const HeapNonFixedOffsets *offsets = 0);

  // Functions that don't care about the reference-counting style.
  void emitFixLifetime(toolchain::Value *value);

  // Routines that are generic over the reference-counting style:
  //   - strong references
  void emitStrongRetain(toolchain::Value *value, ReferenceCounting refcounting,
                        Atomicity atomicity);
  void emitStrongRelease(toolchain::Value *value, ReferenceCounting refcounting,
                         Atomicity atomicity);
  toolchain::Value *emitLoadRefcountedPtr(Address addr, ReferenceCounting style);

  toolchain::Value *getReferenceStorageExtraInhabitantIndex(Address src,
                                                   ReferenceOwnership ownership,
                                                   ReferenceCounting style);
  void storeReferenceStorageExtraInhabitant(toolchain::Value *index,
                                            Address dest,
                                            ReferenceOwnership ownership,
                                            ReferenceCounting style);

#define NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Kind) \
  void emit##Kind##Name##Init(toolchain::Value *val, Address dest); \
  void emit##Kind##Name##Assign(toolchain::Value *value, Address dest); \
  void emit##Kind##Name##CopyInit(Address destAddr, Address srcAddr); \
  void emit##Kind##Name##TakeInit(Address destAddr, Address srcAddr); \
  void emit##Kind##Name##CopyAssign(Address destAddr, Address srcAddr); \
  void emit##Kind##Name##TakeAssign(Address destAddr, Address srcAddr); \
  toolchain::Value *emit##Kind##Name##LoadStrong(Address src, \
                                            toolchain::Type *resultType); \
  toolchain::Value *emit##Kind##Name##TakeStrong(Address src, \
                                            toolchain::Type *resultType); \
  void emit##Kind##Name##Destroy(Address addr);
#define ALWAYS_OR_SOMETIMES_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Kind) \
  void emit##Kind##Name##Retain(toolchain::Value *value, Atomicity atomicity); \
  void emit##Kind##Name##Release(toolchain::Value *value, Atomicity atomicity); \
  void emit##Kind##StrongRetain##Name(toolchain::Value *value, Atomicity atomicity);\
  void emit##Kind##StrongRetainAnd##Name##Release(toolchain::Value *value, \
                                                  Atomicity atomicity);
#define NEVER_LOADABLE_CHECKED_REF_STORAGE(Name, ...) \
  NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Native) \
  NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Unknown) \
  void emit##Name##Init(toolchain::Value *val, Address dest, ReferenceCounting style); \
  void emit##Name##Assign(toolchain::Value *value, Address dest, \
                          ReferenceCounting style); \
  void emit##Name##CopyInit(Address destAddr, Address srcAddr, \
                            ReferenceCounting style); \
  void emit##Name##TakeInit(Address destAddr, Address srcAddr, \
                            ReferenceCounting style); \
  void emit##Name##CopyAssign(Address destAddr, Address srcAddr, \
                              ReferenceCounting style); \
  void emit##Name##TakeAssign(Address destAddr, Address srcAddr, \
                              ReferenceCounting style); \
  toolchain::Value *emit##Name##LoadStrong(Address src, toolchain::Type *resultType, \
                                      ReferenceCounting style); \
  toolchain::Value *emit##Name##TakeStrong(Address src, toolchain::Type *resultType, \
                                      ReferenceCounting style); \
  void emit##Name##Destroy(Address addr, ReferenceCounting style);
#define SOMETIMES_LOADABLE_CHECKED_REF_STORAGE(Name, ...) \
  NEVER_LOADABLE_CHECKED_REF_STORAGE(Name, "...") \
  ALWAYS_OR_SOMETIMES_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Native) \
  ALWAYS_OR_SOMETIMES_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Unknown) \
  void emit##Name##Retain(toolchain::Value *value, ReferenceCounting style, \
                         Atomicity atomicity); \
  void emit##Name##Release(toolchain::Value *value, ReferenceCounting style, \
                          Atomicity atomicity); \
  void emitStrongRetain##Name(toolchain::Value *value, ReferenceCounting style, \
                              Atomicity atomicity); \
  void emitStrongRetainAnd##Name##Release(toolchain::Value *value, \
                                          ReferenceCounting style, \
                                          Atomicity atomicity);
#define ALWAYS_LOADABLE_CHECKED_REF_STORAGE(Name, ...) \
  ALWAYS_OR_SOMETIMES_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Native) \
  void emit##Name##Retain(toolchain::Value *value, ReferenceCounting style, \
                         Atomicity atomicity) { \
    assert(style == ReferenceCounting::Native); \
    emitNative##Name##Retain(value, atomicity); \
  } \
  void emit##Name##Release(toolchain::Value *value, ReferenceCounting style, \
                          Atomicity atomicity) { \
    assert(style == ReferenceCounting::Native); \
    emitNative##Name##Release(value, atomicity); \
  } \
  void emitStrongRetain##Name(toolchain::Value *value, ReferenceCounting style, \
                              Atomicity atomicity) { \
    assert(style == ReferenceCounting::Native); \
    emitNativeStrongRetain##Name(value, atomicity); \
  } \
  void emitStrongRetainAnd##Name##Release(toolchain::Value *value, \
                                          ReferenceCounting style, \
                                          Atomicity atomicity) { \
    assert(style == ReferenceCounting::Native); \
    emitNativeStrongRetainAnd##Name##Release(value, atomicity); \
  }
#include "language/AST/ReferenceStorage.def"
#undef NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER
#undef ALWAYS_OR_SOMETIMES_LOADABLE_CHECKED_REF_STORAGE_HELPER

  // Routines for the Codira native reference-counting style.
  //   - strong references
  void emitNativeStrongAssign(toolchain::Value *value, Address addr);
  void emitNativeStrongInit(toolchain::Value *value, Address addr);
  void emitNativeStrongRetain(toolchain::Value *value, Atomicity atomicity);
  void emitNativeStrongRelease(toolchain::Value *value, Atomicity atomicity);
  void emitNativeSetDeallocating(toolchain::Value *value);

  // Routines for the ObjC reference-counting style.
  void emitObjCStrongRetain(toolchain::Value *value);
  toolchain::Value *emitObjCRetainCall(toolchain::Value *value);
  toolchain::Value *emitObjCAutoreleaseCall(toolchain::Value *value);
  void emitObjCStrongRelease(toolchain::Value *value);

  toolchain::Value *emitBlockCopyCall(toolchain::Value *value);
  void emitBlockRelease(toolchain::Value *value);

  void emitForeignReferenceTypeLifetimeOperation(ValueDecl *fn,
                                                 toolchain::Value *value,
                                                 bool needsNullCheck = false);

  // Routines for an unknown reference-counting style (meaning,
  // dynamically something compatible with either the ObjC or Codira styles).
  //   - strong references
  void emitUnknownStrongRetain(toolchain::Value *value, Atomicity atomicity);
  void emitUnknownStrongRelease(toolchain::Value *value, Atomicity atomicity);

  // Routines for the Builtin.NativeObject reference-counting style.
  void emitBridgeStrongRetain(toolchain::Value *value, Atomicity atomicity);
  void emitBridgeStrongRelease(toolchain::Value *value, Atomicity atomicity);

  // Routines for the ErrorType reference-counting style.
  void emitErrorStrongRetain(toolchain::Value *value);
  void emitErrorStrongRelease(toolchain::Value *value);

  toolchain::Value *emitIsUniqueCall(toolchain::Value *value, ReferenceCounting style,
                                SourceLoc loc, bool isNonNull);

  toolchain::Value *emitIsEscapingClosureCall(toolchain::Value *value, SourceLoc loc,
                                         unsigned verificationType);

  Address emitTaskAlloc(toolchain::Value *size,
                        Alignment alignment);
  void emitTaskDealloc(Address address);
  void emitTaskDeallocThrough(Address address);

  toolchain::Value *alignUpToMaximumAlignment(toolchain::Type *sizeTy, toolchain::Value *val);

  //--- Expression emission
  //------------------------------------------------------
public:
  void emitFakeExplosion(const TypeInfo &type, Explosion &explosion);

//--- Type emission ------------------------------------------------------------
public:
  /// Look up a local type metadata reference, returning a null response
  /// if no entry was found which can satisfy the request.  This may need
  /// emit code to materialize the reference.
  ///
  /// This does a look up for a formal ("AST") type.  If you are looking for
  /// type metadata that will work for working with a representation
  /// ("lowered", "SIL") type, use getGetLocalTypeMetadataForLayout.
  MetadataResponse tryGetLocalTypeMetadata(CanType type,
                                           DynamicMetadataRequest request);

  /// Look up a local type data reference, returning null if no entry was
  /// found.  This may need to emit code to materialize the reference.
  ///
  /// The data kind cannot be for type metadata; use tryGetLocalTypeMetadata
  /// for that.
  toolchain::Value *tryGetLocalTypeData(CanType type, LocalTypeDataKind kind);

  /// The same as tryGetLocalTypeMetadata, but for representation-compatible
  /// "layout" metadata.  The returned metadata may not be for a type that
  /// has anything to do with the formal type that was lowered to the given
  /// type; however, it is guaranteed to have equivalent characteristics
  /// in terms of layout, spare bits, POD-ness, and so on.
  ///
  /// We use a separate function name for this to clarify that you should
  /// only ever be looking for type metadata for a lowered SILType for the
  /// purposes of local manipulation, such as the layout of a type or
  /// emitting a value-copy.
  MetadataResponse tryGetLocalTypeMetadataForLayout(SILType type,
                                           DynamicMetadataRequest request);

  /// The same as tryGetForLocalTypeData, but for representation-compatible
  /// "layout" metadata.  See the comment on tryGetLocalTypeMetadataForLayout.
  ///
  /// The data kind cannot be for type metadata; use
  /// tryGetLocalTypeMetadataForLayout for that.
  toolchain::Value *tryGetLocalTypeDataForLayout(SILType type,
                                            LocalTypeDataKind kind);

  /// Add a local type metadata reference at a point which definitely
  /// dominates all of its uses.
  void setUnscopedLocalTypeMetadata(CanType type,
                                    MetadataResponse response);

  /// Add a local type data reference at a point which definitely
  /// dominates all of its uses.
  ///
  /// The data kind cannot be for type metadata; use
  /// setUnscopedLocalTypeMetadata for that.
  void setUnscopedLocalTypeData(CanType type, LocalTypeDataKind kind,
                                toolchain::Value *data);

  /// Add a local type metadata reference that is valid at the current
  /// insertion point.
  void setScopedLocalTypeMetadata(CanType type, MetadataResponse value);

  /// Add a local type data reference that is valid at the current
  /// insertion point.
  ///
  /// The data kind cannot be for type metadata; use setScopedLocalTypeMetadata
  /// for that.
  void setScopedLocalTypeData(CanType type, LocalTypeDataKind kind,
                              toolchain::Value *data);

  /// The same as setScopedLocalTypeMetadata, but for representation-compatible
  /// "layout" metadata.  See the comment on tryGetLocalTypeMetadataForLayout.
  void setScopedLocalTypeMetadataForLayout(SILType type, MetadataResponse value);

  /// The same as setScopedLocalTypeData, but for representation-compatible
  /// "layout" metadata.  See the comment on tryGetLocalTypeMetadataForLayout.
  ///
  /// The data kind cannot be for type metadata; use
  /// setScopedLocalTypeMetadataForLayout for that.
  void setScopedLocalTypeDataForLayout(SILType type, LocalTypeDataKind kind,
                                       toolchain::Value *data);

  // These are for the private use of the LocalTypeData subsystem.
  MetadataResponse tryGetLocalTypeMetadata(LocalTypeDataKey key,
                                           DynamicMetadataRequest request);
  toolchain::Value *tryGetLocalTypeData(LocalTypeDataKey key);
  MetadataResponse tryGetConcreteLocalTypeData(LocalTypeDataKey key,
                                               DynamicMetadataRequest request);
  void setUnscopedLocalTypeData(LocalTypeDataKey key, MetadataResponse value);
  void setScopedLocalTypeData(LocalTypeDataKey key, MetadataResponse value,
                              bool mayEmitDebugInfo = true);

  /// Given a concrete type metadata node, add all the local type data
  /// that we can reach from it.
  void bindLocalTypeDataFromTypeMetadata(CanType type, IsExact_t isExact,
                                         toolchain::Value *metadata,
                                         MetadataState metadataState);

  /// Given the witness table parameter, bind local type data for
  /// the witness table itself and any conditional requirements.
  void bindLocalTypeDataFromSelfWitnessTable(
                const ProtocolConformance *conformance,
                toolchain::Value *selfTable,
                toolchain::function_ref<CanType (CanType)> mapTypeIntoContext);

  void setDominanceResolver(DominanceResolverFunction resolver) {
    assert(DominanceResolver == nullptr);
    DominanceResolver = resolver;
  }

  bool isActiveDominancePointDominatedBy(DominancePoint point) {
    // If the point is universal, it dominates.
    if (point.isUniversal()) return true;

    assert(!ActiveDominancePoint.isUniversal() &&
           "active dominance point is universal but there exists a"
           "non-universal point?");

    // If we don't have a resolver, we're emitting a simple helper
    // function; just assume dominance.
    if (!DominanceResolver) return true;

    // Otherwise, ask the resolver.
    return DominanceResolver(*this, ActiveDominancePoint, point);
  }

  /// Is the current dominance point conditional in some way not
  /// tracked by the active dominance point?
  ///
  /// This should only be used by the local type data cache code.
  bool isConditionalDominancePoint() const {
    return ConditionalDominance != nullptr;
  }

  void registerConditionalLocalTypeDataKey(LocalTypeDataKey key) {
    assert(ConditionalDominance != nullptr &&
           "not in a conditional dominance scope");
    ConditionalDominance->registerConditionalLocalTypeDataKey(key);
  }

  /// Return the currently-active dominance point.
  DominancePoint getActiveDominancePoint() const {
    return ActiveDominancePoint;
  }

  /// A RAII object for temporarily changing the dominance of the active
  /// definition point.
  class DominanceScope {
    IRGenFunction &IGF;
    DominancePoint OldDominancePoint;
  public:
    explicit DominanceScope(IRGenFunction &IGF, DominancePoint newPoint)
        : IGF(IGF), OldDominancePoint(IGF.ActiveDominancePoint) {
      IGF.ActiveDominancePoint = newPoint;
      assert(!newPoint.isOrdinary() || IGF.DominanceResolver);
    }

    DominanceScope(const DominanceScope &other) = delete;
    DominanceScope &operator=(const DominanceScope &other) = delete;

    ~DominanceScope() {
      IGF.ActiveDominancePoint = OldDominancePoint;
    }
  };

  /// A RAII object for temporarily suppressing type-data caching at the
  /// active definition point.  Do this if you're adding local control flow
  /// that isn't modeled by the dominance system.
  class ConditionalDominanceScope {
    IRGenFunction &IGF;
    ConditionalDominanceScope *OldScope;
    SmallVector<LocalTypeDataKey, 2> RegisteredKeys;

  public:
    explicit ConditionalDominanceScope(IRGenFunction &IGF)
        : IGF(IGF), OldScope(IGF.ConditionalDominance) {
      IGF.ConditionalDominance = this;
    }

    ConditionalDominanceScope(const ConditionalDominanceScope &other) = delete;
    ConditionalDominanceScope &operator=(const ConditionalDominanceScope &other)
      = delete;

    void registerConditionalLocalTypeDataKey(LocalTypeDataKey key) {
      RegisteredKeys.push_back(key);
    }

    ~ConditionalDominanceScope();
  };

  /// The kind of value DynamicSelf is.
  enum DynamicSelfKind {
    /// An object reference.
    ObjectReference,
    /// A Codira metatype.
    CodiraMetatype,
    /// An ObjC metatype.
    ObjCMetatype,
  };

  toolchain::Value *getDynamicSelfMetadata();
  void setDynamicSelfMetadata(CanType selfBaseTy, bool selfIsExact,
                              toolchain::Value *value, DynamicSelfKind kind);
#ifndef NDEBUG
  LocalTypeDataCache const *getLocalTypeData() { return LocalTypeData; }
#endif

  /// A forwardable argument is a load that is immediately preceeds the apply it
  /// is used as argument to. If there is no side-effecting instructions between
  /// said load and the apply, we can memcpy the loads address to the apply's
  /// indirect argument alloca.
  void clearForwardableArguments() {
    forwardableArguments.clear();
  }

  void setForwardableArgument(unsigned idx) {
    forwardableArguments.insert(idx);
  }

  bool isForwardableArgument(unsigned idx) const {
    return forwardableArguments.contains(idx);
  }
private:
  LocalTypeDataCache &getOrCreateLocalTypeData();
  void destroyLocalTypeData();

  LocalTypeDataCache *LocalTypeData = nullptr;

  /// The dominance resolver.  This can be set at most once; when it's not
  /// set, this emission must never have a non-null active definition point.
  DominanceResolverFunction DominanceResolver = nullptr;
  DominancePoint ActiveDominancePoint = DominancePoint::universal();
  ConditionalDominanceScope *ConditionalDominance = nullptr;
  
  /// The value that satisfies metadata lookups for DynamicSelfType.
  toolchain::Value *SelfValue = nullptr;
  /// If set, the dynamic Self type is assumed to be equivalent to this exact class.
  CanType SelfType;
  bool SelfTypeIsExact = false;
  DynamicSelfKind SelfKind;

  toolchain::SmallSetVector<unsigned, 16> forwardableArguments;
};

using ConditionalDominanceScope = IRGenFunction::ConditionalDominanceScope;

} // end namespace irgen
} // end namespace language

#endif
