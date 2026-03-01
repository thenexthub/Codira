//===- SMTExtensionOps.cpp - SMT extension for the Transform dialect ------===//
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

#include "mlir/Dialect/Transform/SMTExtension/SMTExtensionOps.h"
#include "mlir/Dialect/SMT/IR/SMTDialect.h"
#include "mlir/Dialect/SMT/IR/SMTOps.h"
#include "mlir/Dialect/Transform/IR/TransformTypes.h"

using namespace mlir;

#define GET_OP_CLASSES
#include "mlir/Dialect/Transform/SMTExtension/SMTExtensionOps.cpp.inc"

//===----------------------------------------------------------------------===//
// ConstrainParamsOp
//===----------------------------------------------------------------------===//

void transform::smt::ConstrainParamsOp::getEffects(
    SmallVectorImpl<MemoryEffects::EffectInstance> &effects) {
  onlyReadsHandle(getParamsMutable(), effects);
  producesHandle(getResults(), effects);
}

DiagnosedSilenceableFailure
transform::smt::ConstrainParamsOp::apply(transform::TransformRewriter &rewriter,
                                         transform::TransformResults &results,
                                         transform::TransformState &state) {
  // TODO: Proper operational semantics are to check the SMT problem in the body
  //       with a SMT solver with the arguments of the body constrained to the
  //       values passed into the op. Success or failure is then determined by
  //       the solver's result.
  //       One way to support this is to just promise the TransformOpInterface
  //       and allow for users to attach their own implementation, which would,
  //       e.g., translate the ops to SMTLIB and hand that over to the user's
  //       favourite solver. This requires changes to the dialect's verifier.
  return emitSilenceableFailure(getLoc())
         << "op does not have interpreted semantics yet";
}

LogicalResult transform::smt::ConstrainParamsOp::verify() {
  auto yieldTerminator =
      dyn_cast<mlir::smt::YieldOp>(getRegion().front().back());
  if (!yieldTerminator)
    return emitOpError() << "expected '"
                         << mlir::smt::YieldOp::getOperationName()
                         << "' as terminator";

  auto checkTypes = [](size_t idx, Type smtType, StringRef smtDesc,
                       Type paramType, StringRef paramDesc,
                       auto *atOp) -> InFlightDiagnostic {
    if (!isa<mlir::smt::BoolType, mlir::smt::IntType, mlir::smt::BitVectorType>(
            smtType))
      return atOp->emitOpError() << "the type of " << smtDesc << " #" << idx
                                 << " is expected to be either a !smt.bool, a "
                                    "!smt.int, or a !smt.bv";

    assert(isa<TransformParamTypeInterface>(paramType) &&
           "ODS specifies params' type should implement param interface");
    if (isa<transform::AnyParamType>(paramType))
      return {}; // No further checks can be done.

    // NB: This cast must succeed as long as the only implementors of
    //     TransformParamTypeInterface are AnyParamType and ParamType.
    Type typeWrappedByParam = cast<ParamType>(paramType).getType();

    if (isa<mlir::smt::IntType>(smtType)) {
      if (!isa<IntegerType>(typeWrappedByParam))
        return atOp->emitOpError()
               << "the type of " << smtDesc << " #" << idx
               << " is !smt.int though the corresponding " << paramDesc
               << " type (" << paramType << ") is not wrapping an integer type";
    } else if (isa<mlir::smt::BoolType>(smtType)) {
      auto wrappedIntType = dyn_cast<IntegerType>(typeWrappedByParam);
      if (!wrappedIntType || wrappedIntType.getWidth() != 1)
        return atOp->emitOpError()
               << "the type of " << smtDesc << " #" << idx
               << " is !smt.bool though the corresponding " << paramDesc
               << " type (" << paramType << ") is not wrapping i1";
    } else if (auto bvSmtType = dyn_cast<mlir::smt::BitVectorType>(smtType)) {
      auto wrappedIntType = dyn_cast<IntegerType>(typeWrappedByParam);
      if (!wrappedIntType || wrappedIntType.getWidth() != bvSmtType.getWidth())
        return atOp->emitOpError()
               << "the type of " << smtDesc << " #" << idx << " is " << smtType
               << " though the corresponding " << paramDesc << " type ("
               << paramType
               << ") is not wrapping an integer type of the same bitwidth";
    }

    return {};
  };

  if (getOperands().size() != getBody().getNumArguments())
    return emitOpError(
        "must have the same number of block arguments as operands");

  for (auto [idx, operandType, blockArgType] :
       toolchain::enumerate(getOperandTypes(), getBody().getArgumentTypes())) {
    InFlightDiagnostic typeCheckResult =
        checkTypes(idx, blockArgType, "block arg", operandType, "operand",
                   /*atOp=*/this);
    if (LogicalResult(typeCheckResult).failed())
      return typeCheckResult;
  }

  for (auto &op : getBody().getOps()) {
    if (!isa<mlir::smt::SMTDialect>(op.getDialect()))
      return emitOpError(
          "ops contained in region should belong to SMT-dialect");
  }

  if (yieldTerminator->getNumOperands() != getNumResults())
    return yieldTerminator.emitOpError()
           << "expected terminator to have as many operands as the parent op "
              "has results";

  for (auto [idx, termOperandType, resultType] : toolchain::enumerate(
           yieldTerminator->getOperands().getType(), getResultTypes())) {
    InFlightDiagnostic typeCheckResult =
        checkTypes(idx, termOperandType, "terminator operand", resultType,
                   "result", /*atOp=*/&yieldTerminator);
    if (LogicalResult(typeCheckResult).failed())
      return typeCheckResult;
  }

  return success();
}
