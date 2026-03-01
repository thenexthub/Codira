//===- OpenACCToLLVMIRTranslation.cpp -------------------------------------===//
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
// This file implements a translation between the MLIR OpenACC dialect and LLVM
// IR.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/Dialect/OpenACC/OpenACCToLLVMIRTranslation.h"
#include "mlir/Analysis/TopologicalSortUtils.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/OpenACC/OpenACC.h"
#include "mlir/IR/Operation.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Target/LLVMIR/Dialect/OpenMPCommon.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"

#include "vm/core/ADT/TypeSwitch.h"
#include "vm/core/Frontend/OpenMP/OMPConstants.h"

using namespace mlir;

using OpenACCIRBuilder = toolchain::OpenMPIRBuilder;

//===----------------------------------------------------------------------===//
// Utility functions
//===----------------------------------------------------------------------===//

/// Flag values are extracted from openmp/libomptarget/include/omptarget.h and
/// mapped to corresponding OpenACC flags.
static constexpr uint64_t kCreateFlag = 0x000;
static constexpr uint64_t kDeviceCopyinFlag = 0x001;
static constexpr uint64_t kHostCopyoutFlag = 0x002;
static constexpr uint64_t kPresentFlag = 0x1000;
static constexpr uint64_t kDeleteFlag = 0x008;
// Runtime extension to implement the OpenACC second reference counter.
static constexpr uint64_t kHoldFlag = 0x2000;

/// Default value for the device id
static constexpr int64_t kDefaultDevice = -1;

/// Create the location struct from the operation location information.
static toolchain::Value *createSourceLocationInfo(OpenACCIRBuilder &builder,
                                             Operation *op) {
  auto loc = op->getLoc();
  auto funcOp = op->getParentOfType<LLVM::LLVMFuncOp>();
  StringRef funcName = funcOp ? funcOp.getName() : "unknown";
  uint32_t strLen;
  toolchain::Constant *locStr = mlir::LLVM::createSourceLocStrFromLocation(
      loc, builder, funcName, strLen);
  return builder.getOrCreateIdent(locStr, strLen);
}

/// Return the runtime function used to lower the given operation.
static toolchain::Function *getAssociatedFunction(OpenACCIRBuilder &builder,
                                             Operation *op) {
  return toolchain::TypeSwitch<Operation *, toolchain::Function *>(op)
      .Case([&](acc::EnterDataOp) {
        return builder.getOrCreateRuntimeFunctionPtr(
            toolchain::omp::OMPRTL___tgt_target_data_begin_mapper);
      })
      .Case([&](acc::ExitDataOp) {
        return builder.getOrCreateRuntimeFunctionPtr(
            toolchain::omp::OMPRTL___tgt_target_data_end_mapper);
      })
      .Case([&](acc::UpdateOp) {
        return builder.getOrCreateRuntimeFunctionPtr(
            toolchain::omp::OMPRTL___tgt_target_data_update_mapper);
      });
  llvm_unreachable("Unknown OpenACC operation");
}

/// Extract pointer, size and mapping information from operands
/// to populate the future functions arguments.
static LogicalResult
processOperands(toolchain::IRBuilderBase &builder,
                LLVM::ModuleTranslation &moduleTranslation, Operation *op,
                ValueRange operands, unsigned totalNbOperand,
                uint64_t operandFlag, SmallVector<uint64_t> &flags,
                SmallVectorImpl<toolchain::Constant *> &names, unsigned &index,
                struct OpenACCIRBuilder::MapperAllocas &mapperAllocas) {
  OpenACCIRBuilder *accBuilder = moduleTranslation.getOpenMPBuilder();
  toolchain::LLVMContext &ctx = builder.getContext();
  auto *i8PtrTy = toolchain::PointerType::getUnqual(ctx);
  auto *arrI8PtrTy = toolchain::ArrayType::get(i8PtrTy, totalNbOperand);
  auto *i64Ty = toolchain::Type::getInt64Ty(ctx);
  auto *arrI64Ty = toolchain::ArrayType::get(i64Ty, totalNbOperand);

  for (Value data : operands) {
    toolchain::Value *dataValue = moduleTranslation.lookupValue(data);

    toolchain::Value *dataPtrBase;
    toolchain::Value *dataPtr;
    toolchain::Value *dataSize;

    if (isa<LLVM::LLVMPointerType>(data.getType())) {
      dataPtrBase = dataValue;
      dataPtr = dataValue;
      dataSize = accBuilder->getSizeInBytes(dataValue);
    } else {
      return op->emitOpError()
             << "Data operand must be legalized before translation."
             << "Unsupported type: " << data.getType();
    }

    // Store base pointer extracted from operand into the i-th position of
    // argBase.
    toolchain::Value *ptrBaseGEP = builder.CreateInBoundsGEP(
        arrI8PtrTy, mapperAllocas.ArgsBase,
        {builder.getInt32(0), builder.getInt32(index)});
    builder.CreateStore(dataPtrBase, ptrBaseGEP);

    // Store pointer extracted from operand into the i-th position of args.
    toolchain::Value *ptrGEP = builder.CreateInBoundsGEP(
        arrI8PtrTy, mapperAllocas.Args,
        {builder.getInt32(0), builder.getInt32(index)});
    builder.CreateStore(dataPtr, ptrGEP);

    // Store size extracted from operand into the i-th position of argSizes.
    toolchain::Value *sizeGEP = builder.CreateInBoundsGEP(
        arrI64Ty, mapperAllocas.ArgSizes,
        {builder.getInt32(0), builder.getInt32(index)});
    builder.CreateStore(dataSize, sizeGEP);

    flags.push_back(operandFlag);
    toolchain::Constant *mapName =
        mlir::LLVM::createMappingInformation(data.getLoc(), *accBuilder);
    names.push_back(mapName);
    ++index;
  }
  return success();
}

/// Process data operands from acc::EnterDataOp
static LogicalResult
processDataOperands(toolchain::IRBuilderBase &builder,
                    LLVM::ModuleTranslation &moduleTranslation,
                    acc::EnterDataOp op, SmallVector<uint64_t> &flags,
                    SmallVectorImpl<toolchain::Constant *> &names,
                    struct OpenACCIRBuilder::MapperAllocas &mapperAllocas) {
  // TODO add `create_zero` and `attach` operands

  unsigned index = 0;

  // Create operands are handled as `alloc` call.
  // Copyin operands are handled as `to` call.
  toolchain::SmallVector<mlir::Value> create, copyin;
  for (mlir::Value dataOp : op.getDataClauseOperands()) {
    if (auto createOp = dataOp.getDefiningOp<acc::CreateOp>()) {
      create.push_back(createOp.getVarPtr());
    } else if (auto copyinOp = mlir::dyn_cast_or_null<acc::CopyinOp>(
                   dataOp.getDefiningOp())) {
      copyin.push_back(copyinOp.getVarPtr());
    }
  }

  auto nbTotalOperands = create.size() + copyin.size();

  // Create operands are handled as `alloc` call.
  if (failed(processOperands(builder, moduleTranslation, op, create,
                             nbTotalOperands, kCreateFlag, flags, names, index,
                             mapperAllocas)))
    return failure();

  // Copyin operands are handled as `to` call.
  if (failed(processOperands(builder, moduleTranslation, op, copyin,
                             nbTotalOperands, kDeviceCopyinFlag, flags, names,
                             index, mapperAllocas)))
    return failure();

  return success();
}

/// Process data operands from acc::ExitDataOp
static LogicalResult
processDataOperands(toolchain::IRBuilderBase &builder,
                    LLVM::ModuleTranslation &moduleTranslation,
                    acc::ExitDataOp op, SmallVector<uint64_t> &flags,
                    SmallVectorImpl<toolchain::Constant *> &names,
                    struct OpenACCIRBuilder::MapperAllocas &mapperAllocas) {
  // TODO add `detach` operands

  unsigned index = 0;

  toolchain::SmallVector<mlir::Value> deleteOperands, copyoutOperands;
  for (mlir::Value dataOp : op.getDataClauseOperands()) {
    if (auto devicePtrOp = mlir::dyn_cast_or_null<acc::GetDevicePtrOp>(
            dataOp.getDefiningOp())) {
      for (auto &u : devicePtrOp.getAccPtr().getUses()) {
        if (mlir::dyn_cast_or_null<acc::DeleteOp>(u.getOwner()))
          deleteOperands.push_back(devicePtrOp.getVarPtr());
        else if (mlir::dyn_cast_or_null<acc::CopyoutOp>(u.getOwner()))
          copyoutOperands.push_back(devicePtrOp.getVarPtr());
      }
    }
  }

  auto nbTotalOperands = deleteOperands.size() + copyoutOperands.size();

  // Delete operands are handled as `delete` call.
  if (failed(processOperands(builder, moduleTranslation, op, deleteOperands,
                             nbTotalOperands, kDeleteFlag, flags, names, index,
                             mapperAllocas)))
    return failure();

  // Copyout operands are handled as `from` call.
  if (failed(processOperands(builder, moduleTranslation, op, copyoutOperands,
                             nbTotalOperands, kHostCopyoutFlag, flags, names,
                             index, mapperAllocas)))
    return failure();

  return success();
}

/// Process data operands from acc::UpdateOp
static LogicalResult
processDataOperands(toolchain::IRBuilderBase &builder,
                    LLVM::ModuleTranslation &moduleTranslation,
                    acc::UpdateOp op, SmallVector<uint64_t> &flags,
                    SmallVectorImpl<toolchain::Constant *> &names,
                    struct OpenACCIRBuilder::MapperAllocas &mapperAllocas) {
  unsigned index = 0;

  // Host operands are handled as `from` call.
  // Device operands are handled as `to` call.
  toolchain::SmallVector<mlir::Value> from, to;
  for (mlir::Value dataOp : op.getDataClauseOperands()) {
    if (auto getDevicePtrOp = mlir::dyn_cast_or_null<acc::GetDevicePtrOp>(
            dataOp.getDefiningOp())) {
      from.push_back(getDevicePtrOp.getVarPtr());
    } else if (auto updateDeviceOp =
                   mlir::dyn_cast_or_null<acc::UpdateDeviceOp>(
                       dataOp.getDefiningOp())) {
      to.push_back(updateDeviceOp.getVarPtr());
    }
  }

  if (failed(processOperands(builder, moduleTranslation, op, from, from.size(),
                             kHostCopyoutFlag, flags, names, index,
                             mapperAllocas)))
    return failure();

  if (failed(processOperands(builder, moduleTranslation, op, to, to.size(),
                             kDeviceCopyinFlag, flags, names, index,
                             mapperAllocas)))
    return failure();
  return success();
}

//===----------------------------------------------------------------------===//
// Conversion functions
//===----------------------------------------------------------------------===//

/// Converts an OpenACC data operation into LLVM IR.
static LogicalResult convertDataOp(acc::DataOp &op,
                                   toolchain::IRBuilderBase &builder,
                                   LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::LLVMContext &ctx = builder.getContext();
  auto enclosingFuncOp = op.getOperation()->getParentOfType<LLVM::LLVMFuncOp>();
  toolchain::Function *enclosingFunction =
      moduleTranslation.lookupFunction(enclosingFuncOp.getName());

  OpenACCIRBuilder *accBuilder = moduleTranslation.getOpenMPBuilder();

  toolchain::Value *srcLocInfo = createSourceLocationInfo(*accBuilder, op);

  toolchain::Function *beginMapperFunc = accBuilder->getOrCreateRuntimeFunctionPtr(
      toolchain::omp::OMPRTL___tgt_target_data_begin_mapper);

  toolchain::Function *endMapperFunc = accBuilder->getOrCreateRuntimeFunctionPtr(
      toolchain::omp::OMPRTL___tgt_target_data_end_mapper);

  // Number of arguments in the data operation.
  unsigned totalNbOperand = op.getNumDataOperands();

  struct OpenACCIRBuilder::MapperAllocas mapperAllocas;
  OpenACCIRBuilder::InsertPointTy allocaIP(
      &enclosingFunction->getEntryBlock(),
      enclosingFunction->getEntryBlock().getFirstInsertionPt());
  accBuilder->createMapperAllocas(builder.saveIP(), allocaIP, totalNbOperand,
                                  mapperAllocas);

  SmallVector<uint64_t> flags;
  SmallVector<toolchain::Constant *> names;
  unsigned index = 0;

  // TODO handle no_create, deviceptr and attach operands.

  toolchain::SmallVector<mlir::Value> copyin, copyout, create, present,
      deleteOperands;
  for (mlir::Value dataOp : op.getDataClauseOperands()) {
    if (auto devicePtrOp = mlir::dyn_cast_or_null<acc::GetDevicePtrOp>(
            dataOp.getDefiningOp())) {
      for (auto &u : devicePtrOp.getAccPtr().getUses()) {
        if (mlir::dyn_cast_or_null<acc::DeleteOp>(u.getOwner())) {
          deleteOperands.push_back(devicePtrOp.getVarPtr());
        } else if (mlir::dyn_cast_or_null<acc::CopyoutOp>(u.getOwner())) {
          // TODO copyout zero currenlty handled as copyout. Update when
          // extension available.
          copyout.push_back(devicePtrOp.getVarPtr());
        }
      }
    } else if (auto copyinOp = mlir::dyn_cast_or_null<acc::CopyinOp>(
                   dataOp.getDefiningOp())) {
      // TODO copyin readonly currenlty handled as copyin. Update when extension
      // available.
      copyin.push_back(copyinOp.getVarPtr());
    } else if (auto createOp = mlir::dyn_cast_or_null<acc::CreateOp>(
                   dataOp.getDefiningOp())) {
      // TODO create zero currenlty handled as create. Update when extension
      // available.
      create.push_back(createOp.getVarPtr());
    } else if (auto presentOp = mlir::dyn_cast_or_null<acc::PresentOp>(
                   dataOp.getDefiningOp())) {
      present.push_back(createOp.getVarPtr());
    }
  }

  auto nbTotalOperands = copyin.size() + copyout.size() + create.size() +
                         present.size() + deleteOperands.size();

  // Copyin operands are handled as `to` call.
  if (failed(processOperands(builder, moduleTranslation, op, copyin,
                             nbTotalOperands, kDeviceCopyinFlag | kHoldFlag,
                             flags, names, index, mapperAllocas)))
    return failure();

  // Delete operands are handled as `delete` call.
  if (failed(processOperands(builder, moduleTranslation, op, deleteOperands,
                             nbTotalOperands, kDeleteFlag, flags, names, index,
                             mapperAllocas)))
    return failure();

  // Copyout operands are handled as `from` call.
  if (failed(processOperands(builder, moduleTranslation, op, copyout,
                             nbTotalOperands, kHostCopyoutFlag | kHoldFlag,
                             flags, names, index, mapperAllocas)))
    return failure();

  // Create operands are handled as `alloc` call.
  if (failed(processOperands(builder, moduleTranslation, op, create,
                             nbTotalOperands, kCreateFlag | kHoldFlag, flags,
                             names, index, mapperAllocas)))
    return failure();

  if (failed(processOperands(builder, moduleTranslation, op, present,
                             nbTotalOperands, kPresentFlag | kHoldFlag, flags,
                             names, index, mapperAllocas)))
    return failure();

  toolchain::GlobalVariable *maptypes =
      accBuilder->createOffloadMaptypes(flags, ".offload_maptypes");
  toolchain::Value *maptypesArg = builder.CreateConstInBoundsGEP2_32(
      toolchain::ArrayType::get(toolchain::Type::getInt64Ty(ctx), totalNbOperand),
      maptypes, /*Idx0=*/0, /*Idx1=*/0);

  toolchain::GlobalVariable *mapnames =
      accBuilder->createOffloadMapnames(names, ".offload_mapnames");
  toolchain::Value *mapnamesArg = builder.CreateConstInBoundsGEP2_32(
      toolchain::ArrayType::get(toolchain::PointerType::getUnqual(ctx), totalNbOperand),
      mapnames, /*Idx0=*/0, /*Idx1=*/0);

  // Create call to start the data region.
  accBuilder->emitMapperCall(builder.saveIP(), beginMapperFunc, srcLocInfo,
                             maptypesArg, mapnamesArg, mapperAllocas,
                             kDefaultDevice, totalNbOperand);

  // Convert the region.
  toolchain::BasicBlock *entryBlock = nullptr;

  for (Block &bb : op.getRegion()) {
    toolchain::BasicBlock *llvmBB = toolchain::BasicBlock::Create(
        ctx, "acc.data", builder.GetInsertBlock()->getParent());
    if (entryBlock == nullptr)
      entryBlock = llvmBB;
    moduleTranslation.mapBlock(&bb, llvmBB);
  }

  auto afterDataRegion = builder.saveIP();

  toolchain::BranchInst *sourceTerminator = builder.CreateBr(entryBlock);

  builder.restoreIP(afterDataRegion);
  toolchain::BasicBlock *endDataBlock = toolchain::BasicBlock::Create(
      ctx, "acc.end_data", builder.GetInsertBlock()->getParent());

  SetVector<Block *> blocks = getBlocksSortedByDominance(op.getRegion());
  for (Block *bb : blocks) {
    toolchain::BasicBlock *llvmBB = moduleTranslation.lookupBlock(bb);
    if (bb->isEntryBlock()) {
      assert(sourceTerminator->getNumSuccessors() == 1 &&
             "provided entry block has multiple successors");
      sourceTerminator->setSuccessor(0, llvmBB);
    }

    if (failed(
            moduleTranslation.convertBlock(*bb, bb->isEntryBlock(), builder))) {
      return failure();
    }

    if (isa<acc::TerminatorOp, acc::YieldOp>(bb->getTerminator()))
      builder.CreateBr(endDataBlock);
  }

  // Create call to end the data region.
  builder.SetInsertPoint(endDataBlock);
  accBuilder->emitMapperCall(builder.saveIP(), endMapperFunc, srcLocInfo,
                             maptypesArg, mapnamesArg, mapperAllocas,
                             kDefaultDevice, totalNbOperand);

  return success();
}

/// Converts an OpenACC standalone data operation into LLVM IR.
template <typename OpTy>
static LogicalResult
convertStandaloneDataOp(OpTy &op, toolchain::IRBuilderBase &builder,
                        LLVM::ModuleTranslation &moduleTranslation) {
  auto enclosingFuncOp =
      op.getOperation()->template getParentOfType<LLVM::LLVMFuncOp>();
  toolchain::Function *enclosingFunction =
      moduleTranslation.lookupFunction(enclosingFuncOp.getName());

  OpenACCIRBuilder *accBuilder = moduleTranslation.getOpenMPBuilder();

  auto *srcLocInfo = createSourceLocationInfo(*accBuilder, op);
  auto *mapperFunc = getAssociatedFunction(*accBuilder, op);

  // Number of arguments in the enter_data operation.
  unsigned totalNbOperand = op.getNumDataOperands();

  toolchain::LLVMContext &ctx = builder.getContext();

  struct OpenACCIRBuilder::MapperAllocas mapperAllocas;
  OpenACCIRBuilder::InsertPointTy allocaIP(
      &enclosingFunction->getEntryBlock(),
      enclosingFunction->getEntryBlock().getFirstInsertionPt());
  accBuilder->createMapperAllocas(builder.saveIP(), allocaIP, totalNbOperand,
                                  mapperAllocas);

  SmallVector<uint64_t> flags;
  SmallVector<toolchain::Constant *> names;

  if (failed(processDataOperands(builder, moduleTranslation, op, flags, names,
                                 mapperAllocas)))
    return failure();

  toolchain::GlobalVariable *maptypes =
      accBuilder->createOffloadMaptypes(flags, ".offload_maptypes");
  toolchain::Value *maptypesArg = builder.CreateConstInBoundsGEP2_32(
      toolchain::ArrayType::get(toolchain::Type::getInt64Ty(ctx), totalNbOperand),
      maptypes, /*Idx0=*/0, /*Idx1=*/0);

  toolchain::GlobalVariable *mapnames =
      accBuilder->createOffloadMapnames(names, ".offload_mapnames");
  toolchain::Value *mapnamesArg = builder.CreateConstInBoundsGEP2_32(
      toolchain::ArrayType::get(toolchain::PointerType::getUnqual(ctx), totalNbOperand),
      mapnames, /*Idx0=*/0, /*Idx1=*/0);

  accBuilder->emitMapperCall(builder.saveIP(), mapperFunc, srcLocInfo,
                             maptypesArg, mapnamesArg, mapperAllocas,
                             kDefaultDevice, totalNbOperand);

  return success();
}

namespace {

/// Implementation of the dialect interface that converts operations belonging
/// to the OpenACC dialect to LLVM IR.
class OpenACCDialectLLVMIRTranslationInterface
    : public LLVMTranslationDialectInterface {
public:
  using LLVMTranslationDialectInterface::LLVMTranslationDialectInterface;

  /// Translates the given operation to LLVM IR using the provided IR builder
  /// and saving the state in `moduleTranslation`.
  LogicalResult
  convertOperation(Operation *op, toolchain::IRBuilderBase &builder,
                   LLVM::ModuleTranslation &moduleTranslation) const final;
};

} // namespace

/// Given an OpenACC MLIR operation, create the corresponding LLVM IR
/// (including OpenACC runtime calls).
LogicalResult OpenACCDialectLLVMIRTranslationInterface::convertOperation(
    Operation *op, toolchain::IRBuilderBase &builder,
    LLVM::ModuleTranslation &moduleTranslation) const {

  return toolchain::TypeSwitch<Operation *, LogicalResult>(op)
      .Case([&](acc::DataOp dataOp) {
        return convertDataOp(dataOp, builder, moduleTranslation);
      })
      .Case([&](acc::EnterDataOp enterDataOp) {
        return convertStandaloneDataOp<acc::EnterDataOp>(enterDataOp, builder,
                                                         moduleTranslation);
      })
      .Case([&](acc::ExitDataOp exitDataOp) {
        return convertStandaloneDataOp<acc::ExitDataOp>(exitDataOp, builder,
                                                        moduleTranslation);
      })
      .Case([&](acc::UpdateOp updateOp) {
        return convertStandaloneDataOp<acc::UpdateOp>(updateOp, builder,
                                                      moduleTranslation);
      })
      .Case<acc::TerminatorOp, acc::YieldOp>([](auto op) {
        // `yield` and `terminator` can be just omitted. The block structure was
        // created in the function that handles their parent operation.
        assert(op->getNumOperands() == 0 &&
               "unexpected OpenACC terminator with operands");
        return success();
      })
      .Case<acc::CreateOp, acc::CopyinOp, acc::CopyoutOp, acc::DeleteOp,
            acc::UpdateDeviceOp, acc::GetDevicePtrOp>([](auto op) {
        // NOP
        return success();
      })
      .Default([&](Operation *op) {
        return op->emitError("unsupported OpenACC operation: ")
               << op->getName();
      });
}

void mlir::registerOpenACCDialectTranslation(DialectRegistry &registry) {
  registry.insert<acc::OpenACCDialect>();
  registry.addExtension(+[](MLIRContext *ctx, acc::OpenACCDialect *dialect) {
    dialect->addInterfaces<OpenACCDialectLLVMIRTranslationInterface>();
  });
}

void mlir::registerOpenACCDialectTranslation(MLIRContext &context) {
  DialectRegistry registry;
  registerOpenACCDialectTranslation(registry);
  context.appendDialectRegistry(registry);
}
