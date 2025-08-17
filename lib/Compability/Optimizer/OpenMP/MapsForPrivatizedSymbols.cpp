//===- MapsForPrivatizedSymbols.cpp ---------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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

//===----------------------------------------------------------------------===//
/// \file
/// An OpenMP dialect related pass for FIR/HLFIR which creates MapInfoOp
/// instances for certain privatized symbols.
/// For example, if an allocatable variable is used in a private clause attached
/// to a omp.target op, then the allocatable variable's descriptor will be
/// needed on the device (e.g. GPU). This descriptor needs to be separately
/// mapped onto the device. This pass creates the necessary omp.map.info ops for
/// this.
//===----------------------------------------------------------------------===//
// TODO:
// 1. Before adding omp.map.info, check if we already have an omp.map.info for
// the variable in question.
// 2. Generalize this for more than just omp.target ops.
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Builder/DirectivesCommon.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/HLFIRTools.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/Dialect/Support/KindMapping.h"
#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"
#include "language/Compability/Optimizer/OpenMP/Passes.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Pass/Pass.h"
#include "toolchain/Frontend/OpenMP/OMPConstants.h"
#include "toolchain/Support/Debug.h"
#include <type_traits>

#define DEBUG_TYPE "omp-maps-for-privatized-symbols"
#define PDBGS() (toolchain::dbgs() << "[" << DEBUG_TYPE << "]: ")
namespace flangomp {
#define GEN_PASS_DEF_MAPSFORPRIVATIZEDSYMBOLSPASS
#include "language/Compability/Optimizer/OpenMP/Passes.h.inc"
} // namespace flangomp

using namespace mlir;

namespace {
class MapsForPrivatizedSymbolsPass
    : public flangomp::impl::MapsForPrivatizedSymbolsPassBase<
          MapsForPrivatizedSymbolsPass> {

  omp::MapInfoOp createMapInfo(Location loc, Value var,
                               fir::FirOpBuilder &builder) {
    // Check if a value of type `type` can be passed to the kernel by value.
    // All kernel parameters are of pointer type, so if the value can be
    // represented inside of a pointer, then it can be passed by value.
    auto canPassByValue = [&](mlir::Type type) {
      const mlir::DataLayout &dl = builder.getDataLayout();
      mlir::Type ptrTy = mlir::LLVM::LLVMPointerType::get(builder.getContext());
      uint64_t ptrSize = dl.getTypeSize(ptrTy);
      uint64_t ptrAlign = dl.getTypePreferredAlignment(ptrTy);

      auto [size, align] = fir::getTypeSizeAndAlignmentOrCrash(
          loc, type, dl, builder.getKindMap());
      return size <= ptrSize && align <= ptrAlign;
    };

    uint64_t mapTypeTo = static_cast<
        std::underlying_type_t<toolchain::omp::OpenMPOffloadMappingFlags>>(
        toolchain::omp::OpenMPOffloadMappingFlags::OMP_MAP_TO);
    Operation *definingOp = var.getDefiningOp();

    Value varPtr = var;
    // We want the first result of the hlfir.declare op because our goal
    // is to map the descriptor (fir.box or fir.boxchar) and the first
    // result for hlfir.declare is the descriptor if a the symbol being
    // declared needs a descriptor.
    // Some types are boxed immediately before privatization. These have other
    // operations in between the privatization and the declaration. It is safe
    // to use var directly here because they will be boxed anyway.
    if (auto declOp = toolchain::dyn_cast_if_present<hlfir::DeclareOp>(definingOp))
      varPtr = declOp.getBase();

    // If we do not have a reference to a descriptor but the descriptor itself,
    // then we need to store that on the stack so that we can map the
    // address of the descriptor.
    if (mlir::isa<fir::BaseBoxType>(varPtr.getType()) ||
        mlir::isa<fir::BoxCharType>(varPtr.getType())) {
      OpBuilder::InsertPoint savedInsPoint = builder.saveInsertionPoint();
      mlir::Block *allocaBlock = builder.getAllocaBlock();
      assert(allocaBlock && "No allocablock  found for a funcOp");
      builder.setInsertionPointToStart(allocaBlock);
      auto alloca = fir::AllocaOp::create(builder, loc, varPtr.getType());
      builder.restoreInsertionPoint(savedInsPoint);
      fir::StoreOp::create(builder, loc, varPtr, alloca);
      varPtr = alloca;
    }
    assert(mlir::isa<omp::PointerLikeType>(varPtr.getType()) &&
           "Dealing with a varPtr that is not a PointerLikeType");

    // Figure out the bounds because knowing the bounds will help the subsequent
    // MapInfoFinalizationPass map the underlying data of the descriptor.
    toolchain::SmallVector<mlir::Value> boundsOps;
    if (needsBoundsOps(varPtr))
      genBoundsOps(builder, varPtr, boundsOps);

    mlir::omp::VariableCaptureKind captureKind =
        mlir::omp::VariableCaptureKind::ByRef;
    if (fir::isa_trivial(fir::unwrapRefType(varPtr.getType())) ||
        fir::isa_char(fir::unwrapRefType(varPtr.getType()))) {
      if (canPassByValue(fir::unwrapRefType(varPtr.getType()))) {
        captureKind = mlir::omp::VariableCaptureKind::ByCopy;
      }
    }

    return omp::MapInfoOp::create(
        builder, loc, varPtr.getType(), varPtr,
        TypeAttr::get(toolchain::cast<omp::PointerLikeType>(varPtr.getType())
                          .getElementType()),
        builder.getIntegerAttr(builder.getIntegerType(64, /*isSigned=*/false),
                               mapTypeTo),
        builder.getAttr<omp::VariableCaptureKindAttr>(captureKind),
        /*varPtrPtr=*/Value{},
        /*members=*/SmallVector<Value>{},
        /*member_index=*/mlir::ArrayAttr{},
        /*bounds=*/boundsOps,
        /*mapperId=*/mlir::FlatSymbolRefAttr(), /*name=*/StringAttr(),
        builder.getBoolAttr(false));
  }
  void addMapInfoOp(omp::TargetOp targetOp, omp::MapInfoOp mapInfoOp) {
    auto argIface = toolchain::cast<omp::BlockArgOpenMPOpInterface>(*targetOp);
    unsigned insertIndex =
        argIface.getMapBlockArgsStart() + argIface.numMapBlockArgs();
    targetOp.getMapVarsMutable().append(ValueRange{mapInfoOp});
    targetOp.getRegion().insertArgument(insertIndex, mapInfoOp.getType(),
                                        mapInfoOp.getLoc());
  }
  void addMapInfoOps(omp::TargetOp targetOp,
                     toolchain::SmallVectorImpl<omp::MapInfoOp> &mapInfoOps) {
    for (auto mapInfoOp : mapInfoOps)
      addMapInfoOp(targetOp, mapInfoOp);
  }
  void runOnOperation() override {
    ModuleOp module = getOperation()->getParentOfType<ModuleOp>();
    fir::KindMapping kindMap = fir::getKindMapping(module);
    fir::FirOpBuilder builder{module, std::move(kindMap)};
    toolchain::DenseMap<Operation *, toolchain::SmallVector<omp::MapInfoOp, 4>>
        mapInfoOpsForTarget;

    getOperation()->walk([&](omp::TargetOp targetOp) {
      if (targetOp.getPrivateVars().empty())
        return;
      OperandRange privVars = targetOp.getPrivateVars();
      toolchain::SmallVector<int64_t> privVarMapIdx;

      std::optional<ArrayAttr> privSyms = targetOp.getPrivateSyms();
      SmallVector<omp::MapInfoOp, 4> mapInfoOps;
      for (auto [privVar, privSym] : toolchain::zip_equal(privVars, *privSyms)) {

        SymbolRefAttr privatizerName = toolchain::cast<SymbolRefAttr>(privSym);
        omp::PrivateClauseOp privatizer =
            SymbolTable::lookupNearestSymbolFrom<omp::PrivateClauseOp>(
                targetOp, privatizerName);
        if (!privatizer.needsMap()) {
          privVarMapIdx.push_back(-1);
          continue;
        }

        privVarMapIdx.push_back(targetOp.getMapVars().size() +
                                mapInfoOps.size());

        builder.setInsertionPoint(targetOp);
        Location loc = targetOp.getLoc();
        omp::MapInfoOp mapInfoOp = createMapInfo(loc, privVar, builder);
        mapInfoOps.push_back(mapInfoOp);

        LLVM_DEBUG(PDBGS() << "MapsForPrivatizedSymbolsPass created ->\n"
                           << mapInfoOp << "\n");
      }
      if (!mapInfoOps.empty()) {
        mapInfoOpsForTarget.insert({targetOp.getOperation(), mapInfoOps});
        targetOp.setPrivateMapsAttr(
            mlir::DenseI64ArrayAttr::get(targetOp.getContext(), privVarMapIdx));
      }
    });
    if (!mapInfoOpsForTarget.empty()) {
      for (auto &[targetOp, mapInfoOps] : mapInfoOpsForTarget) {
        addMapInfoOps(static_cast<omp::TargetOp>(targetOp), mapInfoOps);
      }
    }
  }
  // As the name suggests, this function examines var to determine if
  // it has dynamic size. If true, this pass'll have to extract these
  // bounds from descriptor of var and add the bounds to the resultant
  // MapInfoOp.
  bool needsBoundsOps(mlir::Value var) {
    assert(mlir::isa<omp::PointerLikeType>(var.getType()) &&
           "needsBoundsOps can deal only with pointer types");
    mlir::Type t = fir::unwrapRefType(var.getType());
    // t could be a box, so look inside the box
    auto innerType = fir::dyn_cast_ptrOrBoxEleTy(t);
    if (innerType)
      return fir::hasDynamicSize(innerType);
    return fir::hasDynamicSize(t);
  }

  void genBoundsOps(fir::FirOpBuilder &builder, mlir::Value var,
                    toolchain::SmallVector<mlir::Value> &boundsOps) {
    mlir::Location loc = var.getLoc();
    fir::factory::AddrAndBoundsInfo info =
        fir::factory::getDataOperandBaseAddr(builder, var,
                                             /*isOptional=*/false, loc);
    fir::ExtendedValue extendedValue =
        hlfir::translateToExtendedValue(loc, builder, hlfir::Entity{info.addr},
                                        /*continguousHint=*/true)
            .first;
    toolchain::SmallVector<mlir::Value> boundsOpsVec =
        fir::factory::genImplicitBoundsOps<mlir::omp::MapBoundsOp,
                                           mlir::omp::MapBoundsType>(
            builder, info, extendedValue,
            /*dataExvIsAssumedSize=*/false, loc);
    for (auto bounds : boundsOpsVec)
      boundsOps.push_back(bounds);
  }
};
} // namespace
