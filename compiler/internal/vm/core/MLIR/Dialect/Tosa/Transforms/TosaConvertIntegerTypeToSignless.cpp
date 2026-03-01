//===- TosaConvertIntegerTypeToSignless.cpp
//-------------------------------------------===//
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
//===-------------------------------------------------------------------------------===//

// -----------
// Motivation:
// -----------

// The TOSA specification uses a signless type system, which means that
// information about signedness must be encapsulated by the operations
// themselves. For example, tosa.rescale provides the attributes
// `input_unsigned` and `output_unsigned` to indicate whether the input/output
// should be interpreted as unsigned or signed.

// The TOSA dialect, on the other hand, allows the use of signed or unsigned
// types in addition to signless. As such, when converting from TOSA dialect to
// other formats, we need to ensure that we conform to the TOSA specification.

// ---------
// Overview:
// ---------

// This pass converts signed or unsigned integer types to signless. It currently
// does this greedily for all operators and can also change the signature of the
// function. Should the signature of the entrypoint function change, it will be
// the responsibility of the user to carry signedness information of the inputs
// and outputs independently.

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Dialect/Tosa/Transforms/Passes.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
namespace tosa {

#define GEN_PASS_DEF_TOSACONVERTINTEGERTYPETOSIGNLESS
#include "mlir/Dialect/Tosa/Transforms/Passes.h.inc"

namespace {
class ToSignlessTensorTypeConverter : public TypeConverter {
  static Type convertType(Type type) {
    const auto tensorType = dyn_cast<TensorType>(type);
    if (!tensorType)
      return type;

    const auto intType = dyn_cast<IntegerType>(tensorType.getElementType());
    if (!intType ||
        intType.getSignedness() == IntegerType::SignednessSemantics::Signless)
      return type;

    const auto signlessType = IntegerType::get(
        intType.getContext(), intType.getWidth(), IntegerType::Signless);
    return tensorType.cloneWith(std::nullopt, signlessType);
  }

public:
  explicit ToSignlessTensorTypeConverter() { addConversion(convertType); }
};

class ConvertGenericOpWithIntegerTensorType : public ConversionPattern {
public:
  ConvertGenericOpWithIntegerTensorType(TypeConverter &typeConverter,
                                        MLIRContext *context)
      : ConversionPattern(typeConverter, MatchAnyOpTypeTag{}, 0, context) {}

  LogicalResult
  matchAndRewrite(Operation *op, ArrayRef<Value> operands,
                  ConversionPatternRewriter &rewriter) const final {
    // Typically TOSA operators have a single result, but some have an
    // arbitrary number. 4 seems like a good balance as an optimization
    // hint for storing result types.
    constexpr unsigned int numResults = 4;

    // Convert integer types to signless
    SmallVector<Type, numResults> resultTypes;
    if (failed(typeConverter->convertTypes(op->getResultTypes(), resultTypes)))
      return failure();

    // Create new op with replaced operands and results
    auto *newOp = Operation::create(
        op->getLoc(), op->getName(), resultTypes, operands, op->getAttrs(),
        op->getPropertiesStorage(), op->getSuccessors(), op->getNumRegions());

    // Handle regions in e.g. tosa.cond_if and tosa.while_loop
    for (auto regions : toolchain::zip(op->getRegions(), newOp->getRegions())) {
      Region &before = std::get<0>(regions);
      Region &parent = std::get<1>(regions);
      rewriter.inlineRegionBefore(before, parent, parent.end());
      if (failed(rewriter.convertRegionTypes(&parent, *typeConverter)))
        return failure();
    }

    // Replace with rewritten op
    rewriter.insert(newOp);
    rewriter.replaceOp(op, newOp->getResults());
    return success();
  }
};

class ConvertTosaConstWithIntegerTensorType
    : public OpConversionPattern<tosa::ConstOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(tosa::ConstOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    const ElementsAttr oldAttr = op.getValues();
    const auto oldTy = toolchain::cast<ShapedType>(oldAttr.getType());
    const auto newTy =
        toolchain::cast<ShapedType>(typeConverter->convertType(oldTy));
    if (oldTy == newTy)
      return success();

    ElementsAttr newAttr = oldAttr;
    if (auto denseAttr = toolchain::dyn_cast<DenseElementsAttr>(oldAttr)) {
      newAttr = DenseElementsAttr::get(newTy, denseAttr.getRawData());
    } else {
      return rewriter.notifyMatchFailure(op, "unknown elements attribute type");
    }

    rewriter.replaceOpWithNewOp<tosa::ConstOp>(op, newTy, newAttr);
    return success();
  }
};

class TosaConvertIntegerTypeToSignless
    : public impl::TosaConvertIntegerTypeToSignlessBase<
          TosaConvertIntegerTypeToSignless> {
public:
  void runOnOperation() override {
    MLIRContext *context = &getContext();
    ConversionTarget target(*context);
    ToSignlessTensorTypeConverter typeConverter;

    target.addDynamicallyLegalOp<func::FuncOp>([&](func::FuncOp op) {
      return typeConverter.isSignatureLegal(op.getFunctionType()) &&
             typeConverter.isLegal(&op.getBody());
    });
    target.addDynamicallyLegalOp<tosa::ConstOp>([&](tosa::ConstOp op) {
      return typeConverter.isLegal(op.getType()) &&
             typeConverter.isLegal(op.getValues().getType());
    });
    target.markUnknownOpDynamicallyLegal([&](Operation *op) {
      return typeConverter.isLegal(op->getOperandTypes()) &&
             typeConverter.isLegal(op->getResultTypes());
    });

    RewritePatternSet patterns(context);
    populateFunctionOpInterfaceTypeConversionPattern<func::FuncOp>(
        patterns, typeConverter);
    patterns.add<ConvertGenericOpWithIntegerTensorType>(typeConverter, context);
    patterns.add<ConvertTosaConstWithIntegerTensorType>(typeConverter, context);

    if (failed(
            applyFullConversion(getOperation(), target, std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace

} // namespace tosa
} // namespace mlir
