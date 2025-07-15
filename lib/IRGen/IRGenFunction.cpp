//===--- IRGenFunction.cpp - Codira Per-Function IR Generation -------------===//
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
//  This file implements basic setup and teardown for the class which
//  performs IR generation for function bodies.
//
//===----------------------------------------------------------------------===//
#include "language/ABI/MetadataValues.h"
#include "language/AST/IRGenOptions.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/SourceLoc.h"
#include "language/IRGen/Linking.h"
#include "toolchain/IR/Instructions.h"
#include "toolchain/IR/Function.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/BinaryFormat/MachO.h"

#include "Callee.h"
#include "Explosion.h"
#include "GenPointerAuth.h"
#include "IRGenDebugInfo.h"
#include "IRGenFunction.h"
#include "IRGenModule.h"
#include "LoadableTypeInfo.h"

using namespace language;
using namespace irgen;

static toolchain::cl::opt<bool> EnableTrapDebugInfo(
    "enable-trap-debug-info", toolchain::cl::init(true), toolchain::cl::Hidden,
    toolchain::cl::desc("Generate failure-message functions in the debug info"));

IRGenFunction::IRGenFunction(IRGenModule &IGM, toolchain::Function *Fn,
                             bool isPerformanceConstraint,
                             OptimizationMode OptMode,
                             const SILDebugScope *DbgScope,
                             std::optional<SILLocation> DbgLoc)
    : IGM(IGM), Builder(IGM.getLLVMContext(),
                        IGM.DebugInfo && !IGM.Context.LangOpts.DebuggerSupport),
      OptMode(OptMode), isPerformanceConstraint(isPerformanceConstraint),
      CurFn(Fn), DbgScope(DbgScope) {

  // Make sure the instructions in this function are attached its debug scope.
  if (IGM.DebugInfo) {
    // Functions, especially artificial thunks and closures, are often
    // generated on-the-fly while we are in the middle of another
    // function. Be nice and preserve the current debug location until
    // after we're done with this function.
    IGM.DebugInfo->pushLoc();
  }

  emitPrologue();
}

IRGenFunction::~IRGenFunction() {
  // Move the trap basic blocks to the end of the function.
  for (auto *FailBB : FailBBs) {
    CurFn->splice(CurFn->end(), CurFn, FailBB->getIterator());
  }

  emitEpilogue();

  // Restore the debug location.
  if (IGM.DebugInfo) IGM.DebugInfo->popLoc();

  // Tear down any side-table data structures.
  if (LocalTypeData) destroyLocalTypeData();

  // All dynamically allocated metadata should have been cleaned up.
}

OptimizationMode IRGenFunction::getEffectiveOptimizationMode() const {
  if (OptMode != OptimizationMode::NotSet)
    return OptMode;

  return IGM.getOptions().OptMode;
}

bool IRGenFunction::canStackPromotePackMetadata() const {
  return IGM.getSILModule().getOptions().EnablePackMetadataStackPromotion &&
         !packMetadataStackPromotionDisabled;
}

bool IRGenFunction::outliningCanCallValueWitnesses() const {
  if (!IGM.getOptions().UseTypeLayoutValueHandling)
    return false;
  return !isPerformanceConstraint && !IGM.Context.LangOpts.hasFeature(Feature::Embedded);
}

ModuleDecl *IRGenFunction::getCodiraModule() const {
  return IGM.getCodiraModule();
}

SILModule &IRGenFunction::getSILModule() const {
  return IGM.getSILModule();
}

Lowering::TypeConverter &IRGenFunction::getSILTypes() const {
  return IGM.getSILTypes();
}

const IRGenOptions &IRGenFunction::getOptions() const {
  return IGM.getOptions();
}

// Returns the default atomicity of the module.
Atomicity IRGenFunction::getDefaultAtomicity() {
  return getSILModule().isDefaultAtomic() ? Atomicity::Atomic : Atomicity::NonAtomic;
}

/// Call the toolchain.memcpy intrinsic.  The arguments need not already
/// be of i8* type.
void IRGenFunction::emitMemCpy(toolchain::Value *dest, toolchain::Value *src,
                               Size size, Alignment align) {
  emitMemCpy(dest, src, IGM.getSize(size), align);
}

void IRGenFunction::emitMemCpy(toolchain::Value *dest, toolchain::Value *src,
                               toolchain::Value *size, Alignment align) {
  Builder.CreateMemCpy(dest, toolchain::MaybeAlign(align.getValue()), src,
                       toolchain::MaybeAlign(align.getValue()), size);
}

void IRGenFunction::emitMemCpy(Address dest, Address src, Size size) {
  emitMemCpy(dest, src, IGM.getSize(size));
}

void IRGenFunction::emitMemCpy(Address dest, Address src, toolchain::Value *size) {
  // Map over to the inferior design of the LLVM intrinsic.
  emitMemCpy(dest.getAddress(), src.getAddress(), size,
             std::min(dest.getAlignment(), src.getAlignment()));
}

static toolchain::Value *emitAllocatingCall(IRGenFunction &IGF, FunctionPointer fn,
                                       ArrayRef<toolchain::Value *> args,
                                       const toolchain::Twine &name) {
  auto allocAttrs = IGF.IGM.getAllocAttrs();
  toolchain::CallInst *call =
      IGF.Builder.CreateCall(fn, toolchain::ArrayRef(args.begin(), args.size()));
  call->setAttributes(allocAttrs);
  return call;
}

/// Emit a 'raw' allocation, which has no heap pointer and is
/// not guaranteed to be zero-initialized.
toolchain::Value *IRGenFunction::emitAllocRawCall(toolchain::Value *size,
                                             toolchain::Value *alignMask,
                                             const toolchain::Twine &name) {
  // For now, all we have is language_slowAlloc.
  return emitAllocatingCall(*this, IGM.getSlowAllocFunctionPointer(),
                            {size, alignMask}, name);
}

/// Emit a heap allocation.
toolchain::Value *IRGenFunction::emitAllocObjectCall(toolchain::Value *metadata,
                                                toolchain::Value *size,
                                                toolchain::Value *alignMask,
                                                const toolchain::Twine &name) {
  // For now, all we have is language_allocObject.
  return emitAllocatingCall(*this, IGM.getAllocObjectFunctionPointer(),
                            {metadata, size, alignMask}, name);
}

toolchain::Value *IRGenFunction::emitInitStackObjectCall(toolchain::Value *metadata,
                                                    toolchain::Value *object,
                                                    const toolchain::Twine &name) {
  toolchain::CallInst *call = Builder.CreateCall(
      IGM.getInitStackObjectFunctionPointer(), {metadata, object}, name);
  call->setDoesNotThrow();
  return call;
}

toolchain::Value *IRGenFunction::emitInitStaticObjectCall(toolchain::Value *metadata,
                                                     toolchain::Value *object,
                                                     const toolchain::Twine &name) {
  toolchain::CallInst *call = Builder.CreateCall(
      IGM.getInitStaticObjectFunctionPointer(), {metadata, object}, name);
  call->setDoesNotThrow();
  return call;
}

toolchain::Value *IRGenFunction::emitVerifyEndOfLifetimeCall(toolchain::Value *object,
                                                      const toolchain::Twine &name) {
  toolchain::CallInst *call = Builder.CreateCall(
      IGM.getVerifyEndOfLifetimeFunctionPointer(), {object}, name);
  call->setDoesNotThrow();
  return call;
}

void IRGenFunction::emitAllocBoxCall(toolchain::Value *typeMetadata,
                                      toolchain::Value *&box,
                                      toolchain::Value *&valueAddress) {
  toolchain::CallInst *call =
      Builder.CreateCall(IGM.getAllocBoxFunctionPointer(), typeMetadata);
  call->addFnAttr(toolchain::Attribute::NoUnwind);

  box = Builder.CreateExtractValue(call, 0);
  valueAddress = Builder.CreateExtractValue(call, 1);
}

void IRGenFunction::emitMakeBoxUniqueCall(toolchain::Value *box,
                                          toolchain::Value *typeMetadata,
                                          toolchain::Value *alignMask,
                                          toolchain::Value *&outBox,
                                          toolchain::Value *&outValueAddress) {
  toolchain::CallInst *call = Builder.CreateCall(
      IGM.getMakeBoxUniqueFunctionPointer(), {box, typeMetadata, alignMask});
  call->addFnAttr(toolchain::Attribute::NoUnwind);

  outBox = Builder.CreateExtractValue(call, 0);
  outValueAddress = Builder.CreateExtractValue(call, 1);
}


void IRGenFunction::emitDeallocBoxCall(toolchain::Value *box,
                                        toolchain::Value *typeMetadata) {
  toolchain::CallInst *call =
      Builder.CreateCall(IGM.getDeallocBoxFunctionPointer(), box);
  call->setCallingConv(IGM.DefaultCC);
  call->addFnAttr(toolchain::Attribute::NoUnwind);
}

toolchain::Value *IRGenFunction::emitProjectBoxCall(toolchain::Value *box,
                                                toolchain::Value *typeMetadata) {
  toolchain::CallInst *call =
      Builder.CreateCall(IGM.getProjectBoxFunctionPointer(), box);
  call->setCallingConv(IGM.DefaultCC);
  call->addFnAttr(toolchain::Attribute::NoUnwind);
  return call;
}

toolchain::Value *IRGenFunction::emitAllocEmptyBoxCall() {
  toolchain::CallInst *call =
      Builder.CreateCall(IGM.getAllocEmptyBoxFunctionPointer(), {});
  call->setCallingConv(IGM.DefaultCC);
  call->addFnAttr(toolchain::Attribute::NoUnwind);
  return call;
}

static void emitDeallocatingCall(IRGenFunction &IGF, FunctionPointer fn,
                                 std::initializer_list<toolchain::Value *> args) {
  toolchain::CallInst *call =
      IGF.Builder.CreateCall(fn, toolchain::ArrayRef(args.begin(), args.size()));
  call->setDoesNotThrow();
}

/// Emit a 'raw' deallocation, which has no heap pointer and is not
/// guaranteed to be zero-initialized.
void IRGenFunction::emitDeallocRawCall(toolchain::Value *pointer,
                                       toolchain::Value *size,
                                       toolchain::Value *alignMask) {
  // For now, all we have is language_slowDealloc.
  return emitDeallocatingCall(*this, IGM.getSlowDeallocFunctionPointer(),
                              {pointer, size, alignMask});
}

void IRGenFunction::emitTSanInoutAccessCall(toolchain::Value *address) {
  auto fn = IGM.getTSanInoutAccessFunctionPointer();

  toolchain::Value *castAddress = Builder.CreateBitCast(address, IGM.Int8PtrTy);

  // Passing 0 as the caller PC causes compiler-rt to get our PC.
  toolchain::Value *callerPC = toolchain::ConstantPointerNull::get(IGM.Int8PtrTy);

  // A magic number agreed upon with compiler-rt to indicate a modifying
  // access.
  const unsigned kExternalTagCodiraModifyingAccess = 0x1;
  toolchain::Value *tagValue =
      toolchain::ConstantInt::get(IGM.SizeTy, kExternalTagCodiraModifyingAccess);
  toolchain::Value *castTag = Builder.CreateIntToPtr(tagValue, IGM.Int8PtrTy);

  Builder.CreateCall(fn, {castAddress, callerPC, castTag});
}

// This is shamelessly copied from clang's codegen. We need to get the clang
// functionality into a shared header so that platforms only
// needs to be updated in one place.
static unsigned getBaseMachOPlatformID(const toolchain::Triple &TT) {
  switch (TT.getOS()) {
  case toolchain::Triple::Darwin:
  case toolchain::Triple::MacOSX:
    return toolchain::MachO::PLATFORM_MACOS;
  case toolchain::Triple::IOS:
    return toolchain::MachO::PLATFORM_IOS;
  case toolchain::Triple::TvOS:
    return toolchain::MachO::PLATFORM_TVOS;
  case toolchain::Triple::WatchOS:
    return toolchain::MachO::PLATFORM_WATCHOS;
  case toolchain::Triple::XROS:
    return toolchain::MachO::PLATFORM_XROS;
  default:
    return /*Unknown platform*/ 0;
  }
}

toolchain::Value *
IRGenFunction::emitTargetOSVersionAtLeastCall(toolchain::Value *major,
                                              toolchain::Value *minor,
                                              toolchain::Value *patch) {
  auto fn = IGM.getPlatformVersionAtLeastFunctionPointer();

  toolchain::Value *platformID =
    toolchain::ConstantInt::get(IGM.Int32Ty, getBaseMachOPlatformID(IGM.Triple));
  return Builder.CreateCall(fn, {platformID, major, minor, patch});
}

toolchain::Value *
IRGenFunction::emitTargetVariantOSVersionAtLeastCall(toolchain::Value *major,
                                                     toolchain::Value *minor,
                                                     toolchain::Value *patch) {
  auto *fn = cast<toolchain::Function>(IGM.getPlatformVersionAtLeastFn());

  toolchain::Value *iOSPlatformID =
    toolchain::ConstantInt::get(IGM.Int32Ty, toolchain::MachO::PLATFORM_IOS);
  return Builder.CreateCall(fn->getFunctionType(), fn, {iOSPlatformID, major, minor, patch});
}

toolchain::Value *
IRGenFunction::emitTargetOSVersionOrVariantOSVersionAtLeastCall(
    toolchain::Value *major, toolchain::Value *minor, toolchain::Value *patch,
    toolchain::Value *variantMajor, toolchain::Value *variantMinor,
    toolchain::Value *variantPatch) {
  auto *fn = cast<toolchain::Function>(
      IGM.getTargetOSVersionOrVariantOSVersionAtLeastFn());

  toolchain::Value *macOSPlatformID =
      toolchain::ConstantInt::get(IGM.Int32Ty, toolchain::MachO::PLATFORM_MACOS);

  toolchain::Value *iOSPlatformID =
    toolchain::ConstantInt::get(IGM.Int32Ty, toolchain::MachO::PLATFORM_IOS);

  return Builder.CreateCall(fn->getFunctionType(),
                            fn, {macOSPlatformID, major, minor, patch,
                                 iOSPlatformID, variantMajor, variantMinor,
                                 variantPatch});
}



/// Initialize a relative indirectable pointer to the given value.
/// This always leaves the value in the direct state; if it's not a
/// far reference, it's the caller's responsibility to ensure that the
/// pointer ranges are sufficient.
void IRGenFunction::emitStoreOfRelativeIndirectablePointer(toolchain::Value *value,
                                                           Address addr,
                                                           bool isFar) {
  value = Builder.CreatePtrToInt(value, IGM.IntPtrTy);
  auto addrAsInt =
    Builder.CreatePtrToInt(addr.getAddress(), IGM.IntPtrTy);

  auto difference = Builder.CreateSub(value, addrAsInt);
  if (!isFar) {
    difference = Builder.CreateTrunc(difference, IGM.RelativeAddressTy);
  }

  Builder.CreateStore(difference, addr);
}

toolchain::Value *
IRGenFunction::emitLoadOfRelativePointer(Address addr, bool isFar,
                                         toolchain::Type *expectedPointedToType,
                                         const toolchain::Twine &name) {
  toolchain::Value *value = Builder.CreateLoad(addr);
  assert(value->getType() ==
         (isFar ? IGM.FarRelativeAddressTy : IGM.RelativeAddressTy));
  if (!isFar) {
    value = Builder.CreateSExt(value, IGM.IntPtrTy);
  }
  auto *addrInt = Builder.CreatePtrToInt(addr.getAddress(), IGM.IntPtrTy);
  auto *uncastPointerInt = Builder.CreateAdd(addrInt, value);
  auto *uncastPointer = Builder.CreateIntToPtr(uncastPointerInt, IGM.Int8PtrTy);
  auto uncastPointerAddress =
      Address(uncastPointer, IGM.Int8Ty, IGM.getPointerAlignment());
  auto pointer =
      Builder.CreateElementBitCast(uncastPointerAddress, expectedPointedToType);
  return pointer.getAddress();
}

toolchain::Value *IRGenFunction::emitLoadOfCompactFunctionPointer(
    Address addr, bool isFar, toolchain::Type *expectedPointedToType,
    const toolchain::Twine &name) {
  if (IGM.getOptions().CompactAbsoluteFunctionPointer) {
    toolchain::Value *value = Builder.CreateLoad(addr);
    auto *uncastPointer = Builder.CreateIntToPtr(value, IGM.Int8PtrTy);
    auto pointer = Builder.CreateElementBitCast(
        Address(uncastPointer, IGM.Int8Ty, IGM.getPointerAlignment()),
        expectedPointedToType);
    return pointer.getAddress();
  } else {
    return emitLoadOfRelativePointer(addr, isFar, expectedPointedToType, name);
  }
}

void IRGenFunction::emitFakeExplosion(const TypeInfo &type,
                                      Explosion &explosion) {
  if (!isa<LoadableTypeInfo>(type)) {
    explosion.add(toolchain::UndefValue::get(type.getStorageType()->getPointerTo()));
    return;
  }

  ExplosionSchema schema = cast<LoadableTypeInfo>(type).getSchema();
  for (auto &element : schema) {
    toolchain::Type *elementType;
    if (element.isAggregate()) {
      elementType = element.getAggregateType()->getPointerTo();
    } else {
      elementType = element.getScalarType();
    }
    
    explosion.add(toolchain::UndefValue::get(elementType));
  }
}

void IRGenFunction::unimplemented(SourceLoc Loc, StringRef Message) {
  return IGM.unimplemented(Loc, Message);
}

// Debug output for Explosions.

void Explosion::print(toolchain::raw_ostream &OS) {
  for (auto value : toolchain::ArrayRef(Values).slice(NextValue)) {
    value->print(OS);
    OS << '\n';
  }
}

void Explosion::dump() {
  print(toolchain::errs());
}

toolchain::Value *Offset::getAsValue(IRGenFunction &IGF) const {
  if (isStatic()) {
    return IGF.IGM.getSize(getStatic());
  } else {
    return getDynamic();
  }
}

Offset Offset::offsetBy(IRGenFunction &IGF, Size other) const {
  if (isStatic()) {
    return Offset(getStatic() + other);
  }
  auto otherVal = toolchain::ConstantInt::get(IGF.IGM.SizeTy, other.getValue());
  return Offset(IGF.Builder.CreateAdd(getDynamic(), otherVal));
}

Address IRGenFunction::emitAddressAtOffset(toolchain::Value *base, Offset offset,
                                           toolchain::Type *objectTy,
                                           Alignment objectAlignment,
                                           const toolchain::Twine &name) {
  // Use a slightly more obvious IR pattern if it's a multiple of the type
  // size.  I'll confess that this is partly just to avoid updating tests.
  if (offset.isStatic()) {
    auto byteOffset = offset.getStatic();
    Size objectSize(IGM.DataLayout.getTypeAllocSize(objectTy));
    if (byteOffset.isMultipleOf(objectSize)) {
      // Cast to T*.
      auto objectPtrTy = objectTy->getPointerTo();
      base = Builder.CreateBitCast(base, objectPtrTy);

      // GEP to the slot, computing the index as a signed number.
      auto scaledIndex =
        int64_t(byteOffset.getValue()) / int64_t(objectSize.getValue());
      auto indexValue = IGM.getSize(Size(scaledIndex));
      auto slotPtr = Builder.CreateInBoundsGEP(objectTy, base, indexValue);

      return Address(slotPtr, objectTy, objectAlignment);
    }
  }

  // GEP to the slot.
  auto offsetValue = offset.getAsValue(*this);
  auto slotPtr = emitByteOffsetGEP(base, offsetValue, objectTy);
  return Address(slotPtr, objectTy, objectAlignment);
}

toolchain::CallInst *IRBuilder::CreateExpectCond(IRGenModule &IGM,
                                            toolchain::Value *value,
                                            bool expectedValue,
                                            const Twine &name) {
  unsigned flag = expectedValue ? 1 : 0;
  return CreateExpect(value, toolchain::ConstantInt::get(IGM.Int1Ty, flag), name);
}

toolchain::CallInst *IRBuilder::CreateNonMergeableTrap(IRGenModule &IGM,
                                                  StringRef failureMsg) {
  if (IGM.DebugInfo && IGM.getOptions().isDebugInfoCodeView()) {
    auto TrapLoc = getCurrentDebugLocation();
    // Line 0 is invalid in CodeView, so create a new location that uses the
    // line and column from the inlined location of the trap, that should
    // correspond to its original source location.
    if (TrapLoc.getLine() == 0 && TrapLoc.getInlinedAt()) {
      auto DL = toolchain::DILocation::getDistinct(
          IGM.getLLVMContext(), TrapLoc.getInlinedAt()->getLine(),
          TrapLoc.getInlinedAt()->getColumn(), TrapLoc.getScope(),
          TrapLoc.getInlinedAt());
      SetCurrentDebugLocation(DL);
    }
  }

  if (IGM.IRGen.Opts.shouldOptimize() && !IGM.IRGen.Opts.MergeableTraps) {
    // Emit unique side-effecting inline asm calls in order to eliminate
    // the possibility that an LLVM optimization or code generation pass
    // will merge these blocks back together again. We emit an empty asm
    // string with the side-effect flag set, and with a unique integer
    // argument for each cond_fail we see in the function.
    toolchain::IntegerType *asmArgTy = IGM.Int32Ty;
    toolchain::Type *argTys = {asmArgTy};
    toolchain::FunctionType *asmFnTy =
        toolchain::FunctionType::get(IGM.VoidTy, argTys, false /* = isVarArg */);
    toolchain::InlineAsm *inlineAsm =
        toolchain::InlineAsm::get(asmFnTy, "", "n", true /* = SideEffects */);
    CreateAsmCall(inlineAsm,
                  toolchain::ConstantInt::get(asmArgTy, NumTrapBarriers++));
  }

  // Emit the trap instruction.
  toolchain::Function *trapIntrinsic =
      toolchain::Intrinsic::getDeclaration(&IGM.Module, toolchain::Intrinsic::trap);
  if (EnableTrapDebugInfo && IGM.DebugInfo && !failureMsg.empty()) {
    IGM.DebugInfo->addFailureMessageToCurrentLoc(*this, failureMsg);
  }
  auto Call = IRBuilderBase::CreateCall(trapIntrinsic, {});
  setCallingConvUsingCallee(Call);

  if (!IGM.IRGen.Opts.TrapFuncName.empty()) {
    auto A = toolchain::Attribute::get(getContext(), "trap-fn-name",
                                  IGM.IRGen.Opts.TrapFuncName);
    Call->addFnAttr(A);
  }

  return Call;
}

void IRGenFunction::emitTrap(StringRef failureMessage, bool EmitUnreachable) {
  Builder.CreateNonMergeableTrap(IGM, failureMessage);
  if (EmitUnreachable)
    Builder.CreateUnreachable();
}

void IRGenFunction::emitConditionalTrap(toolchain::Value *condition, StringRef failureMessage,
                                        const SILDebugScope *debugScope) {
  // The condition should be false, or we die.
  auto expectedCond = Builder.CreateExpect(condition,
                                           toolchain::ConstantInt::get(IGM.Int1Ty, 0));

  // Emit individual fail blocks so that we can map the failure back to a source
  // line.
  auto origInsertionPoint = Builder.GetInsertBlock();

  toolchain::BasicBlock *failBB = toolchain::BasicBlock::Create(IGM.getLLVMContext());
  toolchain::BasicBlock *contBB = toolchain::BasicBlock::Create(IGM.getLLVMContext());
  auto br = Builder.CreateCondBr(expectedCond, failBB, contBB);

  if (IGM.getOptions().AnnotateCondFailMessage && !failureMessage.empty())
    br->addAnnotationMetadata(failureMessage);

  Builder.SetInsertPoint(&CurFn->back());
  Builder.emitBlock(failBB);
  if (IGM.DebugInfo && debugScope) {
    // If we are emitting DWARF, this does nothing. Otherwise the ``toolchain.trap``
    // instruction emitted from ``Builtin.condfail`` should have an inlined
    // debug location. This is because zero is not an artificial line location
    // in CodeView.
    IGM.DebugInfo->setInlinedTrapLocation(Builder, debugScope);
  }
  emitTrap(failureMessage, /*EmitUnreachable=*/true);

  Builder.SetInsertPoint(origInsertionPoint);
  Builder.emitBlock(contBB);
  FailBBs.push_back(failBB);
}

Address IRGenFunction::emitTaskAlloc(toolchain::Value *size, Alignment alignment) {
  auto *call = Builder.CreateCall(IGM.getTaskAllocFunctionPointer(), {size});
  call->setDoesNotThrow();
  call->setCallingConv(IGM.CodiraCC);
  auto address = Address(call, IGM.Int8Ty, alignment);
  return address;
}

void IRGenFunction::emitTaskDealloc(Address address) {
  auto *call = Builder.CreateCall(IGM.getTaskDeallocFunctionPointer(),
                                  {address.getAddress()});
  call->setDoesNotThrow();
  call->setCallingConv(IGM.CodiraCC);
}

toolchain::Value *IRGenFunction::alignUpToMaximumAlignment(toolchain::Type *sizeTy, toolchain::Value *val) {
  auto *alignMask = toolchain::ConstantInt::get(sizeTy, MaximumAlignment - 1);
  auto *invertedMask = Builder.CreateNot(alignMask);
  return Builder.CreateAnd(Builder.CreateAdd(val, alignMask), invertedMask);
}

/// Returns the current task \p currTask as a Builtin.RawUnsafeContinuation at +1.
static toolchain::Value *unsafeContinuationFromTask(IRGenFunction &IGF,
                                               toolchain::Value *currTask) {
  auto &IGM = IGF.IGM;
  auto &Builder = IGF.Builder;

  auto &rawPointerTI = IGM.getRawUnsafeContinuationTypeInfo();
  return Builder.CreateBitOrPointerCast(currTask, rawPointerTI.getStorageType());
}

static toolchain::Value *emitLoadOfResumeContextFromTask(IRGenFunction &IGF,
                                                    toolchain::Value *task) {
  // Task.ResumeContext is at field index 8 within CodiraTaskTy. The offset comes
  // from 7 pointers (two within the single RefCountedStructTy) and 2 Int32
  // fields.
  const unsigned taskResumeContextIndex = 8;
  const Size taskResumeContextOffset = (7 * IGF.IGM.getPointerSize()) + Size(8);

  auto addr = Address(task, IGF.IGM.CodiraTaskTy, IGF.IGM.getPointerAlignment());
  auto resumeContextAddr = IGF.Builder.CreateStructGEP(
    addr, taskResumeContextIndex, taskResumeContextOffset);
  toolchain::Value *resumeContext = IGF.Builder.CreateLoad(resumeContextAddr);
  if (auto &schema = IGF.getOptions().PointerAuth.TaskResumeContext) {
    auto info = PointerAuthInfo::emit(IGF, schema,
                                      resumeContextAddr.getAddress(),
                                      PointerAuthEntity());
    resumeContext = emitPointerAuthAuth(IGF, resumeContext, info);
  }
  return resumeContext;
}

static Address emitLoadOfContinuationContext(IRGenFunction &IGF,
                                             toolchain::Value *continuation) {
  auto ptr = emitLoadOfResumeContextFromTask(IGF, continuation);
  ptr = IGF.Builder.CreateBitCast(ptr, IGF.IGM.ContinuationAsyncContextPtrTy);
  return Address(ptr, IGF.IGM.ContinuationAsyncContextTy,
                 IGF.IGM.getAsyncContextAlignment());
}

static Address emitAddrOfContinuationNormalResultPointer(IRGenFunction &IGF,
                                                         Address context) {
  assert(context.getType() == IGF.IGM.ContinuationAsyncContextPtrTy);
  auto offset = 5 * IGF.IGM.getPointerSize();
  return IGF.Builder.CreateStructGEP(context, 4, offset);
}

static Address emitAddrOfContinuationErrorResultPointer(IRGenFunction &IGF,
                                                        Address context) {
  assert(context.getType() == IGF.IGM.ContinuationAsyncContextPtrTy);
  auto offset = 4 * IGF.IGM.getPointerSize();
  return IGF.Builder.CreateStructGEP(context, 3, offset);
}

void IRGenFunction::emitGetAsyncContinuation(SILType resumeTy,
                                             StackAddress resultAddr,
                                             Explosion &out,
                                             bool canThrow) {
  // A continuation is just a reference to the current async task,
  // parked with a special context:
  //
  // struct ContinuationAsyncContext : AsyncContext {
  //   std::atomic<size_t> awaitSynchronization;
  //   CodiraError *errResult;
  //   Result *result;
  //   SerialExecutorRef resumeExecutor;
  // };
  //
  // We need fill out this context essentially as if we were calling
  // something.

  // Create and setup the continuation context.
  auto continuationContext =
    createAlloca(IGM.ContinuationAsyncContextTy,
                 IGM.getAsyncContextAlignment());
  AsyncCoroutineCurrentContinuationContext = continuationContext.getAddress();
  // TODO: add lifetime with matching lifetime in await_async_continuation

  // We're required to initialize three fields in the continuation
  // context before calling language_continuation_init:

  // - Parent, the parent context pointer, which we initialize to
  //   the current context.
  auto contextBase = Builder.CreateStructGEP(continuationContext, 0, Size(0));
  auto parentContextAddr = Builder.CreateStructGEP(contextBase, 0, Size(0));
  toolchain::Value *asyncContextValue =
    Builder.CreateBitCast(getAsyncContext(), IGM.CodiraContextPtrTy);
  if (auto schema = IGM.getOptions().PointerAuth.AsyncContextParent) {
    auto authInfo = PointerAuthInfo::emit(*this, schema,
                                          parentContextAddr.getAddress(),
                                          PointerAuthEntity());
    asyncContextValue = emitPointerAuthSign(*this, asyncContextValue, authInfo);
  }
  Builder.CreateStore(asyncContextValue, parentContextAddr);

  // - NormalResult, the pointer to the normal result, which we initialize
  //   to the result address that we were given, or else a temporary slot.
  //   TODO: emit lifetime.start for this temporary, paired with a
  //   lifetime.end within the await after we take from the slot.
  auto normalResultAddr =
    emitAddrOfContinuationNormalResultPointer(*this, continuationContext);
  if (!resultAddr.getAddress().isValid()) {
    auto &resumeTI = getTypeInfo(resumeTy);
    resultAddr =
      resumeTI.allocateStack(*this, resumeTy, "async.continuation.result");
  }
  Builder.CreateStore(Builder.CreateBitOrPointerCast(
                            resultAddr.getAddress().getAddress(),
                            IGM.OpaquePtrTy),
                      normalResultAddr);

  // - ResumeParent, the continuation function pointer, which we initialize
  //   with the result of a new call to @toolchain.coro.async.resume; we'll pair
  //   this with a suspend point when we emit the corresponding
  //   await_async_continuation.
  auto coroResume =
    Builder.CreateIntrinsicCall(toolchain::Intrinsic::coro_async_resume, {});
  auto resumeFunctionAddr =
    Builder.CreateStructGEP(contextBase, 1, IGM.getPointerSize());
  toolchain::Value *coroResumeValue =
    Builder.CreateBitOrPointerCast(coroResume,
                                   IGM.TaskContinuationFunctionPtrTy);
  if (auto schema = IGM.getOptions().PointerAuth.AsyncContextResume) {
    auto authInfo = PointerAuthInfo::emit(*this, schema,
                                          resumeFunctionAddr.getAddress(),
                                          PointerAuthEntity());
    coroResumeValue = emitPointerAuthSign(*this, coroResumeValue, authInfo);
  }
  Builder.CreateStore(coroResumeValue, resumeFunctionAddr);

  // Save the resume intrinsic call for await_async_continuation.
  assert(AsyncCoroutineCurrentResume == nullptr &&
         "Don't support nested get_async_continuation");
  AsyncCoroutineCurrentResume = coroResume;

  AsyncContinuationFlags flags;
  if (canThrow) flags.setCanThrow(true);

  // Call the language_continuation_init runtime function to initialize
  // the rest of the continuation and return the task pointer back to us.
  auto task = Builder.CreateCall(IGM.getContinuationInitFunctionPointer(),
                                 {continuationContext.getAddress(),
                                  IGM.getSize(Size(flags.getOpaqueValue()))});
  task->setCallingConv(IGM.CodiraCC);

  // TODO: if we have a better idea of what executor to return to than
  // the current executor, overwrite the ResumeToExecutor field.

  auto unsafeContinuation = unsafeContinuationFromTask(*this, task);
  out.add(unsafeContinuation);
}

static bool shouldUseContinuationAwait(IRGenModule &IGM) {
  auto &ctx = IGM.Context;
  auto module = ctx.getLoadedModule(ctx.Id_Concurrency);
  assert(module && "building async code without concurrency library");
  SmallVector<ValueDecl *, 1> results;
  module->lookupValue(ctx.getIdentifier("_abiEnableAwaitContinuation"),
                      NLKind::UnqualifiedLookup, results);
  assert(results.size() <= 1);
  return !results.empty();
}

void IRGenFunction::emitAwaitAsyncContinuation(
    SILType resumeTy, bool isIndirectResult,
    Explosion &outDirectResult, toolchain::BasicBlock *&normalBB,
    toolchain::PHINode *&optionalErrorResult, toolchain::BasicBlock *&optionalErrorBB) {
  assert(AsyncCoroutineCurrentContinuationContext && "no active continuation");
  Address continuationContext(AsyncCoroutineCurrentContinuationContext,
                              IGM.ContinuationAsyncContextTy,
                              IGM.getAsyncContextAlignment());

  // Call language_continuation_await to check whether the continuation
  // has already been resumed.
  bool useContinuationAwait = shouldUseContinuationAwait(IGM);

  // As a temporary hack for compatibility with SDKs that don't provide
  // language_continuation_await, emit the old inline sequence.  This can
  // be removed as soon as we're sure that such SDKs don't exist.
  if (!useContinuationAwait) {
    auto contAwaitSyncAddr =
      Builder.CreateStructGEP(continuationContext, 2,
                              3 * IGM.getPointerSize()).getAddress();

    auto pendingV = toolchain::ConstantInt::get(
        IGM.SizeTy, unsigned(ContinuationStatus::Pending));
    auto awaitedV = toolchain::ConstantInt::get(
        IGM.SizeTy, unsigned(ContinuationStatus::Awaited));
    auto results = Builder.CreateAtomicCmpXchg(
        contAwaitSyncAddr, pendingV, awaitedV, toolchain::MaybeAlign(),
        toolchain::AtomicOrdering::Release /*success ordering*/,
        toolchain::AtomicOrdering::Acquire /* failure ordering */,
        toolchain::SyncScope::System);
    auto firstAtAwait = Builder.CreateExtractValue(results, 1);
    auto contBB = createBasicBlock("await.async.resume");
    auto abortBB = createBasicBlock("await.async.abort");
    Builder.CreateCondBr(firstAtAwait, abortBB, contBB);
    Builder.emitBlock(abortBB);
    {
      // We were the first to the sync point. "Abort" (return from the
      // coroutine partial function, without making a tail call to anything)
      // because the continuation result is not available yet. When the
      // continuation is later resumed, the task will get scheduled
      // starting from the suspension point.
      emitCoroutineOrAsyncExit(false);
    }

    Builder.emitBlock(contBB);
  }

  {
    // Set up the suspend point.
    SmallVector<toolchain::Value *, 8> arguments;
    unsigned languageAsyncContextIndex = 0;
    arguments.push_back(IGM.getInt32(languageAsyncContextIndex)); // context index
    arguments.push_back(AsyncCoroutineCurrentResume);
    auto resumeProjFn = getOrCreateResumePrjFn();
    arguments.push_back(
        Builder.CreateBitOrPointerCast(resumeProjFn, IGM.Int8PtrTy));

    toolchain::Constant *awaitFnPtr;
    if (useContinuationAwait) {
      awaitFnPtr = IGM.getAwaitAsyncContinuationFn();
    } else {
      auto resumeFnPtr =
        getFunctionPointerForResumeIntrinsic(AsyncCoroutineCurrentResume);
      awaitFnPtr = createAsyncDispatchFn(resumeFnPtr, {IGM.Int8PtrTy});
    }
    arguments.push_back(
        Builder.CreateBitOrPointerCast(awaitFnPtr, IGM.Int8PtrTy));

    if (useContinuationAwait) {
      arguments.push_back(continuationContext.getAddress());
    } else {
      arguments.push_back(AsyncCoroutineCurrentResume);
      arguments.push_back(Builder.CreateBitOrPointerCast(
        continuationContext.getAddress(), IGM.Int8PtrTy));
    }

    auto resultTy =
        toolchain::StructType::get(IGM.getLLVMContext(), {IGM.Int8PtrTy}, false /*packed*/);
    emitSuspendAsyncCall(languageAsyncContextIndex, resultTy, arguments);
  }

  // If there's an error destination (i.e. if the continuation is throwing),
  // load the error value out and check whether it's null.  If so, branch
  // to the error destination.
  if (optionalErrorBB) {
    auto normalContBB = createBasicBlock("await.async.normal");
    auto contErrResultAddr =
        emitAddrOfContinuationErrorResultPointer(*this, continuationContext);
    auto errorRes = Builder.CreateLoad(contErrResultAddr);
    auto nullError = toolchain::Constant::getNullValue(errorRes->getType());
    auto hasError = Builder.CreateICmpNE(errorRes, nullError);
    optionalErrorResult->addIncoming(errorRes, Builder.GetInsertBlock());

    // Predict no error.
    hasError =
        getSILModule().getOptions().EnableThrowsPrediction ?
        Builder.CreateExpectCond(IGM, hasError, false) : hasError;

    Builder.CreateCondBr(hasError, optionalErrorBB, normalContBB);
    Builder.emitBlock(normalContBB);
  }

  // We're now on the normal-result path.  If we didn't have an indirect
  // result slot, load from the temporary we created during
  // get_async_continuation.
  if (!isIndirectResult) {
    auto contResultAddrAddr =
        emitAddrOfContinuationNormalResultPointer(*this, continuationContext);
    auto resultAddrVal =
        Builder.CreateLoad(contResultAddrAddr);
    // Take the result.
    auto &resumeTI = cast<LoadableTypeInfo>(getTypeInfo(resumeTy));
    auto resultStorageTy = resumeTI.getStorageType();
    auto resultAddr =
        Address(Builder.CreateBitOrPointerCast(resultAddrVal,
                                               resultStorageTy->getPointerTo()),
                resultStorageTy, resumeTI.getFixedAlignment());
    resumeTI.loadAsTake(*this, resultAddr, outDirectResult);
  }

  Builder.CreateBr(normalBB);
  AsyncCoroutineCurrentResume = nullptr;
  AsyncCoroutineCurrentContinuationContext = nullptr;
}

void IRGenFunction::emitResumeAsyncContinuationReturning(
                        toolchain::Value *continuation, toolchain::Value *srcPtr,
                        SILType valueTy, bool throwing) {
  continuation = Builder.CreateBitCast(continuation, IGM.CodiraTaskPtrTy);
  auto &valueTI = getTypeInfo(valueTy);
  Address srcAddr = valueTI.getAddressForPointer(srcPtr);

  // Extract the destination value pointer and cast it from an opaque
  // pointer type.
  Address context = emitLoadOfContinuationContext(*this, continuation);
  auto destPtrAddr = emitAddrOfContinuationNormalResultPointer(*this, context);
  auto destPtr = Builder.CreateBitCast(Builder.CreateLoad(destPtrAddr),
                                     valueTI.getStorageType()->getPointerTo());
  Address destAddr = valueTI.getAddressForPointer(destPtr);

  valueTI.initializeWithTake(*this, destAddr, srcAddr, valueTy,
                             /*outlined*/ false, /*zeroizeIfSensitive=*/ true);

  auto call = Builder.CreateCall(
      throwing ? IGM.getContinuationThrowingResumeFunctionPointer()
               : IGM.getContinuationResumeFunctionPointer(),
      {continuation});
  call->setCallingConv(IGM.CodiraCC);
}

void IRGenFunction::emitResumeAsyncContinuationThrowing(
                        toolchain::Value *continuation, toolchain::Value *error) {
  continuation = Builder.CreateBitCast(continuation, IGM.CodiraTaskPtrTy);
  auto call = Builder.CreateCall(
      IGM.getContinuationThrowingResumeWithErrorFunctionPointer(),
      {continuation, error});
  call->setCallingConv(IGM.CodiraCC);
}

void IRGenFunction::emitClearSensitive(Address address, toolchain::Value *size) {
  // If our deployment target doesn't contain the new language_clearSensitive,
  // fall back to memset_s
  auto deploymentAvailability =
      AvailabilityRange::forDeploymentTarget(IGM.Context);
  auto clearSensitiveAvail = IGM.Context.getClearSensitiveAvailability();
  if (!deploymentAvailability.isContainedIn(clearSensitiveAvail)) {
    Builder.CreateCall(IGM.getMemsetSFunctionPointer(),
                         {address.getAddress(), size,
                          toolchain::ConstantInt::get(IGM.Int32Ty, 0), size});
    return;
  }
  Builder.CreateCall(IGM.getClearSensitiveFunctionPointer(),
                         {address.getAddress(), size});
}
