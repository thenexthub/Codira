//===- CastOps.cpp - MLIR SPIR-V Cast Ops  --------------------------------===//
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
// Defines the cast and conversion operations in the SPIR-V dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"

#include "SPIRVOpUtils.h"
#include "SPIRVParsingUtils.h"

#include "vm/core/ADT/TypeSwitch.h"

using namespace mlir::spirv::AttrNames;

namespace mlir::spirv {

static LogicalResult verifyCastOp(Operation *op,
                                  bool requireSameBitWidth = true,
                                  bool skipBitWidthCheck = false) {
  // Some CastOps have no limit on bit widths for result and operand type.
  if (skipBitWidthCheck)
    return success();

  Type operandType = op->getOperand(0).getType();
  Type resultType = op->getResult(0).getType();

  // ODS checks that result type and operand type have the same shape. Check
  // that composite types match and extract the element types, if any.
  using TypePair = std::pair<Type, Type>;
  auto [operandElemTy, resultElemTy] =
      TypeSwitch<Type, TypePair>(operandType)
          .Case<VectorType, spirv::CooperativeMatrixType>(
              [resultType](auto concreteOperandTy) -> TypePair {
                if (auto concreteResultTy =
                        dyn_cast<decltype(concreteOperandTy)>(resultType)) {
                  return {concreteOperandTy.getElementType(),
                          concreteResultTy.getElementType()};
                }
                return {};
              })
          .Default([resultType](Type operandType) -> TypePair {
            return {operandType, resultType};
          });

  if (!operandElemTy || !resultElemTy)
    return op->emitOpError("incompatible operand and result types");

  unsigned operandTypeBitWidth = operandElemTy.getIntOrFloatBitWidth();
  unsigned resultTypeBitWidth = resultElemTy.getIntOrFloatBitWidth();
  bool isSameBitWidth = operandTypeBitWidth == resultTypeBitWidth;

  if (requireSameBitWidth) {
    if (!isSameBitWidth) {
      return op->emitOpError(
                 "expected the same bit widths for operand type and result "
                 "type, but provided ")
             << operandElemTy << " and " << resultElemTy;
    }
    return success();
  }

  if (isSameBitWidth) {
    return op->emitOpError(
               "expected the different bit widths for operand type and result "
               "type, but provided ")
           << operandElemTy << " and " << resultElemTy;
  }
  return success();
}

//===----------------------------------------------------------------------===//
// spirv.BitcastOp
//===----------------------------------------------------------------------===//

LogicalResult BitcastOp::verify() {
  // TODO: The SPIR-V spec validation rules are different for different
  // versions.
  auto operandType = getOperand().getType();
  auto resultType = getResult().getType();
  if (operandType == resultType) {
    return emitError("result type must be different from operand type");
  }
  if (isa<spirv::PointerType>(operandType) &&
      !isa<spirv::PointerType>(resultType)) {
    return emitError(
        "unhandled bit cast conversion from pointer type to non-pointer type");
  }
  if (!isa<spirv::PointerType>(operandType) &&
      isa<spirv::PointerType>(resultType)) {
    return emitError(
        "unhandled bit cast conversion from non-pointer type to pointer type");
  }
  auto operandBitWidth = getBitWidth(operandType);
  auto resultBitWidth = getBitWidth(resultType);
  if (operandBitWidth != resultBitWidth) {
    return emitOpError("mismatch in result type bitwidth ")
           << resultBitWidth << " and operand type bitwidth "
           << operandBitWidth;
  }
  return success();
}

//===----------------------------------------------------------------------===//
// spirv.ConvertPtrToUOp
//===----------------------------------------------------------------------===//

LogicalResult ConvertPtrToUOp::verify() {
  auto operandType = cast<spirv::PointerType>(getPointer().getType());
  auto resultType = cast<spirv::ScalarType>(getResult().getType());
  if (!resultType || !resultType.isSignlessInteger())
    return emitError("result must be a scalar type of unsigned integer");
  auto spirvModule = (*this)->getParentOfType<spirv::ModuleOp>();
  if (!spirvModule)
    return success();
  auto addressingModel = spirvModule.getAddressingModel();
  if ((addressingModel == spirv::AddressingModel::Logical) ||
      (addressingModel == spirv::AddressingModel::PhysicalStorageBuffer64 &&
       operandType.getStorageClass() !=
           spirv::StorageClass::PhysicalStorageBuffer))
    return emitError("operand must be a physical pointer");
  return success();
}

//===----------------------------------------------------------------------===//
// spirv.ConvertUToPtrOp
//===----------------------------------------------------------------------===//

LogicalResult ConvertUToPtrOp::verify() {
  auto operandType = cast<spirv::ScalarType>(getOperand().getType());
  auto resultType = cast<spirv::PointerType>(getResult().getType());
  if (!operandType || !operandType.isSignlessInteger())
    return emitError("result must be a scalar type of unsigned integer");
  auto spirvModule = (*this)->getParentOfType<spirv::ModuleOp>();
  if (!spirvModule)
    return success();
  auto addressingModel = spirvModule.getAddressingModel();
  if ((addressingModel == spirv::AddressingModel::Logical) ||
      (addressingModel == spirv::AddressingModel::PhysicalStorageBuffer64 &&
       resultType.getStorageClass() !=
           spirv::StorageClass::PhysicalStorageBuffer))
    return emitError("result must be a physical pointer");
  return success();
}

//===----------------------------------------------------------------------===//
// spirv.PtrCastToGenericOp
//===----------------------------------------------------------------------===//

LogicalResult PtrCastToGenericOp::verify() {
  auto operandType = cast<spirv::PointerType>(getPointer().getType());
  auto resultType = cast<spirv::PointerType>(getResult().getType());

  spirv::StorageClass operandStorage = operandType.getStorageClass();
  if (operandStorage != spirv::StorageClass::Workgroup &&
      operandStorage != spirv::StorageClass::CrossWorkgroup &&
      operandStorage != spirv::StorageClass::Function)
    return emitError("pointer must point to the Workgroup, CrossWorkgroup"
                     ", or Function Storage Class");

  spirv::StorageClass resultStorage = resultType.getStorageClass();
  if (resultStorage != spirv::StorageClass::Generic)
    return emitError("result type must be of storage class Generic");

  Type operandPointeeType = operandType.getPointeeType();
  Type resultPointeeType = resultType.getPointeeType();
  if (operandPointeeType != resultPointeeType)
    return emitOpError("pointer operand's pointee type must have the same "
                       "as the op result type, but found ")
           << operandPointeeType << " vs " << resultPointeeType;
  return success();
}

//===----------------------------------------------------------------------===//
// spirv.GenericCastToPtrOp
//===----------------------------------------------------------------------===//

LogicalResult GenericCastToPtrOp::verify() {
  auto operandType = cast<spirv::PointerType>(getPointer().getType());
  auto resultType = cast<spirv::PointerType>(getResult().getType());

  spirv::StorageClass operandStorage = operandType.getStorageClass();
  if (operandStorage != spirv::StorageClass::Generic)
    return emitError("pointer type must be of storage class Generic");

  spirv::StorageClass resultStorage = resultType.getStorageClass();
  if (resultStorage != spirv::StorageClass::Workgroup &&
      resultStorage != spirv::StorageClass::CrossWorkgroup &&
      resultStorage != spirv::StorageClass::Function)
    return emitError("result must point to the Workgroup, CrossWorkgroup, "
                     "or Function Storage Class");

  Type operandPointeeType = operandType.getPointeeType();
  Type resultPointeeType = resultType.getPointeeType();
  if (operandPointeeType != resultPointeeType)
    return emitOpError("pointer operand's pointee type must have the same "
                       "as the op result type, but found ")
           << operandPointeeType << " vs " << resultPointeeType;
  return success();
}

//===----------------------------------------------------------------------===//
// spirv.GenericCastToPtrExplicitOp
//===----------------------------------------------------------------------===//

LogicalResult GenericCastToPtrExplicitOp::verify() {
  auto operandType = cast<spirv::PointerType>(getPointer().getType());
  auto resultType = cast<spirv::PointerType>(getResult().getType());

  spirv::StorageClass operandStorage = operandType.getStorageClass();
  if (operandStorage != spirv::StorageClass::Generic)
    return emitError("pointer type must be of storage class Generic");

  spirv::StorageClass resultStorage = resultType.getStorageClass();
  if (resultStorage != spirv::StorageClass::Workgroup &&
      resultStorage != spirv::StorageClass::CrossWorkgroup &&
      resultStorage != spirv::StorageClass::Function)
    return emitError("result must point to the Workgroup, CrossWorkgroup, "
                     "or Function Storage Class");

  Type operandPointeeType = operandType.getPointeeType();
  Type resultPointeeType = resultType.getPointeeType();
  if (operandPointeeType != resultPointeeType)
    return emitOpError("pointer operand's pointee type must have the same "
                       "as the op result type, but found ")
           << operandPointeeType << " vs " << resultPointeeType;
  return success();
}

//===----------------------------------------------------------------------===//
// spirv.ConvertFToSOp
//===----------------------------------------------------------------------===//

LogicalResult ConvertFToSOp::verify() {
  return verifyCastOp(*this, /*requireSameBitWidth=*/false,
                      /*skipBitWidthCheck=*/true);
}

//===----------------------------------------------------------------------===//
// spirv.ConvertFToUOp
//===----------------------------------------------------------------------===//

LogicalResult ConvertFToUOp::verify() {
  return verifyCastOp(*this, /*requireSameBitWidth=*/false,
                      /*skipBitWidthCheck=*/true);
}

//===----------------------------------------------------------------------===//
// spirv.ConvertSToFOp
//===----------------------------------------------------------------------===//

LogicalResult ConvertSToFOp::verify() {
  return verifyCastOp(*this, /*requireSameBitWidth=*/false,
                      /*skipBitWidthCheck=*/true);
}

//===----------------------------------------------------------------------===//
// spirv.ConvertUToFOp
//===----------------------------------------------------------------------===//

LogicalResult ConvertUToFOp::verify() {
  return verifyCastOp(*this, /*requireSameBitWidth=*/false,
                      /*skipBitWidthCheck=*/true);
}

//===----------------------------------------------------------------------===//
// spirv.FConvertOp
//===----------------------------------------------------------------------===//

LogicalResult spirv::FConvertOp::verify() {
  return verifyCastOp(*this, /*requireSameBitWidth=*/false);
}

//===----------------------------------------------------------------------===//
// spirv.SConvertOp
//===----------------------------------------------------------------------===//

LogicalResult spirv::SConvertOp::verify() {
  return verifyCastOp(*this, /*requireSameBitWidth=*/false);
}

//===----------------------------------------------------------------------===//
// spirv.UConvertOp
//===----------------------------------------------------------------------===//

LogicalResult spirv::UConvertOp::verify() {
  return verifyCastOp(*this, /*requireSameBitWidth=*/false);
}

} // namespace mlir::spirv
