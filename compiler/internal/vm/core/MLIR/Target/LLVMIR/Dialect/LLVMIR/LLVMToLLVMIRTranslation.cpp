//===- LLVMToLLVMIRTranslation.cpp - Translate LLVM dialect to LLVM IR ----===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This file implements a translation between the MLIR LLVM dialect and LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Operation.h"
#include "mlir/Interfaces/CallInterfaces.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"

#include "vm/core/ADT/TypeSwitch.h"
#include "vm/core/IR/DIBuilder.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/InlineAsm.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/MDBuilder.h"
#include "vm/core/IR/MatrixBuilder.h"
#include "vm/core/IR/MemoryModelRelaxationAnnotations.h"
#include "vm/core/Support/LogicalResult.h"

using namespace mlir;
using namespace mlir::LLVM;
using mlir::LLVM::detail::getLLVMConstant;

#include "mlir/Dialect/LLVMIR/LLVMConversionEnumsToLLVM.inc"

static toolchain::FastMathFlags getFastmathFlags(FastmathFlagsInterface &op) {
  using llvmFMF = toolchain::FastMathFlags;
  using FuncT = void (llvmFMF::*)(bool);
  const std::pair<FastmathFlags, FuncT> handlers[] = {
      // clang-format off
      {FastmathFlags::nnan,     &llvmFMF::setNoNaNs},
      {FastmathFlags::ninf,     &llvmFMF::setNoInfs},
      {FastmathFlags::nsz,      &llvmFMF::setNoSignedZeros},
      {FastmathFlags::arcp,     &llvmFMF::setAllowReciprocal},
      {FastmathFlags::contract, &llvmFMF::setAllowContract},
      {FastmathFlags::afn,      &llvmFMF::setApproxFunc},
      {FastmathFlags::reassoc,  &llvmFMF::setAllowReassoc},
      // clang-format on
  };
  toolchain::FastMathFlags ret;
  ::mlir::LLVM::FastmathFlags fmfMlir = op.getFastmathAttr().getValue();
  for (auto it : handlers)
    if (bitEnumContainsAll(fmfMlir, it.first))
      (ret.*(it.second))(true);
  return ret;
}

/// Convert the value of a DenseI64ArrayAttr to a vector of unsigned indices.
static SmallVector<unsigned> extractPosition(ArrayRef<int64_t> indices) {
  SmallVector<unsigned> position;
  toolchain::append_range(position, indices);
  return position;
}

/// Convert an LLVM type to a string for printing in diagnostics.
static std::string diagStr(const toolchain::Type *type) {
  std::string str;
  toolchain::raw_string_ostream os(str);
  type->print(os);
  return str;
}

/// Get the declaration of an overloaded toolchain intrinsic. First we get the
/// overloaded argument types and/or result type from the CallIntrinsicOp, and
/// then use those to get the correct declaration of the overloaded intrinsic.
static FailureOr<toolchain::Function *>
getOverloadedDeclaration(CallIntrinsicOp op, toolchain::Intrinsic::ID id,
                         toolchain::Module *module,
                         LLVM::ModuleTranslation &moduleTranslation) {
  SmallVector<toolchain::Type *, 8> allArgTys;
  for (Type type : op->getOperandTypes())
    allArgTys.push_back(moduleTranslation.convertType(type));

  toolchain::Type *resTy;
  if (op.getNumResults() == 0)
    resTy = toolchain::Type::getVoidTy(module->getContext());
  else
    resTy = moduleTranslation.convertType(op.getResult(0).getType());

  // ATM we do not support variadic intrinsics.
  toolchain::FunctionType *ft = toolchain::FunctionType::get(resTy, allArgTys, false);

  SmallVector<toolchain::Intrinsic::IITDescriptor, 8> table;
  getIntrinsicInfoTableEntries(id, table);
  ArrayRef<toolchain::Intrinsic::IITDescriptor> tableRef = table;

  SmallVector<toolchain::Type *, 8> overloadedArgTys;
  if (toolchain::Intrinsic::matchIntrinsicSignature(ft, tableRef,
                                               overloadedArgTys) !=
      toolchain::Intrinsic::MatchIntrinsicTypesResult::MatchIntrinsicTypes_Match) {
    return mlir::emitError(op.getLoc(), "call intrinsic signature ")
           << diagStr(ft) << " to overloaded intrinsic " << op.getIntrinAttr()
           << " does not match any of the overloads";
  }

  ArrayRef<toolchain::Type *> overloadedArgTysRef = overloadedArgTys;
  return toolchain::Intrinsic::getOrInsertDeclaration(module, id,
                                                 overloadedArgTysRef);
}

static toolchain::OperandBundleDef
convertOperandBundle(OperandRange bundleOperands, StringRef bundleTag,
                     LLVM::ModuleTranslation &moduleTranslation) {
  std::vector<toolchain::Value *> operands;
  operands.reserve(bundleOperands.size());
  for (Value bundleArg : bundleOperands)
    operands.push_back(moduleTranslation.lookupValue(bundleArg));
  return toolchain::OperandBundleDef(bundleTag.str(), std::move(operands));
}

static SmallVector<toolchain::OperandBundleDef>
convertOperandBundles(OperandRangeRange bundleOperands, ArrayAttr bundleTags,
                      LLVM::ModuleTranslation &moduleTranslation) {
  SmallVector<toolchain::OperandBundleDef> bundles;
  bundles.reserve(bundleOperands.size());

  for (auto [operands, tagAttr] : toolchain::zip_equal(bundleOperands, bundleTags)) {
    StringRef tag = cast<StringAttr>(tagAttr).getValue();
    bundles.push_back(convertOperandBundle(operands, tag, moduleTranslation));
  }
  return bundles;
}

static SmallVector<toolchain::OperandBundleDef>
convertOperandBundles(OperandRangeRange bundleOperands,
                      std::optional<ArrayAttr> bundleTags,
                      LLVM::ModuleTranslation &moduleTranslation) {
  if (!bundleTags)
    return {};
  return convertOperandBundles(bundleOperands, *bundleTags, moduleTranslation);
}

/// Builder for LLVM_CallIntrinsicOp
static LogicalResult
convertCallLLVMIntrinsicOp(CallIntrinsicOp op, toolchain::IRBuilderBase &builder,
                           LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::Module *module = builder.GetInsertBlock()->getModule();
  toolchain::Intrinsic::ID id =
      toolchain::Intrinsic::lookupIntrinsicID(op.getIntrinAttr());
  if (!id)
    return mlir::emitError(op.getLoc(), "could not find LLVM intrinsic: ")
           << op.getIntrinAttr();

  toolchain::Function *fn = nullptr;
  if (toolchain::Intrinsic::isOverloaded(id)) {
    auto fnOrFailure =
        getOverloadedDeclaration(op, id, module, moduleTranslation);
    if (failed(fnOrFailure))
      return failure();
    fn = *fnOrFailure;
  } else {
    fn = toolchain::Intrinsic::getOrInsertDeclaration(module, id, {});
  }

  // Check the result type of the call.
  const toolchain::Type *intrinType =
      op.getNumResults() == 0
          ? toolchain::Type::getVoidTy(module->getContext())
          : moduleTranslation.convertType(op.getResultTypes().front());
  if (intrinType != fn->getReturnType()) {
    return mlir::emitError(op.getLoc(), "intrinsic call returns ")
           << diagStr(intrinType) << " but " << op.getIntrinAttr()
           << " actually returns " << diagStr(fn->getReturnType());
  }

  // Check the argument types of the call. If the function is variadic, check
  // the subrange of required arguments.
  if (!fn->getFunctionType()->isVarArg() &&
      op.getArgs().size() != fn->arg_size()) {
    return mlir::emitError(op.getLoc(), "intrinsic call has ")
           << op.getArgs().size() << " operands but " << op.getIntrinAttr()
           << " expects " << fn->arg_size();
  }
  if (fn->getFunctionType()->isVarArg() &&
      op.getArgs().size() < fn->arg_size()) {
    return mlir::emitError(op.getLoc(), "intrinsic call has ")
           << op.getArgs().size() << " operands but variadic "
           << op.getIntrinAttr() << " expects at least " << fn->arg_size();
  }
  // Check the arguments up to the number the function requires.
  for (unsigned i = 0, e = fn->arg_size(); i != e; ++i) {
    const toolchain::Type *expected = fn->getArg(i)->getType();
    const toolchain::Type *actual =
        moduleTranslation.convertType(op.getOperandTypes()[i]);
    if (actual != expected) {
      return mlir::emitError(op.getLoc(), "intrinsic call operand #")
             << i << " has type " << diagStr(actual) << " but "
             << op.getIntrinAttr() << " expects " << diagStr(expected);
    }
  }

  FastmathFlagsInterface itf = op;
  builder.setFastMathFlags(getFastmathFlags(itf));

  auto *inst = builder.CreateCall(
      fn, moduleTranslation.lookupValues(op.getArgs()),
      convertOperandBundles(op.getOpBundleOperands(), op.getOpBundleTags(),
                            moduleTranslation));

  if (failed(moduleTranslation.convertArgAndResultAttrs(op, inst)))
    return failure();

  if (op.getNumResults() == 1)
    moduleTranslation.mapValue(op->getResults().front()) = inst;
  return success();
}

static void convertLinkerOptionsOp(ArrayAttr options,
                                   toolchain::IRBuilderBase &builder,
                                   LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::Module *llvmModule = moduleTranslation.getLLVMModule();
  toolchain::LLVMContext &context = llvmModule->getContext();
  toolchain::NamedMDNode *linkerMDNode =
      llvmModule->getOrInsertNamedMetadata("toolchain.linker.options");
  SmallVector<toolchain::Metadata *> mdNodes;
  mdNodes.reserve(options.size());
  for (auto s : options.getAsRange<StringAttr>()) {
    auto *mdNode = toolchain::MDString::get(context, s.getValue());
    mdNodes.push_back(mdNode);
  }

  auto *listMDNode = toolchain::MDTuple::get(context, mdNodes);
  linkerMDNode->addOperand(listMDNode);
}

static toolchain::Metadata *
convertModuleFlagValue(StringRef key, ArrayAttr arrayAttr,
                       toolchain::IRBuilderBase &builder,
                       LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::LLVMContext &context = builder.getContext();
  toolchain::MDBuilder mdb(context);
  SmallVector<toolchain::Metadata *> nodes;

  if (key == LLVMDialect::getModuleFlagKeyCGProfileName()) {
    for (auto entry : arrayAttr.getAsRange<ModuleFlagCGProfileEntryAttr>()) {
      auto getFuncMetadata = [&](FlatSymbolRefAttr sym) -> toolchain::Metadata * {
        if (!sym)
          return nullptr;
        if (toolchain::Function *fn =
                moduleTranslation.lookupFunction(sym.getValue()))
          return toolchain::ValueAsMetadata::get(fn);
        return nullptr;
      };
      toolchain::Metadata *fromMetadata = getFuncMetadata(entry.getFrom());
      toolchain::Metadata *toMetadata = getFuncMetadata(entry.getTo());

      toolchain::Metadata *vals[] = {
          fromMetadata, toMetadata,
          mdb.createConstant(toolchain::ConstantInt::get(
              toolchain::Type::getInt64Ty(context), entry.getCount()))};
      nodes.push_back(toolchain::MDNode::get(context, vals));
    }
    return toolchain::MDTuple::getDistinct(context, nodes);
  }
  return nullptr;
}

static toolchain::Metadata *convertModuleFlagProfileSummaryAttr(
    StringRef key, ModuleFlagProfileSummaryAttr summaryAttr,
    toolchain::IRBuilderBase &builder, LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::LLVMContext &context = builder.getContext();
  toolchain::MDBuilder mdb(context);

  auto getIntTuple = [&](StringRef key, uint64_t val) -> toolchain::MDTuple * {
    SmallVector<toolchain::Metadata *> tupleNodes{
        mdb.createString(key), mdb.createConstant(toolchain::ConstantInt::get(
                                   toolchain::Type::getInt64Ty(context), val))};
    return toolchain::MDTuple::get(context, tupleNodes);
  };

  SmallVector<toolchain::Metadata *> fmtNode{
      mdb.createString("ProfileFormat"),
      mdb.createString(
          stringifyProfileSummaryFormatKind(summaryAttr.getFormat()))};

  SmallVector<toolchain::Metadata *> vals = {
      toolchain::MDTuple::get(context, fmtNode),
      getIntTuple("TotalCount", summaryAttr.getTotalCount()),
      getIntTuple("MaxCount", summaryAttr.getMaxCount()),
      getIntTuple("MaxInternalCount", summaryAttr.getMaxInternalCount()),
      getIntTuple("MaxFunctionCount", summaryAttr.getMaxFunctionCount()),
      getIntTuple("NumCounts", summaryAttr.getNumCounts()),
      getIntTuple("NumFunctions", summaryAttr.getNumFunctions()),
  };

  if (summaryAttr.getIsPartialProfile())
    vals.push_back(
        getIntTuple("IsPartialProfile", *summaryAttr.getIsPartialProfile()));

  if (summaryAttr.getPartialProfileRatio()) {
    SmallVector<toolchain::Metadata *> tupleNodes{
        mdb.createString("PartialProfileRatio"),
        mdb.createConstant(toolchain::ConstantFP::get(
            toolchain::Type::getDoubleTy(context),
            summaryAttr.getPartialProfileRatio().getValue()))};
    vals.push_back(toolchain::MDTuple::get(context, tupleNodes));
  }

  SmallVector<toolchain::Metadata *> detailedEntries;
  toolchain::Type *llvmInt64Type = toolchain::Type::getInt64Ty(context);
  for (ModuleFlagProfileSummaryDetailedAttr detailedEntry :
       summaryAttr.getDetailedSummary()) {
    SmallVector<toolchain::Metadata *> tupleNodes{
        mdb.createConstant(
            toolchain::ConstantInt::get(llvmInt64Type, detailedEntry.getCutOff())),
        mdb.createConstant(
            toolchain::ConstantInt::get(llvmInt64Type, detailedEntry.getMinCount())),
        mdb.createConstant(toolchain::ConstantInt::get(
            llvmInt64Type, detailedEntry.getNumCounts()))};
    detailedEntries.push_back(toolchain::MDTuple::get(context, tupleNodes));
  }
  SmallVector<toolchain::Metadata *> detailedSummary{
      mdb.createString("DetailedSummary"),
      toolchain::MDTuple::get(context, detailedEntries)};
  vals.push_back(toolchain::MDTuple::get(context, detailedSummary));

  return toolchain::MDNode::get(context, vals);
}

static void convertModuleFlagsOp(ArrayAttr flags, toolchain::IRBuilderBase &builder,
                                 LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::Module *llvmModule = moduleTranslation.getLLVMModule();
  for (auto flagAttr : flags.getAsRange<ModuleFlagAttr>()) {
    toolchain::Metadata *valueMetadata =
        toolchain::TypeSwitch<Attribute, toolchain::Metadata *>(flagAttr.getValue())
            .Case<StringAttr>([&](auto strAttr) {
              return toolchain::MDString::get(builder.getContext(),
                                         strAttr.getValue());
            })
            .Case<IntegerAttr>([&](auto intAttr) {
              return toolchain::ConstantAsMetadata::get(toolchain::ConstantInt::get(
                  toolchain::Type::getInt32Ty(builder.getContext()),
                  intAttr.getInt()));
            })
            .Case<ArrayAttr>([&](auto arrayAttr) {
              return convertModuleFlagValue(flagAttr.getKey().getValue(),
                                            arrayAttr, builder,
                                            moduleTranslation);
            })
            .Case([&](ModuleFlagProfileSummaryAttr summaryAttr) {
              return convertModuleFlagProfileSummaryAttr(
                  flagAttr.getKey().getValue(), summaryAttr, builder,
                  moduleTranslation);
            })
            .Default([](auto) { return nullptr; });

    assert(valueMetadata && "expected valid metadata");
    llvmModule->addModuleFlag(
        convertModFlagBehaviorToLLVM(flagAttr.getBehavior()),
        flagAttr.getKey().getValue(), valueMetadata);
  }
}

static toolchain::DILocalScope *
getLocalScopeFromLoc(toolchain::IRBuilderBase &builder, Location loc,
                     LLVM::ModuleTranslation &moduleTranslation) {
  if (auto scopeLoc =
          loc->findInstanceOf<FusedLocWith<LLVM::DILocalScopeAttr>>())
    if (auto *localScope = toolchain::dyn_cast<toolchain::DILocalScope>(
            moduleTranslation.translateDebugInfo(scopeLoc.getMetadata())))
      return localScope;
  return builder.GetInsertBlock()->getParent()->getSubprogram();
}

static LogicalResult
convertOperationImpl(Operation &opInst, toolchain::IRBuilderBase &builder,
                     LLVM::ModuleTranslation &moduleTranslation) {

  toolchain::IRBuilder<>::FastMathFlagGuard fmfGuard(builder);
  if (auto fmf = dyn_cast<FastmathFlagsInterface>(opInst))
    builder.setFastMathFlags(getFastmathFlags(fmf));

#include "mlir/Dialect/LLVMIR/LLVMConversions.inc"
#include "mlir/Dialect/LLVMIR/LLVMIntrinsicConversions.inc"

  // Emit function calls.  If the "callee" attribute is present, this is a
  // direct function call and we also need to look up the remapped function
  // itself.  Otherwise, this is an indirect call and the callee is the first
  // operand, look it up as a normal value.
  if (auto callOp = dyn_cast<LLVM::CallOp>(opInst)) {
    auto operands = moduleTranslation.lookupValues(callOp.getCalleeOperands());
    SmallVector<toolchain::OperandBundleDef> opBundles =
        convertOperandBundles(callOp.getOpBundleOperands(),
                              callOp.getOpBundleTags(), moduleTranslation);
    ArrayRef<toolchain::Value *> operandsRef(operands);
    toolchain::CallInst *call;
    if (auto attr = callOp.getCalleeAttr()) {
      if (toolchain::Function *function =
              moduleTranslation.lookupFunction(attr.getValue())) {
        call = builder.CreateCall(function, operandsRef, opBundles);
      } else {
        Operation *moduleOp = parentLLVMModule(&opInst);
        Operation *ifuncOp =
            moduleTranslation.symbolTable().lookupSymbolIn(moduleOp, attr);
        toolchain::GlobalValue *ifunc = moduleTranslation.lookupIFunc(ifuncOp);
        toolchain::FunctionType *calleeType = toolchain::cast<toolchain::FunctionType>(
            moduleTranslation.convertType(callOp.getCalleeFunctionType()));
        call = builder.CreateCall(calleeType, ifunc, operandsRef, opBundles);
      }
    } else {
      toolchain::FunctionType *calleeType = toolchain::cast<toolchain::FunctionType>(
          moduleTranslation.convertType(callOp.getCalleeFunctionType()));
      call = builder.CreateCall(calleeType, operandsRef.front(),
                                operandsRef.drop_front(), opBundles);
    }
    call->setCallingConv(convertCConvToLLVM(callOp.getCConv()));
    call->setTailCallKind(convertTailCallKindToLLVM(callOp.getTailCallKind()));
    if (callOp.getConvergentAttr())
      call->addFnAttr(toolchain::Attribute::Convergent);
    if (callOp.getNoUnwindAttr())
      call->addFnAttr(toolchain::Attribute::NoUnwind);
    if (callOp.getWillReturnAttr())
      call->addFnAttr(toolchain::Attribute::WillReturn);
    if (callOp.getNoInlineAttr())
      call->addFnAttr(toolchain::Attribute::NoInline);
    if (callOp.getAlwaysInlineAttr())
      call->addFnAttr(toolchain::Attribute::AlwaysInline);
    if (callOp.getInlineHintAttr())
      call->addFnAttr(toolchain::Attribute::InlineHint);

    if (failed(moduleTranslation.convertArgAndResultAttrs(callOp, call)))
      return failure();

    if (MemoryEffectsAttr memAttr = callOp.getMemoryEffectsAttr()) {
      toolchain::MemoryEffects memEffects =
          toolchain::MemoryEffects(toolchain::MemoryEffects::Location::ArgMem,
                              convertModRefInfoToLLVM(memAttr.getArgMem())) |
          toolchain::MemoryEffects(
              toolchain::MemoryEffects::Location::InaccessibleMem,
              convertModRefInfoToLLVM(memAttr.getInaccessibleMem())) |
          toolchain::MemoryEffects(toolchain::MemoryEffects::Location::Other,
                              convertModRefInfoToLLVM(memAttr.getOther())) |
          toolchain::MemoryEffects(toolchain::MemoryEffects::Location::ErrnoMem,
                              convertModRefInfoToLLVM(memAttr.getErrnoMem())) |
          toolchain::MemoryEffects(
              toolchain::MemoryEffects::Location::TargetMem0,
              convertModRefInfoToLLVM(memAttr.getTargetMem0())) |
          toolchain::MemoryEffects(toolchain::MemoryEffects::Location::TargetMem1,
                              convertModRefInfoToLLVM(memAttr.getTargetMem1()));
      call->setMemoryEffects(memEffects);
    }

    moduleTranslation.setAccessGroupsMetadata(callOp, call);
    moduleTranslation.setAliasScopeMetadata(callOp, call);
    moduleTranslation.setTBAAMetadata(callOp, call);
    // If the called function has a result, remap the corresponding value.  Note
    // that LLVM IR dialect CallOp has either 0 or 1 result.
    if (opInst.getNumResults() != 0)
      moduleTranslation.mapValue(opInst.getResult(0), call);
    // Check that LLVM call returns void for 0-result functions.
    else if (!call->getType()->isVoidTy())
      return failure();
    moduleTranslation.mapCall(callOp, call);
    return success();
  }

  if (auto inlineAsmOp = dyn_cast<LLVM::InlineAsmOp>(opInst)) {
    // TODO: refactor function type creation which usually occurs in std-LLVM
    // conversion.
    SmallVector<Type, 8> operandTypes;
    toolchain::append_range(operandTypes, inlineAsmOp.getOperands().getTypes());

    Type resultType;
    if (inlineAsmOp.getNumResults() == 0) {
      resultType = LLVM::LLVMVoidType::get(&moduleTranslation.getContext());
    } else {
      assert(inlineAsmOp.getNumResults() == 1);
      resultType = inlineAsmOp.getResultTypes()[0];
    }
    auto ft = LLVM::LLVMFunctionType::get(resultType, operandTypes);
    toolchain::InlineAsm *inlineAsmInst =
        inlineAsmOp.getAsmDialect()
            ? toolchain::InlineAsm::get(
                  static_cast<toolchain::FunctionType *>(
                      moduleTranslation.convertType(ft)),
                  inlineAsmOp.getAsmString(), inlineAsmOp.getConstraints(),
                  inlineAsmOp.getHasSideEffects(),
                  inlineAsmOp.getIsAlignStack(),
                  convertAsmDialectToLLVM(*inlineAsmOp.getAsmDialect()))
            : toolchain::InlineAsm::get(static_cast<toolchain::FunctionType *>(
                                       moduleTranslation.convertType(ft)),
                                   inlineAsmOp.getAsmString(),
                                   inlineAsmOp.getConstraints(),
                                   inlineAsmOp.getHasSideEffects(),
                                   inlineAsmOp.getIsAlignStack());
    toolchain::CallInst *inst = builder.CreateCall(
        inlineAsmInst,
        moduleTranslation.lookupValues(inlineAsmOp.getOperands()));
    inst->setTailCallKind(convertTailCallKindToLLVM(
        inlineAsmOp.getTailCallKindAttr().getTailCallKind()));
    if (auto maybeOperandAttrs = inlineAsmOp.getOperandAttrs()) {
      toolchain::AttributeList attrList;
      for (const auto &it : toolchain::enumerate(*maybeOperandAttrs)) {
        Attribute attr = it.value();
        if (!attr)
          continue;
        DictionaryAttr dAttr = cast<DictionaryAttr>(attr);
        if (dAttr.empty())
          continue;
        TypeAttr tAttr =
            cast<TypeAttr>(dAttr.get(InlineAsmOp::getElementTypeAttrName()));
        toolchain::AttrBuilder b(moduleTranslation.getLLVMContext());
        toolchain::Type *ty = moduleTranslation.convertType(tAttr.getValue());
        b.addTypeAttr(toolchain::Attribute::ElementType, ty);
        // shift to account for the returned value (this is always 1 aggregate
        // value in LLVM).
        int shift = (opInst.getNumResults() > 0) ? 1 : 0;
        attrList = attrList.addAttributesAtIndex(
            moduleTranslation.getLLVMContext(), it.index() + shift, b);
      }
      inst->setAttributes(attrList);
    }

    if (opInst.getNumResults() != 0)
      moduleTranslation.mapValue(opInst.getResult(0), inst);
    return success();
  }

  if (auto invOp = dyn_cast<LLVM::InvokeOp>(opInst)) {
    auto operands = moduleTranslation.lookupValues(invOp.getCalleeOperands());
    SmallVector<toolchain::OperandBundleDef> opBundles =
        convertOperandBundles(invOp.getOpBundleOperands(),
                              invOp.getOpBundleTags(), moduleTranslation);
    ArrayRef<toolchain::Value *> operandsRef(operands);
    toolchain::InvokeInst *result;
    if (auto attr = opInst.getAttrOfType<FlatSymbolRefAttr>("callee")) {
      result = builder.CreateInvoke(
          moduleTranslation.lookupFunction(attr.getValue()),
          moduleTranslation.lookupBlock(invOp.getSuccessor(0)),
          moduleTranslation.lookupBlock(invOp.getSuccessor(1)), operandsRef,
          opBundles);
    } else {
      toolchain::FunctionType *calleeType = toolchain::cast<toolchain::FunctionType>(
          moduleTranslation.convertType(invOp.getCalleeFunctionType()));
      result = builder.CreateInvoke(
          calleeType, operandsRef.front(),
          moduleTranslation.lookupBlock(invOp.getSuccessor(0)),
          moduleTranslation.lookupBlock(invOp.getSuccessor(1)),
          operandsRef.drop_front(), opBundles);
    }
    result->setCallingConv(convertCConvToLLVM(invOp.getCConv()));
    if (failed(moduleTranslation.convertArgAndResultAttrs(invOp, result)))
      return failure();
    moduleTranslation.mapBranch(invOp, result);
    // InvokeOp can only have 0 or 1 result
    if (invOp->getNumResults() != 0) {
      moduleTranslation.mapValue(opInst.getResult(0), result);
      return success();
    }
    return success(result->getType()->isVoidTy());
  }

  if (auto lpOp = dyn_cast<LLVM::LandingpadOp>(opInst)) {
    toolchain::Type *ty = moduleTranslation.convertType(lpOp.getType());
    toolchain::LandingPadInst *lpi =
        builder.CreateLandingPad(ty, lpOp.getNumOperands());
    lpi->setCleanup(lpOp.getCleanup());

    // Add clauses
    for (toolchain::Value *operand :
         moduleTranslation.lookupValues(lpOp.getOperands())) {
      // All operands should be constant - checked by verifier
      if (auto *constOperand = dyn_cast<toolchain::Constant>(operand))
        lpi->addClause(constOperand);
    }
    moduleTranslation.mapValue(lpOp.getResult(), lpi);
    return success();
  }

  // Emit branches.  We need to look up the remapped blocks and ignore the
  // block arguments that were transformed into PHI nodes.
  if (auto brOp = dyn_cast<LLVM::BrOp>(opInst)) {
    toolchain::BranchInst *branch =
        builder.CreateBr(moduleTranslation.lookupBlock(brOp.getSuccessor()));
    moduleTranslation.mapBranch(&opInst, branch);
    moduleTranslation.setLoopMetadata(&opInst, branch);
    return success();
  }
  if (auto condbrOp = dyn_cast<LLVM::CondBrOp>(opInst)) {
    toolchain::BranchInst *branch = builder.CreateCondBr(
        moduleTranslation.lookupValue(condbrOp.getOperand(0)),
        moduleTranslation.lookupBlock(condbrOp.getSuccessor(0)),
        moduleTranslation.lookupBlock(condbrOp.getSuccessor(1)));
    moduleTranslation.mapBranch(&opInst, branch);
    moduleTranslation.setLoopMetadata(&opInst, branch);
    return success();
  }
  if (auto switchOp = dyn_cast<LLVM::SwitchOp>(opInst)) {
    toolchain::SwitchInst *switchInst = builder.CreateSwitch(
        moduleTranslation.lookupValue(switchOp.getValue()),
        moduleTranslation.lookupBlock(switchOp.getDefaultDestination()),
        switchOp.getCaseDestinations().size());

    // Handle switch with zero cases.
    if (!switchOp.getCaseValues())
      return success();

    auto *ty = toolchain::cast<toolchain::IntegerType>(
        moduleTranslation.convertType(switchOp.getValue().getType()));
    for (auto i :
         toolchain::zip(toolchain::cast<DenseIntElementsAttr>(*switchOp.getCaseValues()),
                   switchOp.getCaseDestinations()))
      switchInst->addCase(
          toolchain::ConstantInt::get(ty, std::get<0>(i).getLimitedValue()),
          moduleTranslation.lookupBlock(std::get<1>(i)));

    moduleTranslation.mapBranch(&opInst, switchInst);
    return success();
  }
  if (auto indBrOp = dyn_cast<LLVM::IndirectBrOp>(opInst)) {
    toolchain::IndirectBrInst *indBr = builder.CreateIndirectBr(
        moduleTranslation.lookupValue(indBrOp.getAddr()),
        indBrOp->getNumSuccessors());
    for (auto *succ : indBrOp.getSuccessors())
      indBr->addDestination(moduleTranslation.lookupBlock(succ));
    moduleTranslation.mapBranch(&opInst, indBr);
    return success();
  }

  // Emit addressof.  We need to look up the global value referenced by the
  // operation and store it in the MLIR-to-LLVM value mapping.  This does not
  // emit any LLVM instruction.
  if (auto addressOfOp = dyn_cast<LLVM::AddressOfOp>(opInst)) {
    LLVM::GlobalOp global =
        addressOfOp.getGlobal(moduleTranslation.symbolTable());
    LLVM::LLVMFuncOp function =
        addressOfOp.getFunction(moduleTranslation.symbolTable());
    LLVM::AliasOp alias = addressOfOp.getAlias(moduleTranslation.symbolTable());
    LLVM::IFuncOp ifunc = addressOfOp.getIFunc(moduleTranslation.symbolTable());

    // The verifier should not have allowed this.
    assert((global || function || alias || ifunc) &&
           "referencing an undefined global, function, alias, or ifunc");

    toolchain::Value *llvmValue = nullptr;
    if (global)
      llvmValue = moduleTranslation.lookupGlobal(global);
    else if (alias)
      llvmValue = moduleTranslation.lookupAlias(alias);
    else if (function)
      llvmValue = moduleTranslation.lookupFunction(function.getName());
    else
      llvmValue = moduleTranslation.lookupIFunc(ifunc);

    moduleTranslation.mapValue(addressOfOp.getResult(), llvmValue);
    return success();
  }

  // Emit dso_local_equivalent. We need to look up the global value referenced
  // by the operation and store it in the MLIR-to-LLVM value mapping.
  if (auto dsoLocalEquivalentOp =
          dyn_cast<LLVM::DSOLocalEquivalentOp>(opInst)) {
    LLVM::LLVMFuncOp function =
        dsoLocalEquivalentOp.getFunction(moduleTranslation.symbolTable());
    LLVM::AliasOp alias =
        dsoLocalEquivalentOp.getAlias(moduleTranslation.symbolTable());

    // The verifier should not have allowed this.
    assert((function || alias) &&
           "referencing an undefined function, or alias");

    toolchain::Value *llvmValue = nullptr;
    if (alias)
      llvmValue = moduleTranslation.lookupAlias(alias);
    else
      llvmValue = moduleTranslation.lookupFunction(function.getName());

    moduleTranslation.mapValue(
        dsoLocalEquivalentOp.getResult(),
        toolchain::DSOLocalEquivalent::get(cast<toolchain::GlobalValue>(llvmValue)));
    return success();
  }

  // Emit blockaddress. We first need to find the LLVM block referenced by this
  // operation and then create a LLVM block address for it.
  if (auto blockAddressOp = dyn_cast<LLVM::BlockAddressOp>(opInst)) {
    BlockAddressAttr blockAddressAttr = blockAddressOp.getBlockAddr();
    toolchain::BasicBlock *llvmBlock =
        moduleTranslation.lookupBlockAddress(blockAddressAttr);

    toolchain::Value *llvmValue = nullptr;
    StringRef fnName = blockAddressAttr.getFunction().getValue();
    if (llvmBlock) {
      toolchain::Function *llvmFn = moduleTranslation.lookupFunction(fnName);
      llvmValue = toolchain::BlockAddress::get(llvmFn, llvmBlock);
    } else {
      // The matching LLVM block is not yet emitted, a placeholder is created
      // in its place. When the LLVM block is emitted later in translation,
      // the llvmValue is replaced with the actual toolchain::BlockAddress.
      // A GlobalVariable is chosen as placeholder because in general LLVM
      // constants are uniqued and are not proper for RAUW, since that could
      // harm unrelated uses of the constant.
      llvmValue = new toolchain::GlobalVariable(
          *moduleTranslation.getLLVMModule(),
          toolchain::PointerType::getUnqual(moduleTranslation.getLLVMContext()),
          /*isConstant=*/true, toolchain::GlobalValue::LinkageTypes::ExternalLinkage,
          /*Initializer=*/nullptr,
          Twine("__mlir_block_address_")
              .concat(Twine(fnName))
              .concat(Twine((uint64_t)blockAddressOp.getOperation())));
      moduleTranslation.mapUnresolvedBlockAddress(blockAddressOp, llvmValue);
    }

    moduleTranslation.mapValue(blockAddressOp.getResult(), llvmValue);
    return success();
  }

  // Emit block label. If this label is seen before BlockAddressOp is
  // translated, go ahead and already map it.
  if (auto blockTagOp = dyn_cast<LLVM::BlockTagOp>(opInst)) {
    auto funcOp = blockTagOp->getParentOfType<LLVMFuncOp>();
    BlockAddressAttr blockAddressAttr = BlockAddressAttr::get(
        &moduleTranslation.getContext(),
        FlatSymbolRefAttr::get(&moduleTranslation.getContext(),
                               funcOp.getName()),
        blockTagOp.getTag());
    moduleTranslation.mapBlockAddress(blockAddressAttr,
                                      builder.GetInsertBlock());
    return success();
  }

  return failure();
}

static LogicalResult
amendOperationImpl(Operation &op, ArrayRef<toolchain::Instruction *> instructions,
                   NamedAttribute attribute,
                   LLVM::ModuleTranslation &moduleTranslation) {
  StringRef name = attribute.getName();
  if (name == LLVMDialect::getMmraAttrName()) {
    SmallVector<toolchain::MMRAMetadata::TagT> tags;
    if (auto oneTag = dyn_cast<LLVM::MMRATagAttr>(attribute.getValue())) {
      tags.emplace_back(oneTag.getPrefix(), oneTag.getSuffix());
    } else if (auto manyTags = dyn_cast<ArrayAttr>(attribute.getValue())) {
      for (Attribute attr : manyTags) {
        auto tag = dyn_cast<MMRATagAttr>(attr);
        if (!tag)
          return op.emitOpError(
              "MMRA annotations array contains value that isn't an MMRA tag");
        tags.emplace_back(tag.getPrefix(), tag.getSuffix());
      }
    } else {
      return op.emitOpError(
          "toolchain.mmra is something other than an MMRA tag or an array of them");
    }
    toolchain::MDTuple *mmraMd =
        toolchain::MMRAMetadata::getMD(moduleTranslation.getLLVMContext(), tags);
    if (!mmraMd) {
      // Empty list, canonicalizes to nothing
      return success();
    }
    for (toolchain::Instruction *inst : instructions)
      inst->setMetadata(toolchain::LLVMContext::MD_mmra, mmraMd);
    return success();
  }
  return success();
}

namespace {
/// Implementation of the dialect interface that converts operations belonging
/// to the LLVM dialect to LLVM IR.
class LLVMDialectLLVMIRTranslationInterface
    : public LLVMTranslationDialectInterface {
public:
  using LLVMTranslationDialectInterface::LLVMTranslationDialectInterface;

  /// Translates the given operation to LLVM IR using the provided IR builder
  /// and saving the state in `moduleTranslation`.
  LogicalResult
  convertOperation(Operation *op, toolchain::IRBuilderBase &builder,
                   LLVM::ModuleTranslation &moduleTranslation) const final {
    return convertOperationImpl(*op, builder, moduleTranslation);
  }

  /// Handle some metadata that is represented as a discardable attribute.
  LogicalResult
  amendOperation(Operation *op, ArrayRef<toolchain::Instruction *> instructions,
                 NamedAttribute attribute,
                 LLVM::ModuleTranslation &moduleTranslation) const final {
    return amendOperationImpl(*op, instructions, attribute, moduleTranslation);
  }
};
} // namespace

void mlir::registerLLVMDialectTranslation(DialectRegistry &registry) {
  registry.insert<LLVM::LLVMDialect>();
  registry.addExtension(+[](MLIRContext *ctx, LLVM::LLVMDialect *dialect) {
    dialect->addInterfaces<LLVMDialectLLVMIRTranslationInterface>();
  });
}

void mlir::registerLLVMDialectTranslation(MLIRContext &context) {
  DialectRegistry registry;
  registerLLVMDialectTranslation(registry);
  context.appendDialectRegistry(registry);
}
