//===-- CUDA.cpp -- CUDA Fortran specific lowering ------------------------===//
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
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Lower/CUDA.h"
#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"

#define DEBUG_TYPE "flang-lower-cuda"

void language::Compability::lower::initializeDeviceComponentAllocator(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::semantics::Symbol &sym, const fir::MutableBoxValue &box) {
  if (const auto *details{
          sym.GetUltimate()
              .detailsIf<language::Compability::semantics::ObjectEntityDetails>()}) {
    const language::Compability::semantics::DeclTypeSpec *type{details->type()};
    const language::Compability::semantics::DerivedTypeSpec *derived{type ? type->AsDerived()
                                                            : nullptr};
    if (derived) {
      if (!FindCUDADeviceAllocatableUltimateComponent(*derived))
        return; // No device components.

      fir::FirOpBuilder &builder = converter.getFirOpBuilder();
      mlir::Location loc = converter.getCurrentLocation();

      mlir::Type baseTy = fir::unwrapRefType(box.getAddr().getType());

      // Only pointer and allocatable needs post allocation initialization
      // of components descriptors.
      if (!fir::isAllocatableType(baseTy) && !fir::isPointerType(baseTy))
        return;

      // Extract the derived type.
      mlir::Type ty = fir::getDerivedType(baseTy);
      auto recTy = mlir::dyn_cast<fir::RecordType>(ty);
      assert(recTy && "expected fir::RecordType");

      if (auto boxTy = mlir::dyn_cast<fir::BaseBoxType>(baseTy))
        baseTy = boxTy.getEleTy();
      baseTy = fir::unwrapRefType(baseTy);

      language::Compability::semantics::UltimateComponentIterator components{*derived};
      mlir::Value loadedBox = fir::LoadOp::create(builder, loc, box.getAddr());
      mlir::Value addr;
      if (auto seqTy = mlir::dyn_cast<fir::SequenceType>(baseTy)) {
        mlir::Type idxTy = builder.getIndexType();
        mlir::Value one = builder.createIntegerConstant(loc, idxTy, 1);
        mlir::Value zero = builder.createIntegerConstant(loc, idxTy, 0);
        toolchain::SmallVector<fir::DoLoopOp> loops;
        toolchain::SmallVector<mlir::Value> indices;
        toolchain::SmallVector<mlir::Value> extents;
        for (unsigned i = 0; i < seqTy.getDimension(); ++i) {
          mlir::Value dim = builder.createIntegerConstant(loc, idxTy, i);
          auto dimInfo = fir::BoxDimsOp::create(builder, loc, idxTy, idxTy,
                                                idxTy, loadedBox, dim);
          mlir::Value lbub = mlir::arith::AddIOp::create(
              builder, loc, dimInfo.getResult(0), dimInfo.getResult(1));
          mlir::Value ext =
              mlir::arith::SubIOp::create(builder, loc, lbub, one);
          mlir::Value cmp = mlir::arith::CmpIOp::create(
              builder, loc, mlir::arith::CmpIPredicate::sgt, ext, zero);
          ext = mlir::arith::SelectOp::create(builder, loc, cmp, ext, zero);
          extents.push_back(ext);

          auto loop = fir::DoLoopOp::create(
              builder, loc, dimInfo.getResult(0), dimInfo.getResult(1),
              dimInfo.getResult(2), /*isUnordered=*/true,
              /*finalCount=*/false, mlir::ValueRange{});
          loops.push_back(loop);
          indices.push_back(loop.getInductionVar());
          builder.setInsertionPointToStart(loop.getBody());
        }
        mlir::Value boxAddr = fir::BoxAddrOp::create(builder, loc, loadedBox);
        auto shape = fir::ShapeOp::create(builder, loc, extents);
        addr = fir::ArrayCoorOp::create(
            builder, loc, fir::ReferenceType::get(recTy), boxAddr, shape,
            /*slice=*/mlir::Value{}, indices, /*typeparms=*/mlir::ValueRange{});
      } else {
        addr = fir::BoxAddrOp::create(builder, loc, loadedBox);
      }
      for (const auto &compSym : components) {
        if (language::Compability::semantics::IsDeviceAllocatable(compSym)) {
          toolchain::SmallVector<mlir::Value> coord;
          mlir::Type fieldTy = gatherDeviceComponentCoordinatesAndType(
              builder, loc, compSym, recTy, coord);
          assert(coord.size() == 1 && "expect one coordinate");
          mlir::Value comp = fir::CoordinateOp::create(
              builder, loc, builder.getRefType(fieldTy), addr, coord[0]);
          cuf::DataAttributeAttr dataAttr =
              language::Compability::lower::translateSymbolCUFDataAttribute(
                  builder.getContext(), compSym);
          cuf::SetAllocatorIndexOp::create(builder, loc, comp, dataAttr);
        }
      }
    }
  }
}

mlir::Type language::Compability::lower::gatherDeviceComponentCoordinatesAndType(
    fir::FirOpBuilder &builder, mlir::Location loc,
    const language::Compability::semantics::Symbol &sym, fir::RecordType recTy,
    toolchain::SmallVector<mlir::Value> &coordinates) {
  unsigned fieldIdx = recTy.getFieldIndex(sym.name().ToString());
  mlir::Type fieldTy;
  if (fieldIdx != std::numeric_limits<unsigned>::max()) {
    // Field found in the base record type.
    auto fieldName = recTy.getTypeList()[fieldIdx].first;
    fieldTy = recTy.getTypeList()[fieldIdx].second;
    mlir::Value fieldIndex = fir::FieldIndexOp::create(
        builder, loc, fir::FieldType::get(fieldTy.getContext()), fieldName,
        recTy,
        /*typeParams=*/mlir::ValueRange{});
    coordinates.push_back(fieldIndex);
  } else {
    // Field not found in base record type, search in potential
    // record type components.
    for (auto component : recTy.getTypeList()) {
      if (auto childRecTy = mlir::dyn_cast<fir::RecordType>(component.second)) {
        fieldIdx = childRecTy.getFieldIndex(sym.name().ToString());
        if (fieldIdx != std::numeric_limits<unsigned>::max()) {
          mlir::Value parentFieldIndex = fir::FieldIndexOp::create(
              builder, loc, fir::FieldType::get(childRecTy.getContext()),
              component.first, recTy,
              /*typeParams=*/mlir::ValueRange{});
          coordinates.push_back(parentFieldIndex);
          auto fieldName = childRecTy.getTypeList()[fieldIdx].first;
          fieldTy = childRecTy.getTypeList()[fieldIdx].second;
          mlir::Value childFieldIndex = fir::FieldIndexOp::create(
              builder, loc, fir::FieldType::get(fieldTy.getContext()),
              fieldName, childRecTy,
              /*typeParams=*/mlir::ValueRange{});
          coordinates.push_back(childFieldIndex);
          break;
        }
      }
    }
  }
  if (coordinates.empty())
    TODO(loc, "device resident component in complex derived-type hierarchy");
  return fieldTy;
}

cuf::DataAttributeAttr language::Compability::lower::translateSymbolCUFDataAttribute(
    mlir::MLIRContext *mlirContext, const language::Compability::semantics::Symbol &sym) {
  std::optional<language::Compability::common::CUDADataAttr> cudaAttr =
      language::Compability::semantics::GetCUDADataAttr(&sym.GetUltimate());
  return cuf::getDataAttribute(mlirContext, cudaAttr);
}

bool language::Compability::lower::isTransferWithConversion(mlir::Value rhs) {
  if (auto elOp = mlir::dyn_cast<hlfir::ElementalOp>(rhs.getDefiningOp()))
    if (toolchain::hasSingleElement(elOp.getBody()->getOps<hlfir::DesignateOp>()) &&
        toolchain::hasSingleElement(elOp.getBody()->getOps<fir::LoadOp>()) == 1 &&
        toolchain::hasSingleElement(elOp.getBody()->getOps<fir::ConvertOp>()) == 1)
      return true;
  return false;
}
