//===--- GenConcurrency.cpp - IRGen for concurrency features --------------===//
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
//  This file implements IR generation for concurrency features (other than
//  basic async function lowering, which is more spread out).
//
//===----------------------------------------------------------------------===//

#include "GenConcurrency.h"

#include "BitPatternBuilder.h"
#include "ExtraInhabitants.h"
#include "GenCall.h"
#include "GenPointerAuth.h"
#include "GenProto.h"
#include "GenType.h"
#include "IRGenDebugInfo.h"
#include "IRGenFunction.h"
#include "IRGenModule.h"
#include "LoadableTypeInfo.h"
#include "ScalarPairTypeInfo.h"
#include "language/AST/ASTContext.h"
#include "language/AST/ProtocolConformanceRef.h"
#include "language/ABI/MetadataValues.h"
#include "language/Basic/Assertions.h"
#include "toolchain/IR/Module.h"

using namespace language;
using namespace irgen;

namespace {

/// A TypeInfo implementation for Builtin.Executor.
class ExecutorTypeInfo :
  public TrivialScalarPairTypeInfo<ExecutorTypeInfo, LoadableTypeInfo> {

public:
  ExecutorTypeInfo(toolchain::StructType *storageType,
                   Size size, Alignment align, SpareBitVector &&spareBits)
      : TrivialScalarPairTypeInfo(storageType, size, std::move(spareBits),
                                  align, IsTriviallyDestroyable,
                                  IsCopyable, IsFixedSize, IsABIAccessible) {}

  static Size getFirstElementSize(IRGenModule &IGM) {
    return IGM.getPointerSize();
  }
  static StringRef getFirstElementLabel() {
    return ".identity";
  }

  TypeLayoutEntry
  *buildTypeLayoutEntry(IRGenModule &IGM,
                        SILType T,
                        bool useStructLayouts) const override {
    if (!useStructLayouts) {
      return IGM.typeLayoutCache.getOrCreateTypeInfoBasedEntry(*this, T);
    }
    return IGM.typeLayoutCache.getOrCreateScalarEntry(*this, T,
                                            ScalarKind::TriviallyDestroyable);
  }

  static Size getSecondElementOffset(IRGenModule &IGM) {
    return IGM.getPointerSize();
  }
  static Size getSecondElementSize(IRGenModule &IGM) {
    return IGM.getPointerSize();
  }
  static StringRef getSecondElementLabel() {
    return ".impl";
  }

  // The identity pointer is a heap object reference.
  bool mayHaveExtraInhabitants(IRGenModule &IGM) const override {
    return true;
  }
  PointerInfo getPointerInfo(IRGenModule &IGM) const {
    return PointerInfo::forHeapObject(IGM);
  }
  unsigned getFixedExtraInhabitantCount(IRGenModule &IGM) const override {
    return getPointerInfo(IGM).getExtraInhabitantCount(IGM);
  }
  APInt getFixedExtraInhabitantValue(IRGenModule &IGM,
                                     unsigned bits,
                                     unsigned index) const override {
    return getPointerInfo(IGM)
          .getFixedExtraInhabitantValue(IGM, bits, index, 0);
  }
  toolchain::Value *getExtraInhabitantIndex(IRGenFunction &IGF, Address src,
                                       SILType T,
                                       bool isOutlined) const override {
    src = projectFirstElement(IGF, src);
    return getPointerInfo(IGF.IGM).getExtraInhabitantIndex(IGF, src);
  }
  void storeExtraInhabitant(IRGenFunction &IGF, toolchain::Value *index,
                            Address dest, SILType T,
                            bool isOutlined) const override {
    // Store the extra-inhabitant value in the first (identity) word.
    auto first = projectFirstElement(IGF, dest);
    getPointerInfo(IGF.IGM).storeExtraInhabitant(IGF, index, first);

    // Zero the second word.
    auto second = projectSecondElement(IGF, dest);
    IGF.Builder.CreateStore(toolchain::ConstantInt::get(IGF.IGM.ExecutorSecondTy, 0),
                            second);
  }
};

} // end anonymous namespace

const LoadableTypeInfo &IRGenModule::getExecutorTypeInfo() {
  return Types.getExecutorTypeInfo();
}

const LoadableTypeInfo &TypeConverter::getExecutorTypeInfo() {
  if (ExecutorTI) return *ExecutorTI;

  auto ty = IGM.CodiraExecutorTy;

  SpareBitVector spareBits;
  spareBits.append(IGM.getHeapObjectSpareBits());
  spareBits.appendClearBits(IGM.getPointerSize().getValueInBits());

  ExecutorTI =
    new ExecutorTypeInfo(ty, IGM.getPointerSize() * 2,
                         IGM.getPointerAlignment(),
                         std::move(spareBits));
  ExecutorTI->NextConverted = FirstType;
  FirstType = ExecutorTI;
  return *ExecutorTI;
}

void irgen::emitBuildMainActorExecutorRef(IRGenFunction &IGF,
                                          Explosion &out) {
  auto call = IGF.Builder.CreateCall(
      IGF.IGM.getTaskGetMainExecutorFunctionPointer(), {});
  call->setDoesNotThrow();
  call->setCallingConv(IGF.IGM.CodiraCC);

  IGF.emitAllExtractValues(call, IGF.IGM.CodiraExecutorTy, out);
}

void irgen::emitBuildDefaultActorExecutorRef(IRGenFunction &IGF,
                                             toolchain::Value *actor,
                                             Explosion &out) {
  // The implementation word of a default actor is just a null pointer.
  toolchain::Value *identity =
    IGF.Builder.CreatePtrToInt(actor, IGF.IGM.ExecutorFirstTy);
  toolchain::Value *impl = toolchain::ConstantInt::get(IGF.IGM.ExecutorSecondTy, 0);

  out.add(identity);
  out.add(impl);
}

void irgen::emitBuildOrdinaryTaskExecutorRef(
    IRGenFunction &IGF, toolchain::Value *executor, CanType executorType,
    ProtocolConformanceRef executorConf, Explosion &out) {
  // The implementation word of an "ordinary" executor is
  // just the witness table pointer with no flags set.
  toolchain::Value *identity =
      IGF.Builder.CreatePtrToInt(executor, IGF.IGM.ExecutorFirstTy);
  toolchain::Value *impl = emitWitnessTableRef(IGF, executorType, executorConf);
  impl = IGF.Builder.CreatePtrToInt(impl, IGF.IGM.ExecutorSecondTy);

  out.add(identity);
  out.add(impl);
}

void irgen::emitBuildOrdinarySerialExecutorRef(IRGenFunction &IGF,
                                               toolchain::Value *executor,
                                               CanType executorType,
                                               ProtocolConformanceRef executorConf,
                                               Explosion &out) {
  // The implementation word of an "ordinary" serial executor is
  // just the witness table pointer with no flags set.
  toolchain::Value *identity =
    IGF.Builder.CreatePtrToInt(executor, IGF.IGM.ExecutorFirstTy);
  toolchain::Value *impl =
    emitWitnessTableRef(IGF, executorType, executorConf);
  impl = IGF.Builder.CreatePtrToInt(impl, IGF.IGM.ExecutorSecondTy);

  out.add(identity);
  out.add(impl);
}

void irgen::emitBuildComplexEqualitySerialExecutorRef(IRGenFunction &IGF,
                                               toolchain::Value *executor,
                                               CanType executorType,
                                               ProtocolConformanceRef executorConf,
                                               Explosion &out) {
  toolchain::Value *identity =
    IGF.Builder.CreatePtrToInt(executor, IGF.IGM.ExecutorFirstTy);

  // The implementation word of an "complex equality" serial executor is
  // the witness table pointer with the ExecutorKind::ComplexEquality flag set.
  toolchain::Value *impl =
    emitWitnessTableRef(IGF, executorType, executorConf);
  impl = IGF.Builder.CreatePtrToInt(impl, IGF.IGM.ExecutorSecondTy);

  // NOTE: Refer to SerialExecutorRef::ExecutorKind for the flag values.
  toolchain::IntegerType *IntPtrTy = IGF.IGM.IntPtrTy;
  auto complexEqualityExecutorKindFlag =
      toolchain::Constant::getIntegerValue(IntPtrTy, APInt(IntPtrTy->getBitWidth(),
                                                      0b01));
  impl = IGF.Builder.CreateOr(impl, complexEqualityExecutorKindFlag);

  out.add(identity);
  out.add(impl);
}

void irgen::emitGetCurrentExecutor(IRGenFunction &IGF, Explosion &out) {
  auto *call = IGF.Builder.CreateCall(
      IGF.IGM.getTaskGetCurrentExecutorFunctionPointer(), {});
  call->setDoesNotThrow();
  call->setCallingConv(IGF.IGM.CodiraCC);

  IGF.emitAllExtractValues(call, IGF.IGM.CodiraExecutorTy, out);
}

toolchain::Value *irgen::emitBuiltinStartAsyncLet(IRGenFunction &IGF,
                                             toolchain::Value *taskOptions,
                                             toolchain::Value *taskFunction,
                                             toolchain::Value *localContextInfo,
                                             toolchain::Value *localResultBuffer,
                                             SubstitutionMap subs) {
  localContextInfo = IGF.Builder.CreateBitCast(localContextInfo,
                                               IGF.IGM.OpaquePtrTy);
  
  // stack allocate AsyncLet, and begin lifetime for it (until EndAsyncLet)
  auto ty = toolchain::ArrayType::get(IGF.IGM.Int8PtrTy, NumWords_AsyncLet);
  auto address = IGF.createAlloca(ty, Alignment(Alignment_AsyncLet));
  auto alet = IGF.Builder.CreateBitCast(address.getAddress(),
                                        IGF.IGM.Int8PtrTy);
  IGF.Builder.CreateLifetimeStart(alet);

  assert(subs.getReplacementTypes().size() == 1 &&
         "startAsyncLet should have a type substitution");
  auto futureResultType = subs.getReplacementTypes()[0]->getCanonicalType();

  toolchain::Value *futureResultTypeMetadata =
      toolchain::ConstantPointerNull::get(IGF.IGM.Int8PtrTy);
  if (!IGF.IGM.Context.LangOpts.hasFeature(Feature::Embedded)) {
    futureResultTypeMetadata =
        IGF.emitAbstractTypeMetadataRef(futureResultType);
  }

  // The concurrency runtime for older Apple OSes has a bug in task formation
  // for `async let`s that may manifest when trying to use room in the
  // parent task's preallocated `async let` buffer for the child task's
  // initial task allocator slab. If targeting those older OSes, pad the
  // context size for async let entry points to never fit in the preallocated
  // space, so that we don't run into that bug. We leave a note on the
  // declaration so that coroutine splitting can pad out the final context
  // size after splitting.
  auto deploymentAvailability =
      AvailabilityRange::forDeploymentTarget(IGF.IGM.Context);
  if (!deploymentAvailability.isContainedIn(
                                   IGF.IGM.Context.getCodira57Availability()))
  {
    auto taskAsyncFunctionPointer
                = cast<toolchain::GlobalVariable>(taskFunction->stripPointerCasts());

    if (auto taskAsyncID
          = IGF.IGM.getAsyncCoroIDMapping(taskAsyncFunctionPointer)) {
      // If the entry point function has already been emitted, retroactively
      // pad out the initial context size in the async function pointer record
      // and ID intrinsic so that it will never fit in the preallocated space.
      uint64_t origSize = cast<toolchain::ConstantInt>(taskAsyncID->getArgOperand(0))
        ->getValue().getLimitedValue();
      
      uint64_t paddedSize = std::max(origSize,
                     (NumWords_AsyncLet * IGF.IGM.getPointerSize()).getValue());
      auto paddedSizeVal = toolchain::ConstantInt::get(IGF.IGM.Int32Ty, paddedSize);
      taskAsyncID->setArgOperand(0, paddedSizeVal);
      
      auto origInit = taskAsyncFunctionPointer->getInitializer();
      auto newInit = toolchain::ConstantStruct::get(
                                   cast<toolchain::StructType>(origInit->getType()),
                                   origInit->getAggregateElement(0u),
                                   paddedSizeVal);
      taskAsyncFunctionPointer->setInitializer(newInit);
    } else {
      // If it hasn't been emitted yet, mark it to get the padding when it does
      // get emitted.
      IGF.IGM.markAsyncFunctionPointerForPadding(taskAsyncFunctionPointer);
    }
  }

  // In embedded Codira, create and pass result type info.
  taskOptions =
    maybeAddEmbeddedCodiraResultTypeInfo(IGF, taskOptions, futureResultType);
  
  toolchain::CallInst *call;
  if (localResultBuffer) {
    // This is @_silgen_name("language_asyncLet_begin")
    call = IGF.Builder.CreateCall(IGF.IGM.getAsyncLetBeginFunctionPointer(),
                                  {alet, taskOptions, futureResultTypeMetadata,
                                   taskFunction, localContextInfo,
                                   localResultBuffer});
  } else {
    // This is @_silgen_name("language_asyncLet_start")
    call = IGF.Builder.CreateCall(IGF.IGM.getAsyncLetStartFunctionPointer(),
                                  {alet, taskOptions, futureResultTypeMetadata,
                                   taskFunction, localContextInfo});
  }
  call->setDoesNotThrow();
  call->setCallingConv(IGF.IGM.CodiraCC);

  return alet;
}

void irgen::emitEndAsyncLet(IRGenFunction &IGF, toolchain::Value *alet) {
  auto *call =
      IGF.Builder.CreateCall(IGF.IGM.getEndAsyncLetFunctionPointer(), {alet});
  call->setDoesNotThrow();
  call->setCallingConv(IGF.IGM.CodiraCC);

  IGF.Builder.CreateLifetimeEnd(alet);
}

toolchain::Value *irgen::emitCreateTaskGroup(IRGenFunction &IGF,
                                        SubstitutionMap subs,
                                        toolchain::Value *groupFlags) {
  auto ty = toolchain::ArrayType::get(IGF.IGM.Int8PtrTy, NumWords_TaskGroup);
  auto address = IGF.createAlloca(ty, Alignment(Alignment_TaskGroup));
  auto group = IGF.Builder.CreateBitCast(address.getAddress(),
                                         IGF.IGM.Int8PtrTy);
  IGF.Builder.CreateLifetimeStart(group);
  assert(subs.getReplacementTypes().size() == 1 &&
         "createTaskGroup should have a type substitution");
  auto resultType = subs.getReplacementTypes()[0]->getCanonicalType();

  if (IGF.IGM.Context.LangOpts.hasFeature(Feature::Embedded)) {
    // In Embedded Codira, call language_taskGroup_initializeWithOptions instead, to
    // avoid needing a Metadata argument.
    toolchain::Value *options = toolchain::ConstantPointerNull::get(IGF.IGM.Int8PtrTy);
    toolchain::Value *resultTypeMetadata = toolchain::ConstantPointerNull::get(IGF.IGM.Int8PtrTy);
    options = maybeAddEmbeddedCodiraResultTypeInfo(IGF, options, resultType);
    if (!groupFlags) groupFlags = toolchain::ConstantInt::get(IGF.IGM.SizeTy, 0);
    toolchain::CallInst *call = IGF.Builder.CreateCall(IGF.IGM.getTaskGroupInitializeWithOptionsFunctionPointer(),
                                  {groupFlags, group, resultTypeMetadata, options});
    call->setDoesNotThrow();
    call->setCallingConv(IGF.IGM.CodiraCC);
    return group;
  }

  auto resultTypeMetadata = IGF.emitAbstractTypeMetadataRef(resultType);

  toolchain::CallInst *call;
  if (groupFlags) {
    call = IGF.Builder.CreateCall(IGF.IGM.getTaskGroupInitializeWithFlagsFunctionPointer(),
                                  {groupFlags, group, resultTypeMetadata});
  } else {
    call =
        IGF.Builder.CreateCall(IGF.IGM.getTaskGroupInitializeFunctionPointer(),
                               {group, resultTypeMetadata});
  }
  call->setDoesNotThrow();
  call->setCallingConv(IGF.IGM.CodiraCC);

  return group;
}

void irgen::emitDestroyTaskGroup(IRGenFunction &IGF, toolchain::Value *group) {
  auto *call = IGF.Builder.CreateCall(
      IGF.IGM.getTaskGroupDestroyFunctionPointer(), {group});
  call->setDoesNotThrow();
  call->setCallingConv(IGF.IGM.CodiraCC);

  IGF.Builder.CreateLifetimeEnd(group);
}

toolchain::Function *IRGenModule::getAwaitAsyncContinuationFn() {
  StringRef name = "__language_continuation_await_point";
  if (toolchain::GlobalValue *F = Module.getNamedValue(name))
    return cast<toolchain::Function>(F);

  // The parameters here match the extra arguments passed to
  // @toolchain.coro.suspend.async by emitAwaitAsyncContinuation.
  toolchain::Type *argTys[] = { ContinuationAsyncContextPtrTy };
  auto *suspendFnTy =
    toolchain::FunctionType::get(VoidTy, argTys, false /*vaargs*/);

  toolchain::Function *suspendFn =
      toolchain::Function::Create(suspendFnTy, toolchain::Function::InternalLinkage,
                             name, &Module);
  suspendFn->setCallingConv(CodiraAsyncCC);
  suspendFn->setDoesNotThrow();
  IRGenFunction suspendIGF(*this, suspendFn);
  if (DebugInfo)
    DebugInfo->emitArtificialFunction(suspendIGF, suspendFn);
  auto &Builder = suspendIGF.Builder;

  toolchain::Value *context = suspendFn->getArg(0);
  auto *call =
      Builder.CreateCall(getContinuationAwaitFunctionPointer(), {context});
  call->setCallingConv(CodiraAsyncCC);
  call->setDoesNotThrow();
  call->setTailCallKind(AsyncTailCallKind);

  Builder.CreateRetVoid();
  return suspendFn;
}

void irgen::emitTaskRunInline(IRGenFunction &IGF, SubstitutionMap subs,
                              toolchain::Value *result, toolchain::Value *closure,
                              toolchain::Value *closureContext) {
  assert(subs.getReplacementTypes().size() == 1 &&
         "taskRunInline should have a type substitution");
  auto resultType = subs.getReplacementTypes()[0]->getCanonicalType();
  auto resultTypeMetadata = IGF.emitAbstractTypeMetadataRef(resultType);

  auto *call = IGF.Builder.CreateCall(
      IGF.IGM.getTaskRunInlineFunctionPointer(),
      {result, closure, closureContext, resultTypeMetadata});
  call->setDoesNotThrow();
  call->setCallingConv(IGF.IGM.CodiraCC);
}


void irgen::emitTaskCancel(IRGenFunction &IGF, toolchain::Value *task) {
  if (task->getType() != IGF.IGM.CodiraTaskPtrTy) {
    task = IGF.Builder.CreateBitCast(task, IGF.IGM.CodiraTaskPtrTy);
  }

  auto *call =
      IGF.Builder.CreateCall(IGF.IGM.getTaskCancelFunctionPointer(), {task});
  call->setDoesNotThrow();
  call->setCallingConv(IGF.IGM.CodiraCC);
}

template <class RecordTraits>
static Address allocateOptionRecord(IRGenFunction &IGF,
                                    const RecordTraits &traits) {
  return IGF.createAlloca(RecordTraits::getRecordType(IGF.IGM),
                          IGF.IGM.getPointerAlignment(),
                          traits.getLabel() + "_record");
}

static void initializeOptionRecordHeader(IRGenFunction &IGF,
                                         Address recordAddr,
                                         TaskOptionRecordFlags flags,
                                         toolchain::Value *curRecordPointer) {
  auto baseRecordAddr =
    IGF.Builder.CreateStructGEP(recordAddr, 0, Size(0));

  // Flags
  auto flagsValue =
    toolchain::ConstantInt::get(IGF.IGM.SizeTy, flags.getOpaqueValue());
  IGF.Builder.CreateStore(flagsValue,
    IGF.Builder.CreateStructGEP(baseRecordAddr, 0, Size(0)));

  // Parent
  IGF.Builder.CreateStore(curRecordPointer,
    IGF.Builder.CreateStructGEP(baseRecordAddr, 1, IGF.IGM.getPointerSize()));
}


template <class RecordTraits, class... Args>
static toolchain::Value *initializeOptionRecord(IRGenFunction &IGF,
                                           Address recordAddr,
                                           toolchain::Value *curRecordPointer,
                                           const RecordTraits &traits,
                                           Args &&... args) {
  initializeOptionRecordHeader(IGF, recordAddr, traits.getRecordFlags(),
                               curRecordPointer);

  traits.initialize(IGF, recordAddr, std::forward<Args>(args)...);

  toolchain::Value *newRecordPointer = IGF.Builder.CreateBitOrPointerCast(
      recordAddr.getAddress(), IGF.IGM.CodiraTaskOptionRecordPtrTy);
  return newRecordPointer;
}

template <class RecordTraits, class... Args>
static toolchain::Value *addOptionRecord(IRGenFunction &IGF,
                                    toolchain::Value *curRecordPointer,
                                    const RecordTraits &traits,
                                    Args &&... args) {
  auto recordAddr = allocateOptionRecord(IGF, traits);
  return initializeOptionRecord(IGF, recordAddr, curRecordPointer, traits,
                                std::forward<Args>(args)...);
}

/// Add a task option record to the options list if the given value
/// is present.
template <class RecordTraits>
static toolchain::Value *maybeAddOptionRecord(IRGenFunction &IGF,
                                         toolchain::Value *curRecordPointer,
                                         const RecordTraits &traits,
                                         OptionalExplosion &value) {
  // We can completely avoid doing any work if the value is statically nil.
  if (value.isNone()) return curRecordPointer;

  // Otherwise, allocate the option record.
  auto recordAddr = allocateOptionRecord(IGF, traits);

  // If the value is statically non-nil, we can unconditionally
  // initialize the record and add it to the chain.
  if (value.isSome()) {
    return initializeOptionRecord(IGF, recordAddr, curRecordPointer,
                                  traits, value.getSomeExplosion());
  }

  // Otherwise, we have to check whether the value is nil dynamically.
  toolchain::BasicBlock *contBB = IGF.createBasicBlock(traits.getLabel() + ".cont");
  toolchain::BasicBlock *someBB = IGF.createBasicBlock(traits.getLabel() + ".some");

  auto &ctx = IGF.IGM.Context;

  SILType optionalType =
    IGF.IGM.getLoweredType(traits.getValueType(ctx).wrapInOptionalType());
  auto &optionalStrategy = getEnumImplStrategy(IGF.IGM, optionalType);

  // Branch based on whether the value is nil.  We're going to use the
  // value twice, so borrow it the first time.
  value.getOptionalExplosion().borrowing([&](Explosion &borrowedValue) {
    optionalStrategy.emitValueSwitch(IGF, borrowedValue,
                                     {{ctx.getOptionalSomeDecl(), someBB},
                                      {ctx.getOptionalNoneDecl(), contBB}},
                                     /*default*/ nullptr);
  });
  auto noneOriginBB = IGF.Builder.GetInsertBlock();

  // Enter the block for the case where the value is non-nil.
  IGF.Builder.emitBlock(someBB);

  // Project the payload from the optional value.
  Explosion objectValue;
  optionalStrategy.emitValueProject(IGF, value.getOptionalExplosion(),
                                    ctx.getOptionalSomeDecl(),
                                    objectValue);

  // Initialize the record.
  toolchain::Value *someRecordPointer =
    initializeOptionRecord(IGF, recordAddr, curRecordPointer,
                           traits, objectValue);

  auto someOriginBB = IGF.Builder.GetInsertBlock();
  IGF.Builder.CreateBr(contBB);

  // Enter the continuation block and create a phi to merge the two cases.
  IGF.Builder.emitBlock(contBB);
  auto recordPointerPHI =
    IGF.Builder.CreatePHI(IGF.IGM.CodiraTaskOptionRecordPtrTy, /*num cases*/ 2);
  recordPointerPHI->addIncoming(curRecordPointer, noneOriginBB);
  recordPointerPHI->addIncoming(someRecordPointer, someOriginBB);

  return recordPointerPHI;
}

namespace {
struct EmbeddedCodiraResultTypeOptionRecordTraits {
  CanType formalResultType;

  static StringRef getLabel() {
    return "result_type_info";
  }
  static toolchain::StructType *getRecordType(IRGenModule &IGM) {
    return IGM.CodiraResultTypeInfoTaskOptionRecordTy;
  }
  static TaskOptionRecordFlags getRecordFlags() {
    return TaskOptionRecordFlags(TaskOptionRecordKind::ResultTypeInfo);
  }

  void initialize(IRGenFunction &IGF, Address optionsRecord) const {
    SILType lowered = IGF.IGM.getLoweredType(formalResultType);
    const TypeInfo &TI = IGF.IGM.getTypeInfo(lowered);
    CanType canType = lowered.getASTType();
    FixedPacking packing = TI.getFixedPacking(IGF.IGM);

    // Size
    IGF.Builder.CreateStore(
        TI.getStaticSize(IGF.IGM),
        IGF.Builder.CreateStructGEP(optionsRecord, 1, Size()));
    // Align mask
    IGF.Builder.CreateStore(
        TI.getStaticAlignmentMask(IGF.IGM),
        IGF.Builder.CreateStructGEP(optionsRecord, 2, Size()));

    auto schema = IGF.getOptions().PointerAuth.ValueWitnesses;
    // initializeWithCopy witness
    {
      auto gep = IGF.Builder.CreateStructGEP(optionsRecord, 3, Size());
      toolchain::Value *witness = IGF.IGM.getOrCreateValueWitnessFunction(
          ValueWitness::InitializeWithCopy, packing, canType, lowered, TI);
      auto discriminator = toolchain::ConstantInt::get(
          IGF.IGM.Int64Ty,
          SpecialPointerAuthDiscriminators::InitializeWithCopy);
      auto storageAddress = gep.getAddress();
      auto info =
          PointerAuthInfo::emit(IGF, schema, storageAddress, discriminator);
      if (schema) witness = emitPointerAuthSign(IGF, witness, info);
      IGF.Builder.CreateStore(witness, gep);
    }
    // storeEnumTagSinglePayload witness
    {
      auto gep = IGF.Builder.CreateStructGEP(optionsRecord, 4, Size());
      toolchain::Value *witness = IGF.IGM.getOrCreateValueWitnessFunction(
          ValueWitness::StoreEnumTagSinglePayload, packing, canType, lowered,
          TI);
      auto discriminator = toolchain::ConstantInt::get(
          IGF.IGM.Int64Ty,
          SpecialPointerAuthDiscriminators::StoreEnumTagSinglePayload);
      auto storageAddress = gep.getAddress();
      auto info =
          PointerAuthInfo::emit(IGF, schema, storageAddress, discriminator);
      if (schema) witness = emitPointerAuthSign(IGF, witness, info);
      IGF.Builder.CreateStore(witness, gep);
    }
    // destroy witness
    {
      auto gep = IGF.Builder.CreateStructGEP(optionsRecord, 5, Size());
      toolchain::Value *witness = IGF.IGM.getOrCreateValueWitnessFunction(
          ValueWitness::Destroy, packing, canType, lowered, TI);
      auto discriminator = toolchain::ConstantInt::get(
          IGF.IGM.Int64Ty, SpecialPointerAuthDiscriminators::Destroy);
      auto storageAddress = gep.getAddress();
      auto info =
          PointerAuthInfo::emit(IGF, schema, storageAddress, discriminator);
      if (schema) witness = emitPointerAuthSign(IGF, witness, info);
      IGF.Builder.CreateStore(witness, gep);
    }
  }
};
} // end anonymous namespace

toolchain::Value *irgen::maybeAddEmbeddedCodiraResultTypeInfo(IRGenFunction &IGF,
                                                        toolchain::Value *taskOptions,
                                                        CanType formalResultType) {
  if (!IGF.IGM.Context.LangOpts.hasFeature(Feature::Embedded))
    return taskOptions;

  EmbeddedCodiraResultTypeOptionRecordTraits traits{formalResultType};
  return addOptionRecord(IGF, taskOptions, traits);
}

namespace {

struct InitialSerialExecutorRecordTraits {
  static StringRef getLabel() {
    return "initial_serial_executor";
  }
  static toolchain::StructType *getRecordType(IRGenModule &IGM) {
    return IGM.CodiraInitialSerialExecutorTaskOptionRecordTy;
  }
  static TaskOptionRecordFlags getRecordFlags() {
    return TaskOptionRecordFlags(TaskOptionRecordKind::InitialSerialExecutor);
  }
  static CanType getValueType(ASTContext &ctx) {
    return ctx.TheExecutorType;
  }

  void initialize(IRGenFunction &IGF, Address recordAddr,
                  Explosion &serialExecutor) const {
    auto executorRecord =
      IGF.Builder.CreateStructGEP(recordAddr, 1, 2 * IGF.IGM.getPointerSize());
    IGF.Builder.CreateStore(serialExecutor.claimNext(),
      IGF.Builder.CreateStructGEP(executorRecord, 0, Size()));
    IGF.Builder.CreateStore(serialExecutor.claimNext(),
      IGF.Builder.CreateStructGEP(executorRecord, 1, Size()));
  }
};

struct TaskGroupRecordTraits {
  static StringRef getLabel() {
    return "task_group";
  }
  static toolchain::StructType *getRecordType(IRGenModule &IGM) {
    return IGM.CodiraTaskGroupTaskOptionRecordTy;
  }
  static TaskOptionRecordFlags getRecordFlags() {
    return TaskOptionRecordFlags(TaskOptionRecordKind::TaskGroup);
  }
  static CanType getValueType(ASTContext &ctx) {
    return ctx.TheRawPointerType;
  }

  void initialize(IRGenFunction &IGF, Address recordAddr,
                  Explosion &taskGroup) const {
    IGF.Builder.CreateStore(
        taskGroup.claimNext(),
        IGF.Builder.CreateStructGEP(recordAddr, 1, 2 * IGF.IGM.getPointerSize()));
  }
};

struct InitialTaskExecutorUnownedRecordTraits {
  static StringRef getLabel() {
    return "task_executor_unowned";
  }
  static toolchain::StructType *getRecordType(IRGenModule &IGM) {
    return IGM.CodiraInitialTaskExecutorUnownedPreferenceTaskOptionRecordTy;
  }
  static TaskOptionRecordFlags getRecordFlags() {
    return TaskOptionRecordFlags(TaskOptionRecordKind::InitialTaskExecutorUnowned);
  }
  static CanType getValueType(ASTContext &ctx) {
    return ctx.TheExecutorType;
  }

  void initialize(IRGenFunction &IGF, Address recordAddr,
                  Explosion &taskExecutor) const {
    auto executorRecord =
      IGF.Builder.CreateStructGEP(recordAddr, 1, 2 * IGF.IGM.getPointerSize());
    IGF.Builder.CreateStore(taskExecutor.claimNext(),
      IGF.Builder.CreateStructGEP(executorRecord, 0, Size()));
    IGF.Builder.CreateStore(taskExecutor.claimNext(),
      IGF.Builder.CreateStructGEP(executorRecord, 1, Size()));
  }
};

struct InitialTaskExecutorOwnedRecordTraits {
  static StringRef getLabel() {
    return "task_executor_owned";
  }
  static toolchain::StructType *getRecordType(IRGenModule &IGM) {
    return IGM.CodiraInitialTaskExecutorOwnedPreferenceTaskOptionRecordTy;
  }
  static TaskOptionRecordFlags getRecordFlags() {
    return TaskOptionRecordFlags(TaskOptionRecordKind::InitialTaskExecutorOwned);
  }
  static CanType getValueType(ASTContext &ctx) {
    return OptionalType::get(ctx.getProtocol(KnownProtocolKind::TaskExecutor)
                                 ->getDeclaredInterfaceType())
        ->getCanonicalType();
  }

  void initialize(IRGenFunction &IGF, Address recordAddr,
                  Explosion &taskExecutor) const {
    auto executorRecord =
      IGF.Builder.CreateStructGEP(recordAddr, 1, 2 * IGF.IGM.getPointerSize());

    // This relies on the fact that the HeapObject is directly followed by a
    // pointer to the witness table.
    IGF.Builder.CreateStore(taskExecutor.claimNext(),
        IGF.Builder.CreateStructGEP(executorRecord, 0, Size()));
    IGF.Builder.CreateStore(taskExecutor.claimNext(),
        IGF.Builder.CreateStructGEP(executorRecord, 1, Size()));
  }
};

struct InitialTaskNameRecordTraits {
  static StringRef getLabel() {
    return "task_name";
  }
  static toolchain::StructType *getRecordType(IRGenModule &IGM) {
    return IGM.CodiraInitialTaskNameTaskOptionRecordTy;
  }
  static TaskOptionRecordFlags getRecordFlags() {
    return TaskOptionRecordFlags(TaskOptionRecordKind::InitialTaskName);
  }
  static CanType getValueType(ASTContext &ctx) {
      return ctx.TheRawPointerType;
  }

  // Create 'InitialTaskNameTaskOptionRecord'
  void initialize(IRGenFunction &IGF, Address recordAddr,
                  Explosion &taskName) const {
    auto record =
      IGF.Builder.CreateStructGEP(recordAddr, 1, 2 * IGF.IGM.getPointerSize());
    IGF.Builder.CreateStore(taskName.claimNext(), record);
  }
};

} // end anonymous namespace

static toolchain::Value *
maybeAddInitialSerialExecutorOptionRecord(IRGenFunction &IGF,
                                          toolchain::Value *prevOptions,
                                          OptionalExplosion &serialExecutor) {
  return maybeAddOptionRecord(IGF, prevOptions,
                              InitialSerialExecutorRecordTraits(),
                              serialExecutor);
}

static toolchain::Value *
maybeAddTaskGroupOptionRecord(IRGenFunction &IGF, toolchain::Value *prevOptions,
                              OptionalExplosion &taskGroup) {
  return maybeAddOptionRecord(IGF, prevOptions, TaskGroupRecordTraits(),
                              taskGroup);
}

static toolchain::Value *
maybeAddInitialTaskExecutorOptionRecord(IRGenFunction &IGF,
                                        toolchain::Value *prevOptions,
                                        OptionalExplosion &taskExecutor) {
  return maybeAddOptionRecord(IGF, prevOptions,
                              InitialTaskExecutorUnownedRecordTraits(),
                              taskExecutor);
}

static toolchain::Value *
maybeAddInitialTaskExecutorOwnedOptionRecord(IRGenFunction &IGF,
                                        toolchain::Value *prevOptions,
                                        OptionalExplosion &taskExecutorExistential) {
  return maybeAddOptionRecord(IGF, prevOptions,
                              InitialTaskExecutorOwnedRecordTraits(),
                              taskExecutorExistential);
}

static toolchain::Value *
maybeAddTaskNameOptionRecord(IRGenFunction &IGF, toolchain::Value *prevOptions,
                             OptionalExplosion &taskName) {
  return maybeAddOptionRecord(IGF, prevOptions, InitialTaskNameRecordTraits(),
                              taskName);
}

std::pair<toolchain::Value *, toolchain::Value *>
irgen::emitTaskCreate(IRGenFunction &IGF, toolchain::Value *flags,
                      OptionalExplosion &serialExecutor,
                      OptionalExplosion &taskGroup,
                      OptionalExplosion &taskExecutorUnowned,
                      OptionalExplosion &taskExecutorExistential,
                      OptionalExplosion &taskName,
                      Explosion &taskFunction,
                      SubstitutionMap subs) {
  toolchain::Value *taskOptions =
    toolchain::ConstantPointerNull::get(IGF.IGM.CodiraTaskOptionRecordPtrTy);

  CanType resultType;
  if (subs) {
    resultType = subs.getReplacementTypes()[0]->getCanonicalType();
  } else {
    resultType = IGF.IGM.Context.TheEmptyTupleType;
  }

  toolchain::Value *resultTypeMetadata;
  if (IGF.IGM.Context.LangOpts.hasFeature(Feature::Embedded)) {
    resultTypeMetadata = toolchain::ConstantPointerNull::get(IGF.IGM.Int8PtrTy);
  } else {
    resultTypeMetadata = IGF.emitTypeMetadataRef(resultType);
  }

  // Add an option record for the initial serial executor, if present.
  taskOptions =
    maybeAddInitialSerialExecutorOptionRecord(IGF, taskOptions, serialExecutor);

  // Add an option record for the task group, if present.
  taskOptions = maybeAddTaskGroupOptionRecord(IGF, taskOptions, taskGroup);

  // Add an option record for the initial task executor, if present.
  {
    // Deprecated: This is the UnownedTaskExecutor? which is NOT consuming
    taskOptions = maybeAddInitialTaskExecutorOptionRecord(
        IGF, taskOptions, taskExecutorUnowned);
    // Take an (any TaskExecutor)? which we retain until task has completed
    taskOptions = maybeAddInitialTaskExecutorOwnedOptionRecord(
        IGF, taskOptions, taskExecutorExistential);
  }

  // Add an option record for the initial task name, if present.
  taskOptions = maybeAddTaskNameOptionRecord(IGF, taskOptions, taskName);

  // In embedded Codira, create and pass result type info.
  taskOptions = maybeAddEmbeddedCodiraResultTypeInfo(IGF, taskOptions, resultType);

  auto taskFunctionPtr = taskFunction.claimNext();
  auto taskFunctionContext = taskFunction.claimNext();

  toolchain::CallInst *result = IGF.Builder.CreateCall(
      IGF.IGM.getTaskCreateFunctionPointer(),
      {flags, taskOptions, resultTypeMetadata,
       taskFunctionPtr, taskFunctionContext});
  result->setDoesNotThrow();
  result->setCallingConv(IGF.IGM.CodiraCC);

  // Cast back to NativeObject/RawPointer.
  auto newTask = IGF.Builder.CreateExtractValue(result, { 0 });
  newTask = IGF.Builder.CreateBitCast(newTask, IGF.IGM.RefCountedPtrTy);
  auto newContext = IGF.Builder.CreateExtractValue(result, { 1 });
  newContext = IGF.Builder.CreateBitCast(newContext, IGF.IGM.Int8PtrTy);
  return { newTask, newContext };
}
