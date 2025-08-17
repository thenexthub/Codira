//===-- FIROpPatterns.h -- FIR operation conversion patterns ----*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_CODEGEN_FIROPPATTERNS_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_CODEGEN_FIROPPATTERNS_H

#include "language/Compability/Optimizer/CodeGen/TypeConverter.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"

namespace fir {

struct FIRToLLVMPassOptions;

static constexpr unsigned defaultAddressSpace = 0u;

class ConvertFIRToLLVMPattern : public mlir::ConvertToLLVMPattern {
public:
  ConvertFIRToLLVMPattern(toolchain::StringRef rootOpName,
                          mlir::MLIRContext *context,
                          const fir::LLVMTypeConverter &typeConverter,
                          const fir::FIRToLLVMPassOptions &options,
                          mlir::PatternBenefit benefit = 1);

protected:
  mlir::Type convertType(mlir::Type ty) const {
    return lowerTy().convertType(ty);
  }

  // Convert FIR type to LLVM without turning fir.box<T> into memory
  // reference.
  mlir::Type convertObjectType(mlir::Type firType) const;

  mlir::LLVM::ConstantOp
  genI32Constant(mlir::Location loc, mlir::ConversionPatternRewriter &rewriter,
                 int value) const;

  mlir::LLVM::ConstantOp
  genConstantOffset(mlir::Location loc,
                    mlir::ConversionPatternRewriter &rewriter,
                    int offset) const;

  /// Perform an extension or truncation as needed on an integer value. Lowering
  /// to the specific target may involve some sign-extending or truncation of
  /// values, particularly to fit them from abstract box types to the
  /// appropriate reified structures.
  mlir::Value integerCast(mlir::Location loc,
                          mlir::ConversionPatternRewriter &rewriter,
                          mlir::Type ty, mlir::Value val,
                          bool fold = false) const;

  struct TypePair {
    mlir::Type fir;
    mlir::Type toolchain;
  };

  TypePair getBoxTypePair(mlir::Type firBoxTy) const;

  /// Construct code sequence to extract the specific value from a `fir.box`.
  mlir::Value getValueFromBox(mlir::Location loc, TypePair boxTy,
                              mlir::Value box, mlir::Type resultTy,
                              mlir::ConversionPatternRewriter &rewriter,
                              int boxValue) const;

  /// Method to construct code sequence to get the triple for dimension `dim`
  /// from a box.
  toolchain::SmallVector<mlir::Value, 3>
  getDimsFromBox(mlir::Location loc, toolchain::ArrayRef<mlir::Type> retTys,
                 TypePair boxTy, mlir::Value box, mlir::Value dim,
                 mlir::ConversionPatternRewriter &rewriter) const;

  toolchain::SmallVector<mlir::Value, 3>
  getDimsFromBox(mlir::Location loc, toolchain::ArrayRef<mlir::Type> retTys,
                 TypePair boxTy, mlir::Value box, int dim,
                 mlir::ConversionPatternRewriter &rewriter) const;

  mlir::Value
  loadDimFieldFromBox(mlir::Location loc, TypePair boxTy, mlir::Value box,
                      mlir::Value dim, int off, mlir::Type ty,
                      mlir::ConversionPatternRewriter &rewriter) const;

  mlir::Value
  getDimFieldFromBox(mlir::Location loc, TypePair boxTy, mlir::Value box,
                     int dim, int off, mlir::Type ty,
                     mlir::ConversionPatternRewriter &rewriter) const;

  mlir::Value getStrideFromBox(mlir::Location loc, TypePair boxTy,
                               mlir::Value box, unsigned dim,
                               mlir::ConversionPatternRewriter &rewriter) const;

  /// Read base address from a fir.box. Returned address has type ty.
  mlir::Value
  getBaseAddrFromBox(mlir::Location loc, TypePair boxTy, mlir::Value box,
                     mlir::ConversionPatternRewriter &rewriter) const;

  mlir::Value
  getElementSizeFromBox(mlir::Location loc, mlir::Type resultTy, TypePair boxTy,
                        mlir::Value box,
                        mlir::ConversionPatternRewriter &rewriter) const;

  mlir::Value getRankFromBox(mlir::Location loc, TypePair boxTy,
                             mlir::Value box,
                             mlir::ConversionPatternRewriter &rewriter) const;

  mlir::Value getExtraFromBox(mlir::Location loc, TypePair boxTy,
                              mlir::Value box,
                              mlir::ConversionPatternRewriter &rewriter) const;

  // Get the element type given an LLVM type that is of the form
  // (array|struct|vector)+ and the provided indexes.
  mlir::Type getBoxEleTy(mlir::Type type,
                         toolchain::ArrayRef<std::int64_t> indexes) const;

  // Return LLVM type of the object described by a fir.box of \p boxType.
  mlir::Type getLlvmObjectTypeFromBoxType(mlir::Type boxType) const;

  /// Read the address of the type descriptor from a box.
  mlir::Value
  loadTypeDescAddress(mlir::Location loc, TypePair boxTy, mlir::Value box,
                      mlir::ConversionPatternRewriter &rewriter) const;

  // Load the attribute from the \p box and perform a check against \p maskValue
  // The final comparison is implemented as `(attribute & maskValue) != 0`.
  mlir::Value genBoxAttributeCheck(mlir::Location loc, TypePair boxTy,
                                   mlir::Value box,
                                   mlir::ConversionPatternRewriter &rewriter,
                                   unsigned maskValue) const;

  /// Compute the descriptor size in bytes. The result is not guaranteed to be a
  /// compile time constant if the box is for an assumed rank, in which case the
  /// box rank will be read.
  mlir::Value computeBoxSize(mlir::Location, TypePair boxTy, mlir::Value box,
                             mlir::ConversionPatternRewriter &rewriter) const;

  template <typename... ARGS>
  mlir::LLVM::GEPOp genGEP(mlir::Location loc, mlir::Type ty,
                           mlir::ConversionPatternRewriter &rewriter,
                           mlir::Value base, ARGS... args) const {
    toolchain::SmallVector<mlir::LLVM::GEPArg> cv = {args...};
    auto toolchainPtrTy =
        mlir::LLVM::LLVMPointerType::get(ty.getContext(), /*addressSpace=*/0);
    return mlir::LLVM::GEPOp::create(rewriter, loc, toolchainPtrTy, ty, base, cv);
  }

  // Find the Block in which the alloca should be inserted.
  // The order to recursively find the proper block:
  // 1. An OpenMP Op that will be outlined.
  // 2. An OpenMP or OpenACC Op with one or more regions holding executable
  // code.
  // 3. A LLVMFuncOp
  // 4. The first ancestor that is one of the above.
  mlir::Block *getBlockForAllocaInsert(mlir::Operation *op,
                                       mlir::Region *parentRegion) const;

  // Generate an alloca of size 1 for an object of type \p toolchainObjectTy in the
  // allocation address space provided for the architecture in the DataLayout
  // specification. If the address space is different from the devices
  // program address space we perform a cast. In the case of most architectures
  // the program and allocation address space will be the default of 0 and no
  // cast will be emitted.
  mlir::Value
  genAllocaAndAddrCastWithType(mlir::Location loc, mlir::Type toolchainObjectTy,
                               unsigned alignment,
                               mlir::ConversionPatternRewriter &rewriter) const;

  const fir::LLVMTypeConverter &lowerTy() const {
    return *static_cast<const fir::LLVMTypeConverter *>(
        this->getTypeConverter());
  }

  const mlir::DataLayout &getDataLayout() const {
    return lowerTy().getDataLayout();
  }

  void attachTBAATag(mlir::LLVM::AliasAnalysisOpInterface op,
                     mlir::Type baseFIRType, mlir::Type accessFIRType,
                     mlir::LLVM::GEPOp gep) const {
    lowerTy().attachTBAATag(op, baseFIRType, accessFIRType, gep);
  }

  unsigned
  getAllocaAddressSpace(mlir::ConversionPatternRewriter &rewriter) const;

  unsigned
  getProgramAddressSpace(mlir::ConversionPatternRewriter &rewriter) const;

  unsigned
  getGlobalAddressSpace(mlir::ConversionPatternRewriter &rewriter) const;

  const fir::FIRToLLVMPassOptions &options;

  using ConvertToLLVMPattern::matchAndRewrite;
};

template <typename SourceOp>
class FIROpConversion : public ConvertFIRToLLVMPattern {
public:
  using OpAdaptor = typename SourceOp::Adaptor;
  using OneToNOpAdaptor = typename SourceOp::template GenericAdaptor<
      mlir::ArrayRef<mlir::ValueRange>>;

  explicit FIROpConversion(const LLVMTypeConverter &typeConverter,
                           const fir::FIRToLLVMPassOptions &options,
                           mlir::PatternBenefit benefit = 1)
      : ConvertFIRToLLVMPattern(SourceOp::getOperationName(),
                                &typeConverter.getContext(), typeConverter,
                                options, benefit) {}

  /// Wrappers around the RewritePattern methods that pass the derived op type.
  toolchain::LogicalResult
  matchAndRewrite(mlir::Operation *op, mlir::ArrayRef<mlir::Value> operands,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewrite(mlir::cast<SourceOp>(op),
                           OpAdaptor(operands, mlir::cast<SourceOp>(op)),
                           rewriter);
  }
  toolchain::LogicalResult
  matchAndRewrite(mlir::Operation *op,
                  mlir::ArrayRef<mlir::ValueRange> operands,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    auto sourceOp = mlir::cast<SourceOp>(op);
    return matchAndRewrite(sourceOp, OneToNOpAdaptor(operands, sourceOp),
                           rewriter);
  }
  /// Methods that operate on the SourceOp type. These must be
  /// overridden by the derived pattern class.
  virtual toolchain::LogicalResult
  matchAndRewrite(SourceOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const {
    toolchain_unreachable("matchAndRewrite is not implemented");
  }
  virtual toolchain::LogicalResult
  matchAndRewrite(SourceOp op, OneToNOpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const {
    toolchain::SmallVector<mlir::Value> oneToOneOperands =
        getOneToOneAdaptorOperands(adaptor.getOperands());
    return matchAndRewrite(op, OpAdaptor(oneToOneOperands, adaptor), rewriter);
  }

private:
  using ConvertFIRToLLVMPattern::matchAndRewrite;
};

/// FIR conversion pattern template
template <typename FromOp>
class FIROpAndTypeConversion : public FIROpConversion<FromOp> {
public:
  using FIROpConversion<FromOp>::FIROpConversion;
  using OpAdaptor = typename FromOp::Adaptor;

  toolchain::LogicalResult
  matchAndRewrite(FromOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    mlir::Type ty = this->convertType(op.getType());
    return doRewrite(op, ty, adaptor, rewriter);
  }

  virtual toolchain::LogicalResult
  doRewrite(FromOp addr, mlir::Type ty, OpAdaptor adaptor,
            mlir::ConversionPatternRewriter &rewriter) const = 0;
};

} // namespace fir

#endif // FORTRAN_OPTIMIZER_CODEGEN_FIROPPATTERNS_H
