//===- ModuleTranslation.cpp - MLIR to LLVM conversion --------------------===//
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
// This file implements the translation between an MLIR LLVM dialect module and
// the corresponding LLVMIR module. It only handles core LLVM IR operations.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/ModuleTranslation.h"

#include "AttrKindDetail.h"
#include "DebugTranslation.h"
#include "LoopAnnotationTranslation.h"
#include "mlir/Analysis/TopologicalSortUtils.h"
#include "mlir/Dialect/DLTI/DLTI.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMInterfaces.h"
#include "mlir/Dialect/LLVMIR/Transforms/DIExpressionLegalization.h"
#include "mlir/Dialect/LLVMIR/Transforms/LegalizeForExport.h"
#include "mlir/IR/AttrTypeSubElements.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/DialectResourceBlobManager.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Target/LLVMIR/LLVMTranslationInterface.h"
#include "mlir/Target/LLVMIR/TypeToLLVM.h"

#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/TypeSwitch.h"
#include "vm/core/Analysis/TargetFolder.h"
#include "vm/core/Frontend/OpenMP/OMPIRBuilder.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/CFG.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/InlineAsm.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/MDBuilder.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Verifier.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"
#include "vm/core/Transforms/Utils/Cloning.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"
#include <numeric>
#include <optional>

#define DEBUG_TYPE "toolchain-dialect-to-toolchain-ir"

using namespace mlir;
using namespace mlir::LLVM;
using namespace mlir::LLVM::detail;

#include "mlir/Dialect/LLVMIR/LLVMConversionEnumsToLLVM.inc"

namespace {
/// A customized inserter for LLVM's IRBuilder that captures all LLVM IR
/// instructions that are created for future reference.
///
/// This is intended to be used with the `CollectionScope` RAII object:
///
///     toolchain::IRBuilder<..., InstructionCapturingInserter> builder;
///     {
///       InstructionCapturingInserter::CollectionScope scope(builder);
///       // Call IRBuilder methods as usual.
///
///       // This will return a list of all instructions created by the builder,
///       // in order of creation.
///       builder.getInserter().getCapturedInstructions();
///     }
///     // This will return an empty list.
///     builder.getInserter().getCapturedInstructions();
///
/// The capturing functionality is _disabled_ by default for performance
/// consideration. It needs to be explicitly enabled, which is achieved by
/// creating a `CollectionScope`.
class InstructionCapturingInserter : public toolchain::IRBuilderCallbackInserter {
public:
  /// Constructs the inserter.
  InstructionCapturingInserter()
      : toolchain::IRBuilderCallbackInserter([this](toolchain::Instruction *instruction) {
          if (LLVM_LIKELY(enabled))
            capturedInstructions.push_back(instruction);
        }) {}

  /// Returns the list of LLVM IR instructions captured since the last cleanup.
  ArrayRef<toolchain::Instruction *> getCapturedInstructions() const {
    return capturedInstructions;
  }

  /// Clears the list of captured LLVM IR instructions.
  void clearCapturedInstructions() { capturedInstructions.clear(); }

  /// RAII object enabling the capture of created LLVM IR instructions.
  class CollectionScope {
  public:
    /// Creates the scope for the given inserter.
    CollectionScope(toolchain::IRBuilderBase &irBuilder, bool isBuilderCapturing);

    /// Ends the scope.
    ~CollectionScope();

    ArrayRef<toolchain::Instruction *> getCapturedInstructions() {
      if (!inserter)
        return {};
      return inserter->getCapturedInstructions();
    }

  private:
    /// Back reference to the inserter.
    InstructionCapturingInserter *inserter = nullptr;

    /// List of instructions in the inserter prior to this scope.
    SmallVector<toolchain::Instruction *> previouslyCollectedInstructions;

    /// Whether the inserter was enabled prior to this scope.
    bool wasEnabled;
  };

  /// Enable or disable the capturing mechanism.
  void setEnabled(bool enabled = true) { this->enabled = enabled; }

private:
  /// List of captured instructions.
  SmallVector<toolchain::Instruction *> capturedInstructions;

  /// Whether the collection is enabled.
  bool enabled = false;
};

using CapturingIRBuilder =
    toolchain::IRBuilder<toolchain::TargetFolder, InstructionCapturingInserter>;
} // namespace

InstructionCapturingInserter::CollectionScope::CollectionScope(
    toolchain::IRBuilderBase &irBuilder, bool isBuilderCapturing) {

  if (!isBuilderCapturing)
    return;

  auto &capturingIRBuilder = static_cast<CapturingIRBuilder &>(irBuilder);
  inserter = &capturingIRBuilder.getInserter();
  wasEnabled = inserter->enabled;
  if (wasEnabled)
    previouslyCollectedInstructions.swap(inserter->capturedInstructions);
  inserter->setEnabled(true);
}

InstructionCapturingInserter::CollectionScope::~CollectionScope() {
  if (!inserter)
    return;

  previouslyCollectedInstructions.swap(inserter->capturedInstructions);
  // If collection was enabled (likely in another, surrounding scope), keep
  // the instructions collected in this scope.
  if (wasEnabled) {
    toolchain::append_range(inserter->capturedInstructions,
                       previouslyCollectedInstructions);
  }
  inserter->setEnabled(wasEnabled);
}

/// Translates the given data layout spec attribute to the LLVM IR data layout.
/// Only integer, float, pointer and endianness entries are currently supported.
static FailureOr<toolchain::DataLayout>
translateDataLayout(DataLayoutSpecInterface attribute,
                    const DataLayout &dataLayout,
                    std::optional<Location> loc = std::nullopt) {
  if (!loc)
    loc = UnknownLoc::get(attribute.getContext());

  // Translate the endianness attribute.
  std::string llvmDataLayout;
  toolchain::raw_string_ostream layoutStream(llvmDataLayout);
  for (DataLayoutEntryInterface entry : attribute.getEntries()) {
    auto key = toolchain::dyn_cast_if_present<StringAttr>(entry.getKey());
    if (!key)
      continue;
    if (key.getValue() == DLTIDialect::kDataLayoutEndiannessKey) {
      auto value = cast<StringAttr>(entry.getValue());
      bool isLittleEndian =
          value.getValue() == DLTIDialect::kDataLayoutEndiannessLittle;
      layoutStream << "-" << (isLittleEndian ? "e" : "E");
      continue;
    }
    if (key.getValue() == DLTIDialect::kDataLayoutManglingModeKey) {
      auto value = cast<StringAttr>(entry.getValue());
      layoutStream << "-m:" << value.getValue();
      continue;
    }
    if (key.getValue() == DLTIDialect::kDataLayoutProgramMemorySpaceKey) {
      auto value = cast<IntegerAttr>(entry.getValue());
      uint64_t space = value.getValue().getZExtValue();
      // Skip the default address space.
      if (space == 0)
        continue;
      layoutStream << "-P" << space;
      continue;
    }
    if (key.getValue() == DLTIDialect::kDataLayoutGlobalMemorySpaceKey) {
      auto value = cast<IntegerAttr>(entry.getValue());
      uint64_t space = value.getValue().getZExtValue();
      // Skip the default address space.
      if (space == 0)
        continue;
      layoutStream << "-G" << space;
      continue;
    }
    if (key.getValue() == DLTIDialect::kDataLayoutAllocaMemorySpaceKey) {
      auto value = cast<IntegerAttr>(entry.getValue());
      uint64_t space = value.getValue().getZExtValue();
      // Skip the default address space.
      if (space == 0)
        continue;
      layoutStream << "-A" << space;
      continue;
    }
    if (key.getValue() == DLTIDialect::kDataLayoutStackAlignmentKey) {
      auto value = cast<IntegerAttr>(entry.getValue());
      uint64_t alignment = value.getValue().getZExtValue();
      // Skip the default stack alignment.
      if (alignment == 0)
        continue;
      layoutStream << "-S" << alignment;
      continue;
    }
    if (key.getValue() == DLTIDialect::kDataLayoutFunctionPointerAlignmentKey) {
      auto value = cast<FunctionPointerAlignmentAttr>(entry.getValue());
      uint64_t alignment = value.getAlignment();
      // Skip the default function pointer alignment.
      if (alignment == 0)
        continue;
      layoutStream << "-F" << (value.getFunctionDependent() ? "n" : "i")
                   << alignment;
      continue;
    }
    if (key.getValue() == DLTIDialect::kDataLayoutLegalIntWidthsKey) {
      layoutStream << "-n";
      toolchain::interleave(
          cast<DenseI32ArrayAttr>(entry.getValue()).asArrayRef(), layoutStream,
          [&](int32_t val) { layoutStream << val; }, ":");
      continue;
    }
    emitError(*loc) << "unsupported data layout key " << key;
    return failure();
  }

  // Go through the list of entries to check which types are explicitly
  // specified in entries. Where possible, data layout queries are used instead
  // of directly inspecting the entries.
  for (DataLayoutEntryInterface entry : attribute.getEntries()) {
    auto type = toolchain::dyn_cast_if_present<Type>(entry.getKey());
    if (!type)
      continue;
    // Data layout for the index type is irrelevant at this point.
    if (isa<IndexType>(type))
      continue;
    layoutStream << "-";
    LogicalResult result =
        toolchain::TypeSwitch<Type, LogicalResult>(type)
            .Case<IntegerType, Float16Type, Float32Type, Float64Type,
                  Float80Type, Float128Type>([&](Type type) -> LogicalResult {
              if (auto intType = dyn_cast<IntegerType>(type)) {
                if (intType.getSignedness() != IntegerType::Signless)
                  return emitError(*loc)
                         << "unsupported data layout for non-signless integer "
                         << intType;
                layoutStream << "i";
              } else {
                layoutStream << "f";
              }
              uint64_t size = dataLayout.getTypeSizeInBits(type);
              uint64_t abi = dataLayout.getTypeABIAlignment(type) * 8u;
              uint64_t preferred =
                  dataLayout.getTypePreferredAlignment(type) * 8u;
              layoutStream << size << ":" << abi;
              if (abi != preferred)
                layoutStream << ":" << preferred;
              return success();
            })
            .Case([&](LLVMPointerType type) {
              layoutStream << "p" << type.getAddressSpace() << ":";
              uint64_t size = dataLayout.getTypeSizeInBits(type);
              uint64_t abi = dataLayout.getTypeABIAlignment(type) * 8u;
              uint64_t preferred =
                  dataLayout.getTypePreferredAlignment(type) * 8u;
              uint64_t index = *dataLayout.getTypeIndexBitwidth(type);
              layoutStream << size << ":" << abi << ":" << preferred << ":"
                           << index;
              return success();
            })
            .Default([loc](Type type) {
              return emitError(*loc)
                     << "unsupported type in data layout: " << type;
            });
    if (failed(result))
      return failure();
  }
  StringRef layoutSpec(llvmDataLayout);
  layoutSpec.consume_front("-");

  return toolchain::DataLayout(layoutSpec);
}

/// Builds a constant of a sequential LLVM type `type`, potentially containing
/// other sequential types recursively, from the individual constant values
/// provided in `constants`. `shape` contains the number of elements in nested
/// sequential types. Reports errors at `loc` and returns nullptr on error.
static toolchain::Constant *
buildSequentialConstant(ArrayRef<toolchain::Constant *> &constants,
                        ArrayRef<int64_t> shape, toolchain::Type *type,
                        Location loc) {
  if (shape.empty()) {
    toolchain::Constant *result = constants.front();
    constants = constants.drop_front();
    return result;
  }

  toolchain::Type *elementType;
  if (auto *arrayTy = dyn_cast<toolchain::ArrayType>(type)) {
    elementType = arrayTy->getElementType();
  } else if (auto *vectorTy = dyn_cast<toolchain::VectorType>(type)) {
    elementType = vectorTy->getElementType();
  } else {
    emitError(loc) << "expected sequential LLVM types wrapping a scalar";
    return nullptr;
  }

  SmallVector<toolchain::Constant *, 8> nested;
  nested.reserve(shape.front());
  for (int64_t i = 0; i < shape.front(); ++i) {
    nested.push_back(buildSequentialConstant(constants, shape.drop_front(),
                                             elementType, loc));
    if (!nested.back())
      return nullptr;
  }

  if (shape.size() == 1 && type->isVectorTy())
    return toolchain::ConstantVector::get(nested);
  return toolchain::ConstantArray::get(
      toolchain::ArrayType::get(elementType, shape.front()), nested);
}

/// Returns the first non-sequential type nested in sequential types.
static toolchain::Type *getInnermostElementType(toolchain::Type *type) {
  do {
    if (auto *arrayTy = dyn_cast<toolchain::ArrayType>(type)) {
      type = arrayTy->getElementType();
    } else if (auto *vectorTy = dyn_cast<toolchain::VectorType>(type)) {
      type = vectorTy->getElementType();
    } else {
      return type;
    }
  } while (true);
}

/// Convert a dense elements attribute to an LLVM IR constant using its raw data
/// storage if possible. This supports elements attributes of tensor or vector
/// type and avoids constructing separate objects for individual values of the
/// innermost dimension. Constants for other dimensions are still constructed
/// recursively. Returns null if constructing from raw data is not supported for
/// this type, e.g., element type is not a power-of-two-sized primitive. Reports
/// other errors at `loc`.
static toolchain::Constant *
convertDenseElementsAttr(Location loc, DenseElementsAttr denseElementsAttr,
                         toolchain::Type *llvmType,
                         const ModuleTranslation &moduleTranslation) {
  if (!denseElementsAttr)
    return nullptr;

  toolchain::Type *innermostLLVMType = getInnermostElementType(llvmType);
  if (!toolchain::ConstantDataSequential::isElementTypeCompatible(innermostLLVMType))
    return nullptr;

  ShapedType type = denseElementsAttr.getType();
  if (type.getNumElements() == 0)
    return nullptr;

  // Check that the raw data size matches what is expected for the scalar size.
  // TODO: in theory, we could repack the data here to keep constructing from
  // raw data.
  // TODO: we may also need to consider endianness when cross-compiling to an
  // architecture where it is different.
  int64_t elementByteSize = denseElementsAttr.getRawData().size() /
                            denseElementsAttr.getNumElements();
  if (8 * elementByteSize != innermostLLVMType->getScalarSizeInBits())
    return nullptr;

  // Compute the shape of all dimensions but the innermost. Note that the
  // innermost dimension may be that of the vector element type.
  bool hasVectorElementType = isa<VectorType>(type.getElementType());
  int64_t numAggregates =
      denseElementsAttr.getNumElements() /
      (hasVectorElementType ? 1
                            : denseElementsAttr.getType().getShape().back());
  ArrayRef<int64_t> outerShape = type.getShape();
  if (!hasVectorElementType)
    outerShape = outerShape.drop_back();

  // Handle the case of vector splat, LLVM has special support for it.
  if (denseElementsAttr.isSplat() &&
      (isa<VectorType>(type) || hasVectorElementType)) {
    toolchain::Constant *splatValue = LLVM::detail::getLLVMConstant(
        innermostLLVMType, denseElementsAttr.getSplatValue<Attribute>(), loc,
        moduleTranslation);
    toolchain::Constant *splatVector =
        toolchain::ConstantDataVector::getSplat(0, splatValue);
    SmallVector<toolchain::Constant *> constants(numAggregates, splatVector);
    ArrayRef<toolchain::Constant *> constantsRef = constants;
    return buildSequentialConstant(constantsRef, outerShape, llvmType, loc);
  }
  if (denseElementsAttr.isSplat())
    return nullptr;

  // In case of non-splat, create a constructor for the innermost constant from
  // a piece of raw data.
  std::function<toolchain::Constant *(StringRef)> buildCstData;
  if (isa<TensorType>(type)) {
    auto vectorElementType = dyn_cast<VectorType>(type.getElementType());
    if (vectorElementType && vectorElementType.getRank() == 1) {
      buildCstData = [&](StringRef data) {
        return toolchain::ConstantDataVector::getRaw(
            data, vectorElementType.getShape().back(), innermostLLVMType);
      };
    } else if (!vectorElementType) {
      buildCstData = [&](StringRef data) {
        return toolchain::ConstantDataArray::getRaw(data, type.getShape().back(),
                                               innermostLLVMType);
      };
    }
  } else if (isa<VectorType>(type)) {
    buildCstData = [&](StringRef data) {
      return toolchain::ConstantDataVector::getRaw(data, type.getShape().back(),
                                              innermostLLVMType);
    };
  }
  if (!buildCstData)
    return nullptr;

  // Create innermost constants and defer to the default constant creation
  // mechanism for other dimensions.
  SmallVector<toolchain::Constant *> constants;
  int64_t aggregateSize = denseElementsAttr.getType().getShape().back() *
                          (innermostLLVMType->getScalarSizeInBits() / 8);
  constants.reserve(numAggregates);
  for (unsigned i = 0; i < numAggregates; ++i) {
    StringRef data(denseElementsAttr.getRawData().data() + i * aggregateSize,
                   aggregateSize);
    constants.push_back(buildCstData(data));
  }

  ArrayRef<toolchain::Constant *> constantsRef = constants;
  return buildSequentialConstant(constantsRef, outerShape, llvmType, loc);
}

/// Convert a dense resource elements attribute to an LLVM IR constant using its
/// raw data storage if possible. This supports elements attributes of tensor or
/// vector type and avoids constructing separate objects for individual values
/// of the innermost dimension. Constants for other dimensions are still
/// constructed recursively. Returns nullptr on failure and emits errors at
/// `loc`.
static toolchain::Constant *convertDenseResourceElementsAttr(
    Location loc, DenseResourceElementsAttr denseResourceAttr,
    toolchain::Type *llvmType, const ModuleTranslation &moduleTranslation) {
  assert(denseResourceAttr && "expected non-null attribute");

  toolchain::Type *innermostLLVMType = getInnermostElementType(llvmType);
  if (!toolchain::ConstantDataSequential::isElementTypeCompatible(
          innermostLLVMType)) {
    emitError(loc, "no known conversion for innermost element type");
    return nullptr;
  }

  ShapedType type = denseResourceAttr.getType();
  assert(type.getNumElements() > 0 && "Expected non-empty elements attribute");

  AsmResourceBlob *blob = denseResourceAttr.getRawHandle().getBlob();
  if (!blob) {
    emitError(loc, "resource does not exist");
    return nullptr;
  }

  ArrayRef<char> rawData = blob->getData();

  // Check that the raw data size matches what is expected for the scalar size.
  // TODO: in theory, we could repack the data here to keep constructing from
  // raw data.
  // TODO: we may also need to consider endianness when cross-compiling to an
  // architecture where it is different.
  int64_t numElements = denseResourceAttr.getType().getNumElements();
  int64_t elementByteSize = rawData.size() / numElements;
  if (8 * elementByteSize != innermostLLVMType->getScalarSizeInBits()) {
    emitError(loc, "raw data size does not match element type size");
    return nullptr;
  }

  // Compute the shape of all dimensions but the innermost. Note that the
  // innermost dimension may be that of the vector element type.
  bool hasVectorElementType = isa<VectorType>(type.getElementType());
  int64_t numAggregates =
      numElements / (hasVectorElementType
                         ? 1
                         : denseResourceAttr.getType().getShape().back());
  ArrayRef<int64_t> outerShape = type.getShape();
  if (!hasVectorElementType)
    outerShape = outerShape.drop_back();

  // Create a constructor for the innermost constant from a piece of raw data.
  std::function<toolchain::Constant *(StringRef)> buildCstData;
  if (isa<TensorType>(type)) {
    auto vectorElementType = dyn_cast<VectorType>(type.getElementType());
    if (vectorElementType && vectorElementType.getRank() == 1) {
      buildCstData = [&](StringRef data) {
        return toolchain::ConstantDataVector::getRaw(
            data, vectorElementType.getShape().back(), innermostLLVMType);
      };
    } else if (!vectorElementType) {
      buildCstData = [&](StringRef data) {
        return toolchain::ConstantDataArray::getRaw(data, type.getShape().back(),
                                               innermostLLVMType);
      };
    }
  } else if (isa<VectorType>(type)) {
    buildCstData = [&](StringRef data) {
      return toolchain::ConstantDataVector::getRaw(data, type.getShape().back(),
                                              innermostLLVMType);
    };
  }
  if (!buildCstData) {
    emitError(loc, "unsupported dense_resource type");
    return nullptr;
  }

  // Create innermost constants and defer to the default constant creation
  // mechanism for other dimensions.
  SmallVector<toolchain::Constant *> constants;
  int64_t aggregateSize = denseResourceAttr.getType().getShape().back() *
                          (innermostLLVMType->getScalarSizeInBits() / 8);
  constants.reserve(numAggregates);
  for (unsigned i = 0; i < numAggregates; ++i) {
    StringRef data(rawData.data() + i * aggregateSize, aggregateSize);
    constants.push_back(buildCstData(data));
  }

  ArrayRef<toolchain::Constant *> constantsRef = constants;
  return buildSequentialConstant(constantsRef, outerShape, llvmType, loc);
}

/// Create an LLVM IR constant of `llvmType` from the MLIR attribute `attr`.
/// This currently supports integer, floating point, splat and dense element
/// attributes and combinations thereof. Also, an array attribute with two
/// elements is supported to represent a complex constant.  In case of error,
/// report it to `loc` and return nullptr.
toolchain::Constant *mlir::LLVM::detail::getLLVMConstant(
    toolchain::Type *llvmType, Attribute attr, Location loc,
    const ModuleTranslation &moduleTranslation) {
  if (!attr || isa<UndefAttr>(attr))
    return toolchain::UndefValue::get(llvmType);
  if (isa<ZeroAttr>(attr))
    return toolchain::Constant::getNullValue(llvmType);
  if (auto *structType = dyn_cast<::toolchain::StructType>(llvmType)) {
    auto arrayAttr = dyn_cast<ArrayAttr>(attr);
    if (!arrayAttr) {
      emitError(loc, "expected an array attribute for a struct constant");
      return nullptr;
    }
    SmallVector<toolchain::Constant *> structElements;
    structElements.reserve(structType->getNumElements());
    for (auto [elemType, elemAttr] :
         zip_equal(structType->elements(), arrayAttr)) {
      toolchain::Constant *element =
          getLLVMConstant(elemType, elemAttr, loc, moduleTranslation);
      if (!element)
        return nullptr;
      structElements.push_back(element);
    }
    return toolchain::ConstantStruct::get(structType, structElements);
  }
  // For integer types, we allow a mismatch in sizes as the index type in
  // MLIR might have a different size than the index type in the LLVM module.
  if (auto intAttr = dyn_cast<IntegerAttr>(attr)) {
    // If the attribute is an unsigned integer or a 1-bit integer, zero-extend
    // the value to the bit width of the LLVM type. Otherwise, sign-extend.
    auto intTy = dyn_cast<IntegerType>(intAttr.getType());
    APInt value;
    if (intTy && (intTy.isUnsigned() || intTy.getWidth() == 1))
      value = intAttr.getValue().zextOrTrunc(llvmType->getIntegerBitWidth());
    else
      value = intAttr.getValue().sextOrTrunc(llvmType->getIntegerBitWidth());
    return toolchain::ConstantInt::get(llvmType, value);
  }
  if (auto floatAttr = dyn_cast<FloatAttr>(attr)) {
    const toolchain::fltSemantics &sem = floatAttr.getValue().getSemantics();
    // Special case for 8-bit floats, which are represented by integers due to
    // the lack of native fp8 types in LLVM at the moment. Additionally, handle
    // targets (like AMDGPU) that don't implement bfloat and convert all bfloats
    // to i16.
    unsigned floatWidth = APFloat::getSizeInBits(sem);
    if (llvmType->isIntegerTy(floatWidth))
      return toolchain::ConstantInt::get(llvmType,
                                    floatAttr.getValue().bitcastToAPInt());
    if (llvmType !=
        toolchain::Type::getFloatingPointTy(llvmType->getContext(),
                                       floatAttr.getValue().getSemantics())) {
      emitError(loc, "FloatAttr does not match expected type of the constant");
      return nullptr;
    }
    return toolchain::ConstantFP::get(llvmType, floatAttr.getValue());
  }
  if (auto funcAttr = dyn_cast<FlatSymbolRefAttr>(attr))
    return toolchain::ConstantExpr::getBitCast(
        moduleTranslation.lookupFunction(funcAttr.getValue()), llvmType);
  if (auto splatAttr = dyn_cast<SplatElementsAttr>(attr)) {
    toolchain::Type *elementType;
    uint64_t numElements;
    bool isScalable = false;
    if (auto *arrayTy = dyn_cast<toolchain::ArrayType>(llvmType)) {
      elementType = arrayTy->getElementType();
      numElements = arrayTy->getNumElements();
    } else if (auto *fVectorTy = dyn_cast<toolchain::FixedVectorType>(llvmType)) {
      elementType = fVectorTy->getElementType();
      numElements = fVectorTy->getNumElements();
    } else if (auto *sVectorTy = dyn_cast<toolchain::ScalableVectorType>(llvmType)) {
      elementType = sVectorTy->getElementType();
      numElements = sVectorTy->getMinNumElements();
      isScalable = true;
    } else {
      llvm_unreachable("unrecognized constant vector type");
    }
    // Splat value is a scalar. Extract it only if the element type is not
    // another sequence type. The recursion terminates because each step removes
    // one outer sequential type.
    bool elementTypeSequential =
        isa<toolchain::ArrayType, toolchain::VectorType>(elementType);
    toolchain::Constant *child = getLLVMConstant(
        elementType,
        elementTypeSequential ? splatAttr
                              : splatAttr.getSplatValue<Attribute>(),
        loc, moduleTranslation);
    if (!child)
      return nullptr;
    if (llvmType->isVectorTy())
      return toolchain::ConstantVector::getSplat(
          toolchain::ElementCount::get(numElements, /*Scalable=*/isScalable), child);
    if (llvmType->isArrayTy()) {
      auto *arrayType = toolchain::ArrayType::get(elementType, numElements);
      if (child->isZeroValue() && !elementType->isFPOrFPVectorTy()) {
        return toolchain::ConstantAggregateZero::get(arrayType);
      }
      if (toolchain::ConstantDataSequential::isElementTypeCompatible(elementType)) {
        // TODO: Handle all compatible types. This code only handles integer.
        if (isa<toolchain::IntegerType>(elementType)) {
          if (toolchain::ConstantInt *ci = dyn_cast<toolchain::ConstantInt>(child)) {
            if (ci->getBitWidth() == 8) {
              SmallVector<int8_t> constants(numElements, ci->getZExtValue());
              return toolchain::ConstantDataArray::get(elementType->getContext(),
                                                  constants);
            }
            if (ci->getBitWidth() == 16) {
              SmallVector<int16_t> constants(numElements, ci->getZExtValue());
              return toolchain::ConstantDataArray::get(elementType->getContext(),
                                                  constants);
            }
            if (ci->getBitWidth() == 32) {
              SmallVector<int32_t> constants(numElements, ci->getZExtValue());
              return toolchain::ConstantDataArray::get(elementType->getContext(),
                                                  constants);
            }
            if (ci->getBitWidth() == 64) {
              SmallVector<int64_t> constants(numElements, ci->getZExtValue());
              return toolchain::ConstantDataArray::get(elementType->getContext(),
                                                  constants);
            }
          }
        }
      }
      // std::vector is used here to accomodate large number of elements that
      // exceed SmallVector capacity.
      std::vector<toolchain::Constant *> constants(numElements, child);
      return toolchain::ConstantArray::get(arrayType, constants);
    }
  }

  // Try using raw elements data if possible.
  if (toolchain::Constant *result =
          convertDenseElementsAttr(loc, dyn_cast<DenseElementsAttr>(attr),
                                   llvmType, moduleTranslation)) {
    return result;
  }

  if (auto denseResourceAttr = dyn_cast<DenseResourceElementsAttr>(attr)) {
    return convertDenseResourceElementsAttr(loc, denseResourceAttr, llvmType,
                                            moduleTranslation);
  }

  // Fall back to element-by-element construction otherwise.
  if (auto elementsAttr = dyn_cast<ElementsAttr>(attr)) {
    assert(elementsAttr.getShapedType().hasStaticShape());
    assert(!elementsAttr.getShapedType().getShape().empty() &&
           "unexpected empty elements attribute shape");

    SmallVector<toolchain::Constant *, 8> constants;
    constants.reserve(elementsAttr.getNumElements());
    toolchain::Type *innermostType = getInnermostElementType(llvmType);
    for (auto n : elementsAttr.getValues<Attribute>()) {
      constants.push_back(
          getLLVMConstant(innermostType, n, loc, moduleTranslation));
      if (!constants.back())
        return nullptr;
    }
    ArrayRef<toolchain::Constant *> constantsRef = constants;
    toolchain::Constant *result = buildSequentialConstant(
        constantsRef, elementsAttr.getShapedType().getShape(), llvmType, loc);
    assert(constantsRef.empty() && "did not consume all elemental constants");
    return result;
  }

  if (auto stringAttr = dyn_cast<StringAttr>(attr)) {
    return toolchain::ConstantDataArray::get(moduleTranslation.getLLVMContext(),
                                        ArrayRef<char>{stringAttr.getValue()});
  }

  // Handle arrays of structs that cannot be represented as DenseElementsAttr
  // in MLIR.
  if (auto arrayAttr = dyn_cast<ArrayAttr>(attr)) {
    if (auto *arrayTy = dyn_cast<toolchain::ArrayType>(llvmType)) {
      toolchain::Type *elementType = arrayTy->getElementType();
      Attribute previousElementAttr;
      toolchain::Constant *elementCst = nullptr;
      SmallVector<toolchain::Constant *> constants;
      constants.reserve(arrayTy->getNumElements());
      for (Attribute elementAttr : arrayAttr) {
        // Arrays with a single value or with repeating values are quite common.
        // Short-circuit the translation when the element value is the same as
        // the previous one.
        if (!previousElementAttr || previousElementAttr != elementAttr) {
          previousElementAttr = elementAttr;
          elementCst =
              getLLVMConstant(elementType, elementAttr, loc, moduleTranslation);
          if (!elementCst)
            return nullptr;
        }
        constants.push_back(elementCst);
      }
      return toolchain::ConstantArray::get(arrayTy, constants);
    }
  }

  emitError(loc, "unsupported constant value");
  return nullptr;
}

ModuleTranslation::ModuleTranslation(Operation *module,
                                     std::unique_ptr<toolchain::Module> llvmModule)
    : mlirModule(module), llvmModule(std::move(llvmModule)),
      debugTranslation(
          std::make_unique<DebugTranslation>(module, *this->llvmModule)),
      loopAnnotationTranslation(std::make_unique<LoopAnnotationTranslation>(
          *this, *this->llvmModule)),
      typeTranslator(this->llvmModule->getContext()),
      iface(module->getContext()) {
  assert(satisfiesLLVMModule(mlirModule) &&
         "mlirModule should honor LLVM's module semantics.");
}

ModuleTranslation::~ModuleTranslation() {
  if (ompBuilder && !ompBuilder->isFinalized())
    ompBuilder->finalize();
}

void ModuleTranslation::forgetMapping(Region &region) {
  SmallVector<Region *> toProcess;
  toProcess.push_back(&region);
  while (!toProcess.empty()) {
    Region *current = toProcess.pop_back_val();
    for (Block &block : *current) {
      blockMapping.erase(&block);
      for (Value arg : block.getArguments())
        valueMapping.erase(arg);
      for (Operation &op : block) {
        for (Value value : op.getResults())
          valueMapping.erase(value);
        if (op.hasSuccessors())
          branchMapping.erase(&op);
        if (isa<LLVM::GlobalOp>(op))
          globalsMapping.erase(&op);
        if (isa<LLVM::AliasOp>(op))
          aliasesMapping.erase(&op);
        if (isa<LLVM::IFuncOp>(op))
          ifuncMapping.erase(&op);
        if (isa<LLVM::CallOp>(op))
          callMapping.erase(&op);
        toolchain::append_range(
            toProcess,
            toolchain::map_range(op.getRegions(), [](Region &r) { return &r; }));
      }
    }
  }
}

/// Get the SSA value passed to the current block from the terminator operation
/// of its predecessor.
static Value getPHISourceValue(Block *current, Block *pred,
                               unsigned numArguments, unsigned index) {
  Operation &terminator = *pred->getTerminator();
  if (isa<LLVM::BrOp>(terminator))
    return terminator.getOperand(index);

#ifndef NDEBUG
  toolchain::SmallPtrSet<Block *, 4> seenSuccessors;
  for (unsigned i = 0, e = terminator.getNumSuccessors(); i < e; ++i) {
    Block *successor = terminator.getSuccessor(i);
    auto branch = cast<BranchOpInterface>(terminator);
    SuccessorOperands successorOperands = branch.getSuccessorOperands(i);
    assert(
        (!seenSuccessors.contains(successor) || successorOperands.empty()) &&
        "successors with arguments in LLVM branches must be different blocks");
    seenSuccessors.insert(successor);
  }
#endif

  // For instructions that branch based on a condition value, we need to take
  // the operands for the branch that was taken.
  if (auto condBranchOp = dyn_cast<LLVM::CondBrOp>(terminator)) {
    // For conditional branches, we take the operands from either the "true" or
    // the "false" branch.
    return condBranchOp.getSuccessor(0) == current
               ? condBranchOp.getTrueDestOperands()[index]
               : condBranchOp.getFalseDestOperands()[index];
  }

  if (auto switchOp = dyn_cast<LLVM::SwitchOp>(terminator)) {
    // For switches, we take the operands from either the default case, or from
    // the case branch that was taken.
    if (switchOp.getDefaultDestination() == current)
      return switchOp.getDefaultOperands()[index];
    for (const auto &i : toolchain::enumerate(switchOp.getCaseDestinations()))
      if (i.value() == current)
        return switchOp.getCaseOperands(i.index())[index];
  }

  if (auto indBrOp = dyn_cast<LLVM::IndirectBrOp>(terminator)) {
    // For indirect branches we take operands for each successor.
    for (const auto &i : toolchain::enumerate(indBrOp->getSuccessors())) {
      if (indBrOp->getSuccessor(i.index()) == current)
        return indBrOp.getSuccessorOperands(i.index())[index];
    }
  }

  if (auto invokeOp = dyn_cast<LLVM::InvokeOp>(terminator)) {
    return invokeOp.getNormalDest() == current
               ? invokeOp.getNormalDestOperands()[index]
               : invokeOp.getUnwindDestOperands()[index];
  }

  llvm_unreachable(
      "only branch, switch or invoke operations can be terminators "
      "of a block that has successors");
}

/// Connect the PHI nodes to the results of preceding blocks.
void mlir::LLVM::detail::connectPHINodes(Region &region,
                                         const ModuleTranslation &state) {
  // Skip the first block, it cannot be branched to and its arguments correspond
  // to the arguments of the LLVM function.
  for (Block &bb : toolchain::drop_begin(region)) {
    toolchain::BasicBlock *llvmBB = state.lookupBlock(&bb);
    auto phis = llvmBB->phis();
    auto numArguments = bb.getNumArguments();
    assert(numArguments == std::distance(phis.begin(), phis.end()));
    for (auto [index, phiNode] : toolchain::enumerate(phis)) {
      for (auto *pred : bb.getPredecessors()) {
        // Find the LLVM IR block that contains the converted terminator
        // instruction and use it in the PHI node. Note that this block is not
        // necessarily the same as state.lookupBlock(pred), some operations
        // (in particular, OpenMP operations using OpenMPIRBuilder) may have
        // split the blocks.
        toolchain::Instruction *terminator =
            state.lookupBranch(pred->getTerminator());
        assert(terminator && "missing the mapping for a terminator");
        phiNode.addIncoming(state.lookupValue(getPHISourceValue(
                                &bb, pred, numArguments, index)),
                            terminator->getParent());
      }
    }
  }
}

toolchain::CallInst *mlir::LLVM::detail::createIntrinsicCall(
    toolchain::IRBuilderBase &builder, toolchain::Intrinsic::ID intrinsic,
    ArrayRef<toolchain::Value *> args, ArrayRef<toolchain::Type *> tys) {
  return builder.CreateIntrinsic(intrinsic, tys, args);
}

toolchain::CallInst *mlir::LLVM::detail::createIntrinsicCall(
    toolchain::IRBuilderBase &builder, toolchain::Intrinsic::ID intrinsic,
    toolchain::Type *retTy, ArrayRef<toolchain::Value *> args) {
  return builder.CreateIntrinsic(retTy, intrinsic, args);
}

toolchain::CallInst *mlir::LLVM::detail::createIntrinsicCall(
    toolchain::IRBuilderBase &builder, ModuleTranslation &moduleTranslation,
    Operation *intrOp, toolchain::Intrinsic::ID intrinsic, unsigned numResults,
    ArrayRef<unsigned> overloadedResults, ArrayRef<unsigned> overloadedOperands,
    ArrayRef<unsigned> immArgPositions,
    ArrayRef<StringLiteral> immArgAttrNames) {
  assert(immArgPositions.size() == immArgAttrNames.size() &&
         "LLVM `immArgPositions` and MLIR `immArgAttrNames` should have equal "
         "length");

  SmallVector<toolchain::OperandBundleDef> opBundles;
  size_t numOpBundleOperands = 0;
  auto opBundleSizesAttr = cast_if_present<DenseI32ArrayAttr>(
      intrOp->getAttr(LLVMDialect::getOpBundleSizesAttrName()));
  auto opBundleTagsAttr = cast_if_present<ArrayAttr>(
      intrOp->getAttr(LLVMDialect::getOpBundleTagsAttrName()));

  if (opBundleSizesAttr && opBundleTagsAttr) {
    ArrayRef<int> opBundleSizes = opBundleSizesAttr.asArrayRef();
    assert(opBundleSizes.size() == opBundleTagsAttr.size() &&
           "operand bundles and tags do not match");

    numOpBundleOperands = toolchain::sum_of(opBundleSizes);
    assert(numOpBundleOperands <= intrOp->getNumOperands() &&
           "operand bundle operands is more than the number of operands");

    ValueRange operands = intrOp->getOperands().take_back(numOpBundleOperands);
    size_t nextOperandIdx = 0;
    opBundles.reserve(opBundleSizesAttr.size());

    for (auto [opBundleTagAttr, bundleSize] :
         toolchain::zip(opBundleTagsAttr, opBundleSizes)) {
      auto bundleTag = cast<StringAttr>(opBundleTagAttr).str();
      auto bundleOperands = moduleTranslation.lookupValues(
          operands.slice(nextOperandIdx, bundleSize));
      opBundles.emplace_back(std::move(bundleTag), std::move(bundleOperands));
      nextOperandIdx += bundleSize;
    }
  }

  // Map operands and attributes to LLVM values.
  auto opOperands = intrOp->getOperands().drop_back(numOpBundleOperands);
  auto operands = moduleTranslation.lookupValues(opOperands);
  SmallVector<toolchain::Value *> args(immArgPositions.size() + operands.size());
  for (auto [immArgPos, immArgName] :
       toolchain::zip(immArgPositions, immArgAttrNames)) {
    auto attr = toolchain::cast<TypedAttr>(intrOp->getAttr(immArgName));
    assert(attr.getType().isIntOrFloat() && "expected int or float immarg");
    auto *type = moduleTranslation.convertType(attr.getType());
    args[immArgPos] = LLVM::detail::getLLVMConstant(
        type, attr, intrOp->getLoc(), moduleTranslation);
  }
  unsigned opArg = 0;
  for (auto &arg : args) {
    if (!arg)
      arg = operands[opArg++];
  }

  // Resolve overloaded intrinsic declaration.
  SmallVector<toolchain::Type *> overloadedTypes;
  for (unsigned overloadedResultIdx : overloadedResults) {
    if (numResults > 1) {
      // More than one result is mapped to an LLVM struct.
      overloadedTypes.push_back(moduleTranslation.convertType(
          toolchain::cast<LLVM::LLVMStructType>(intrOp->getResult(0).getType())
              .getBody()[overloadedResultIdx]));
    } else {
      overloadedTypes.push_back(
          moduleTranslation.convertType(intrOp->getResult(0).getType()));
    }
  }
  for (unsigned overloadedOperandIdx : overloadedOperands)
    overloadedTypes.push_back(args[overloadedOperandIdx]->getType());
  toolchain::Module *module = builder.GetInsertBlock()->getModule();
  toolchain::Function *llvmIntr = toolchain::Intrinsic::getOrInsertDeclaration(
      module, intrinsic, overloadedTypes);

  return builder.CreateCall(llvmIntr, args, opBundles);
}

/// Given a single MLIR operation, create the corresponding LLVM IR operation
/// using the `builder`.
LogicalResult ModuleTranslation::convertOperation(Operation &op,
                                                  toolchain::IRBuilderBase &builder,
                                                  bool recordInsertions) {
  const LLVMTranslationDialectInterface *opIface = iface.getInterfaceFor(&op);
  if (!opIface)
    return op.emitError("cannot be converted to LLVM IR: missing "
                        "`LLVMTranslationDialectInterface` registration for "
                        "dialect for op: ")
           << op.getName();

  InstructionCapturingInserter::CollectionScope scope(builder,
                                                      recordInsertions);
  if (failed(opIface->convertOperation(&op, builder, *this)))
    return op.emitError("LLVM Translation failed for operation: ")
           << op.getName();

  return convertDialectAttributes(&op, scope.getCapturedInstructions());
}

/// Convert block to LLVM IR.  Unless `ignoreArguments` is set, emit PHI nodes
/// to define values corresponding to the MLIR block arguments.  These nodes
/// are not connected to the source basic blocks, which may not exist yet.  Uses
/// `builder` to construct the LLVM IR. Expects the LLVM IR basic block to have
/// been created for `bb` and included in the block mapping.  Inserts new
/// instructions at the end of the block and leaves `builder` in a state
/// suitable for further insertion into the end of the block.
LogicalResult ModuleTranslation::convertBlockImpl(Block &bb,
                                                  bool ignoreArguments,
                                                  toolchain::IRBuilderBase &builder,
                                                  bool recordInsertions) {
  builder.SetInsertPoint(lookupBlock(&bb));
  auto *subprogram = builder.GetInsertBlock()->getParent()->getSubprogram();

  // Before traversing operations, make block arguments available through
  // value remapping and PHI nodes, but do not add incoming edges for the PHI
  // nodes just yet: those values may be defined by this or following blocks.
  // This step is omitted if "ignoreArguments" is set.  The arguments of the
  // first block have been already made available through the remapping of
  // LLVM function arguments.
  if (!ignoreArguments) {
    auto predecessors = bb.getPredecessors();
    unsigned numPredecessors =
        std::distance(predecessors.begin(), predecessors.end());
    for (auto arg : bb.getArguments()) {
      auto wrappedType = arg.getType();
      if (!isCompatibleType(wrappedType))
        return emitError(bb.front().getLoc(),
                         "block argument does not have an LLVM type");
      builder.SetCurrentDebugLocation(
          debugTranslation->translateLoc(arg.getLoc(), subprogram));
      toolchain::Type *type = convertType(wrappedType);
      toolchain::PHINode *phi = builder.CreatePHI(type, numPredecessors);
      mapValue(arg, phi);
    }
  }

  // Traverse operations.
  for (auto &op : bb) {
    // Set the current debug location within the builder.
    builder.SetCurrentDebugLocation(
        debugTranslation->translateLoc(op.getLoc(), subprogram));

    if (failed(convertOperation(op, builder, recordInsertions)))
      return failure();

    // Set the branch weight metadata on the translated instruction.
    if (auto iface = dyn_cast<WeightedBranchOpInterface>(op))
      setBranchWeightsMetadata(iface);
  }

  return success();
}

/// A helper method to get the single Block in an operation honoring LLVM's
/// module requirements.
static Block &getModuleBody(Operation *module) {
  return module->getRegion(0).front();
}

/// A helper method to decide if a constant must not be set as a global variable
/// initializer. For an external linkage variable, the variable with an
/// initializer is considered externally visible and defined in this module, the
/// variable without an initializer is externally available and is defined
/// elsewhere.
static bool shouldDropGlobalInitializer(toolchain::GlobalValue::LinkageTypes linkage,
                                        toolchain::Constant *cst) {
  return (linkage == toolchain::GlobalVariable::ExternalLinkage && !cst) ||
         linkage == toolchain::GlobalVariable::ExternalWeakLinkage;
}

/// Sets the runtime preemption specifier of `gv` to dso_local if
/// `dsoLocalRequested` is true, otherwise it is left unchanged.
static void addRuntimePreemptionSpecifier(bool dsoLocalRequested,
                                          toolchain::GlobalValue *gv) {
  if (dsoLocalRequested)
    gv->setDSOLocal(true);
}

/// Attempts to translate an MLIR attribute identified by `key`, optionally with
/// the given `value`, into an LLVM IR attribute. Reports errors at `loc` if
/// any. If the attribute name corresponds to a known LLVM IR attribute kind,
/// creates the LLVM attribute of that kind; otherwise, keeps it as a string
/// attribute. Performs additional checks for attributes known to have or not
/// have a value in order to avoid assertions inside LLVM upon construction.
static FailureOr<toolchain::Attribute>
convertMLIRAttributeToLLVM(Location loc, toolchain::LLVMContext &ctx, StringRef key,
                           StringRef value = StringRef()) {
  auto kind = toolchain::Attribute::getAttrKindFromName(key);
  if (kind == toolchain::Attribute::None)
    return toolchain::Attribute::get(ctx, key, value);

  if (toolchain::Attribute::isIntAttrKind(kind)) {
    if (value.empty())
      return emitError(loc) << "LLVM attribute '" << key << "' expects a value";

    int64_t result;
    if (!value.getAsInteger(/*Radix=*/0, result))
      return toolchain::Attribute::get(ctx, kind, result);
    return toolchain::Attribute::get(ctx, key, value);
  }

  if (!value.empty())
    return emitError(loc) << "LLVM attribute '" << key
                          << "' does not expect a value, found '" << value
                          << "'";

  return toolchain::Attribute::get(ctx, kind);
}

/// Converts the MLIR attributes listed in the given array attribute into LLVM
/// attributes. Returns an `AttrBuilder` containing the converted attributes.
/// Reports error to `loc` if any and returns immediately. Expects `arrayAttr`
/// to contain either string attributes, treated as value-less LLVM attributes,
/// or array attributes containing two string attributes, with the first string
/// being the name of the corresponding LLVM attribute and the second string
/// beings its value. Note that even integer attributes are expected to have
/// their values expressed as strings.
static FailureOr<toolchain::AttrBuilder>
convertMLIRAttributesToLLVM(Location loc, toolchain::LLVMContext &ctx,
                            ArrayAttr arrayAttr, StringRef arrayAttrName) {
  toolchain::AttrBuilder attrBuilder(ctx);
  if (!arrayAttr)
    return attrBuilder;

  for (Attribute attr : arrayAttr) {
    if (auto stringAttr = dyn_cast<StringAttr>(attr)) {
      FailureOr<toolchain::Attribute> llvmAttr =
          convertMLIRAttributeToLLVM(loc, ctx, stringAttr.getValue());
      if (failed(llvmAttr))
        return failure();
      attrBuilder.addAttribute(*llvmAttr);
      continue;
    }

    auto arrayAttr = dyn_cast<ArrayAttr>(attr);
    if (!arrayAttr || arrayAttr.size() != 2)
      return emitError(loc) << "expected '" << arrayAttrName
                            << "' to contain string or array attributes";

    auto keyAttr = dyn_cast<StringAttr>(arrayAttr[0]);
    auto valueAttr = dyn_cast<StringAttr>(arrayAttr[1]);
    if (!keyAttr || !valueAttr)
      return emitError(loc) << "expected arrays within '" << arrayAttrName
                            << "' to contain two strings";

    FailureOr<toolchain::Attribute> llvmAttr = convertMLIRAttributeToLLVM(
        loc, ctx, keyAttr.getValue(), valueAttr.getValue());
    if (failed(llvmAttr))
      return failure();
    attrBuilder.addAttribute(*llvmAttr);
  }

  return attrBuilder;
}

LogicalResult ModuleTranslation::convertGlobalsAndAliases() {
  // Mapping from compile unit to its respective set of global variables.
  DenseMap<toolchain::DICompileUnit *, SmallVector<toolchain::Metadata *>> allGVars;

  // First, create all global variables and global aliases in LLVM IR. A global
  // or alias body may refer to another global/alias or itself, so all the
  // mapping needs to happen prior to body conversion.

  // Create all toolchain::GlobalVariable
  for (auto op : getModuleBody(mlirModule).getOps<LLVM::GlobalOp>()) {
    toolchain::Type *type = convertType(op.getType());
    toolchain::Constant *cst = nullptr;
    if (op.getValueOrNull()) {
      // String attributes are treated separately because they cannot appear as
      // in-function constants and are thus not supported by getLLVMConstant.
      if (auto strAttr = dyn_cast_or_null<StringAttr>(op.getValueOrNull())) {
        cst = toolchain::ConstantDataArray::getString(
            llvmModule->getContext(), strAttr.getValue(), /*AddNull=*/false);
        type = cst->getType();
      } else if (!(cst = getLLVMConstant(type, op.getValueOrNull(), op.getLoc(),
                                         *this))) {
        return failure();
      }
    }

    auto linkage = convertLinkageToLLVM(op.getLinkage());

    // LLVM IR requires constant with linkage other than external or weak
    // external to have initializers. If MLIR does not provide an initializer,
    // default to undef.
    bool dropInitializer = shouldDropGlobalInitializer(linkage, cst);
    if (!dropInitializer && !cst)
      cst = toolchain::UndefValue::get(type);
    else if (dropInitializer && cst)
      cst = nullptr;

    auto *var = new toolchain::GlobalVariable(
        *llvmModule, type, op.getConstant(), linkage, cst, op.getSymName(),
        /*InsertBefore=*/nullptr,
        op.getThreadLocal_() ? toolchain::GlobalValue::GeneralDynamicTLSModel
                             : toolchain::GlobalValue::NotThreadLocal,
        op.getAddrSpace(), op.getExternallyInitialized());

    if (std::optional<mlir::SymbolRefAttr> comdat = op.getComdat()) {
      auto selectorOp = cast<ComdatSelectorOp>(
          SymbolTable::lookupNearestSymbolFrom(op, *comdat));
      var->setComdat(comdatMapping.lookup(selectorOp));
    }

    if (op.getUnnamedAddr().has_value())
      var->setUnnamedAddr(convertUnnamedAddrToLLVM(*op.getUnnamedAddr()));

    if (op.getSection().has_value())
      var->setSection(*op.getSection());

    addRuntimePreemptionSpecifier(op.getDsoLocal(), var);

    std::optional<uint64_t> alignment = op.getAlignment();
    if (alignment.has_value())
      var->setAlignment(toolchain::MaybeAlign(alignment.value()));

    var->setVisibility(convertVisibilityToLLVM(op.getVisibility_()));

    globalsMapping.try_emplace(op, var);

    // Add debug information if present.
    if (op.getDbgExprs()) {
      for (auto exprAttr :
           op.getDbgExprs()->getAsRange<DIGlobalVariableExpressionAttr>()) {
        toolchain::DIGlobalVariableExpression *diGlobalExpr =
            debugTranslation->translateGlobalVariableExpression(exprAttr);
        toolchain::DIGlobalVariable *diGlobalVar = diGlobalExpr->getVariable();
        var->addDebugInfo(diGlobalExpr);

        // There is no `globals` field in DICompileUnitAttr which can be
        // directly assigned to DICompileUnit. We have to build the list by
        // looking at the dbgExpr of all the GlobalOps. The scope of the
        // variable is used to get the DICompileUnit in which to add it. But
        // there are cases where the scope of a global does not directly point
        // to the DICompileUnit and we have to do a bit more work to get to
        // it. Some of those cases are:
        //
        // 1. For the languages that support modules, the scope hierarchy can
        // be variable -> DIModule -> DICompileUnit
        //
        // 2. For the Fortran common block variable, the scope hierarchy can
        // be variable -> DICommonBlock -> DISubprogram -> DICompileUnit
        //
        // 3. For entities like static local variables in C or variable with
        // SAVE attribute in Fortran, the scope hierarchy can be
        // variable -> DISubprogram -> DICompileUnit
        toolchain::DIScope *scope = diGlobalVar->getScope();
        if (auto *mod = dyn_cast_if_present<toolchain::DIModule>(scope))
          scope = mod->getScope();
        else if (auto *cb = dyn_cast_if_present<toolchain::DICommonBlock>(scope)) {
          if (auto *sp =
                  dyn_cast_if_present<toolchain::DISubprogram>(cb->getScope()))
            scope = sp->getUnit();
        } else if (auto *sp = dyn_cast_if_present<toolchain::DISubprogram>(scope))
          scope = sp->getUnit();

        // Get the compile unit (scope) of the the global variable.
        if (toolchain::DICompileUnit *compileUnit =
                dyn_cast_if_present<toolchain::DICompileUnit>(scope)) {
          // Update the compile unit with this incoming global variable
          // expression during the finalizing step later.
          allGVars[compileUnit].push_back(diGlobalExpr);
        }
      }
    }

    // Forward the target-specific attributes to LLVM.
    FailureOr<toolchain::AttrBuilder> convertedTargetSpecificAttrs =
        convertMLIRAttributesToLLVM(op.getLoc(), var->getContext(),
                                    op.getTargetSpecificAttrsAttr(),
                                    op.getTargetSpecificAttrsAttrName());
    if (failed(convertedTargetSpecificAttrs))
      return failure();
    var->addAttributes(*convertedTargetSpecificAttrs);
  }

  // Create all toolchain::GlobalAlias
  for (auto op : getModuleBody(mlirModule).getOps<LLVM::AliasOp>()) {
    toolchain::Type *type = convertType(op.getType());
    toolchain::Constant *cst = nullptr;
    toolchain::GlobalValue::LinkageTypes linkage =
        convertLinkageToLLVM(op.getLinkage());
    toolchain::Module &llvmMod = *llvmModule;

    // Note address space and aliasee info isn't set just yet.
    toolchain::GlobalAlias *var = toolchain::GlobalAlias::create(
        type, op.getAddrSpace(), linkage, op.getSymName(), /*placeholder*/ cst,
        &llvmMod);

    var->setThreadLocalMode(op.getThreadLocal_()
                                ? toolchain::GlobalAlias::GeneralDynamicTLSModel
                                : toolchain::GlobalAlias::NotThreadLocal);

    // Note there is no need to setup the comdat because GlobalAlias calls into
    // the aliasee comdat information automatically.

    if (op.getUnnamedAddr().has_value())
      var->setUnnamedAddr(convertUnnamedAddrToLLVM(*op.getUnnamedAddr()));

    var->setVisibility(convertVisibilityToLLVM(op.getVisibility_()));

    aliasesMapping.try_emplace(op, var);
  }

  // Convert global variable bodies.
  for (auto op : getModuleBody(mlirModule).getOps<LLVM::GlobalOp>()) {
    if (Block *initializer = op.getInitializerBlock()) {
      toolchain::IRBuilder<toolchain::TargetFolder> builder(
          llvmModule->getContext(),
          toolchain::TargetFolder(llvmModule->getDataLayout()));

      [[maybe_unused]] int numConstantsHit = 0;
      [[maybe_unused]] int numConstantsErased = 0;
      DenseMap<toolchain::ConstantAggregate *, int> constantAggregateUseMap;

      for (auto &op : initializer->without_terminator()) {
        if (failed(convertOperation(op, builder)))
          return emitError(op.getLoc(), "fail to convert global initializer");
        auto *cst = dyn_cast<toolchain::Constant>(lookupValue(op.getResult(0)));
        if (!cst)
          return emitError(op.getLoc(), "unemittable constant value");

        // When emitting an LLVM constant, a new constant is created and the old
        // constant may become dangling and take space. We should remove the
        // dangling constants to avoid memory explosion especially for constant
        // arrays whose number of elements is large.
        // Because multiple operations may refer to the same constant, we need
        // to count the number of uses of each constant array and remove it only
        // when the count becomes zero.
        if (auto *agg = dyn_cast<toolchain::ConstantAggregate>(cst)) {
          numConstantsHit++;
          Value result = op.getResult(0);
          int numUsers = std::distance(result.use_begin(), result.use_end());
          auto [iterator, inserted] =
              constantAggregateUseMap.try_emplace(agg, numUsers);
          if (!inserted) {
            // Key already exists, update the value
            iterator->second += numUsers;
          }
        }
        // Scan the operands of the operation to decrement the use count of
        // constants. Erase the constant if the use count becomes zero.
        for (Value v : op.getOperands()) {
          auto cst = dyn_cast<toolchain::ConstantAggregate>(lookupValue(v));
          if (!cst)
            continue;
          auto iter = constantAggregateUseMap.find(cst);
          assert(iter != constantAggregateUseMap.end() && "constant not found");
          iter->second--;
          if (iter->second == 0) {
            // NOTE: cannot call removeDeadConstantUsers() here because it
            // may remove the constant which has uses not be converted yet.
            if (cst->user_empty()) {
              cst->destroyConstant();
              numConstantsErased++;
            }
            constantAggregateUseMap.erase(iter);
          }
        }
      }

      ReturnOp ret = cast<ReturnOp>(initializer->getTerminator());
      toolchain::Constant *cst =
          cast<toolchain::Constant>(lookupValue(ret.getOperand(0)));
      auto *global = cast<toolchain::GlobalVariable>(lookupGlobal(op));
      if (!shouldDropGlobalInitializer(global->getLinkage(), cst))
        global->setInitializer(cst);

      // Try to remove the dangling constants again after all operations are
      // converted.
      for (auto it : constantAggregateUseMap) {
        auto cst = it.first;
        cst->removeDeadConstantUsers();
        if (cst->user_empty()) {
          cst->destroyConstant();
          numConstantsErased++;
        }
      }

      LLVM_DEBUG(toolchain::dbgs()
                     << "Convert initializer for " << op.getName() << "\n";
                 toolchain::dbgs() << numConstantsHit << " new constants hit\n";
                 toolchain::dbgs()
                 << numConstantsErased << " dangling constants erased\n";);
    }
  }

  // Convert toolchain.mlir.global_ctors and dtors.
  for (Operation &op : getModuleBody(mlirModule)) {
    auto ctorOp = dyn_cast<GlobalCtorsOp>(op);
    auto dtorOp = dyn_cast<GlobalDtorsOp>(op);
    if (!ctorOp && !dtorOp)
      continue;

    // The empty / zero initialized version of toolchain.global_(c|d)tors cannot be
    // handled by appendGlobalFn logic below, which just ignores empty (c|d)tor
    // lists. Make sure it gets emitted.
    if ((ctorOp && ctorOp.getCtors().empty()) ||
        (dtorOp && dtorOp.getDtors().empty())) {
      toolchain::IRBuilder<toolchain::TargetFolder> builder(
          llvmModule->getContext(),
          toolchain::TargetFolder(llvmModule->getDataLayout()));
      toolchain::Type *eltTy = toolchain::StructType::get(
          builder.getInt32Ty(), builder.getPtrTy(), builder.getPtrTy());
      toolchain::ArrayType *at = toolchain::ArrayType::get(eltTy, 0);
      toolchain::Constant *zeroInit = toolchain::Constant::getNullValue(at);
      (void)new toolchain::GlobalVariable(
          *llvmModule, zeroInit->getType(), false,
          toolchain::GlobalValue::AppendingLinkage, zeroInit,
          ctorOp ? "toolchain.global_ctors" : "toolchain.global_dtors");
    } else {
      auto range = ctorOp
                       ? toolchain::zip(ctorOp.getCtors(), ctorOp.getPriorities())
                       : toolchain::zip(dtorOp.getDtors(), dtorOp.getPriorities());
      auto appendGlobalFn =
          ctorOp ? toolchain::appendToGlobalCtors : toolchain::appendToGlobalDtors;
      for (const auto &[sym, prio] : range) {
        toolchain::Function *f =
            lookupFunction(cast<FlatSymbolRefAttr>(sym).getValue());
        appendGlobalFn(*llvmModule, f, cast<IntegerAttr>(prio).getInt(),
                       /*Data=*/nullptr);
      }
    }
  }

  for (auto op : getModuleBody(mlirModule).getOps<LLVM::GlobalOp>())
    if (failed(convertDialectAttributes(op, {})))
      return failure();

  // Finally, update the compile units their respective sets of global variables
  // created earlier.
  for (const auto &[compileUnit, globals] : allGVars) {
    compileUnit->replaceGlobalVariables(
        toolchain::MDTuple::get(getLLVMContext(), globals));
  }

  // Convert global alias bodies.
  for (auto op : getModuleBody(mlirModule).getOps<LLVM::AliasOp>()) {
    Block &initializer = op.getInitializerBlock();
    toolchain::IRBuilder<toolchain::TargetFolder> builder(
        llvmModule->getContext(),
        toolchain::TargetFolder(llvmModule->getDataLayout()));

    for (mlir::Operation &op : initializer.without_terminator()) {
      if (failed(convertOperation(op, builder)))
        return emitError(op.getLoc(), "fail to convert alias initializer");
      if (!isa<toolchain::Constant>(lookupValue(op.getResult(0))))
        return emitError(op.getLoc(), "unemittable constant value");
    }

    auto ret = cast<ReturnOp>(initializer.getTerminator());
    auto *cst = cast<toolchain::Constant>(lookupValue(ret.getOperand(0)));
    assert(aliasesMapping.count(op));
    auto *alias = cast<toolchain::GlobalAlias>(aliasesMapping[op]);
    alias->setAliasee(cst);
  }

  for (auto op : getModuleBody(mlirModule).getOps<LLVM::AliasOp>())
    if (failed(convertDialectAttributes(op, {})))
      return failure();

  return success();
}

/// Return a representation of `value` as metadata.
static toolchain::Metadata *convertIntegerToMetadata(toolchain::LLVMContext &context,
                                                const toolchain::APInt &value) {
  toolchain::Constant *constant = toolchain::ConstantInt::get(context, value);
  return toolchain::ConstantAsMetadata::get(constant);
}

/// Return a representation of `value` as an MDNode.
static toolchain::MDNode *convertIntegerToMDNode(toolchain::LLVMContext &context,
                                            const toolchain::APInt &value) {
  return toolchain::MDNode::get(context, convertIntegerToMetadata(context, value));
}

/// Return an MDNode encoding `vec_type_hint` metadata.
static toolchain::MDNode *convertVecTypeHintToMDNode(toolchain::LLVMContext &context,
                                                toolchain::Type *type,
                                                bool isSigned) {
  toolchain::Metadata *typeMD =
      toolchain::ConstantAsMetadata::get(toolchain::UndefValue::get(type));
  toolchain::Metadata *isSignedMD =
      convertIntegerToMetadata(context, toolchain::APInt(32, isSigned ? 1 : 0));
  return toolchain::MDNode::get(context, {typeMD, isSignedMD});
}

/// Return an MDNode with a tuple given by the values in `values`.
static toolchain::MDNode *convertIntegerArrayToMDNode(toolchain::LLVMContext &context,
                                                 ArrayRef<int32_t> values) {
  SmallVector<toolchain::Metadata *> mdValues;
  toolchain::transform(
      values, std::back_inserter(mdValues), [&context](int32_t value) {
        return convertIntegerToMetadata(context, toolchain::APInt(32, value));
      });
  return toolchain::MDNode::get(context, mdValues);
}

LogicalResult ModuleTranslation::convertOneFunction(LLVMFuncOp func) {
  // Clear the block, branch value mappings, they are only relevant within one
  // function.
  blockMapping.clear();
  valueMapping.clear();
  branchMapping.clear();
  toolchain::Function *llvmFunc = lookupFunction(func.getName());

  // Add function arguments to the value remapping table.
  for (auto [mlirArg, llvmArg] :
       toolchain::zip(func.getArguments(), llvmFunc->args()))
    mapValue(mlirArg, &llvmArg);

  // Check the personality and set it.
  if (func.getPersonality()) {
    toolchain::Type *ty = toolchain::PointerType::getUnqual(llvmFunc->getContext());
    if (toolchain::Constant *pfunc = getLLVMConstant(ty, func.getPersonalityAttr(),
                                                func.getLoc(), *this))
      llvmFunc->setPersonalityFn(pfunc);
  }

  if (std::optional<StringRef> section = func.getSection())
    llvmFunc->setSection(*section);

  if (func.getArmStreaming())
    llvmFunc->addFnAttr("aarch64_pstate_sm_enabled");
  else if (func.getArmLocallyStreaming())
    llvmFunc->addFnAttr("aarch64_pstate_sm_body");
  else if (func.getArmStreamingCompatible())
    llvmFunc->addFnAttr("aarch64_pstate_sm_compatible");

  if (func.getArmNewZa())
    llvmFunc->addFnAttr("aarch64_new_za");
  else if (func.getArmInZa())
    llvmFunc->addFnAttr("aarch64_in_za");
  else if (func.getArmOutZa())
    llvmFunc->addFnAttr("aarch64_out_za");
  else if (func.getArmInoutZa())
    llvmFunc->addFnAttr("aarch64_inout_za");
  else if (func.getArmPreservesZa())
    llvmFunc->addFnAttr("aarch64_preserves_za");

  if (auto targetCpu = func.getTargetCpu())
    llvmFunc->addFnAttr("target-cpu", *targetCpu);

  if (auto tuneCpu = func.getTuneCpu())
    llvmFunc->addFnAttr("tune-cpu", *tuneCpu);

  if (auto reciprocalEstimates = func.getReciprocalEstimates())
    llvmFunc->addFnAttr("reciprocal-estimates", *reciprocalEstimates);

  if (auto preferVectorWidth = func.getPreferVectorWidth())
    llvmFunc->addFnAttr("prefer-vector-width", *preferVectorWidth);

  if (auto attr = func.getVscaleRange())
    llvmFunc->addFnAttr(toolchain::Attribute::getWithVScaleRangeArgs(
        getLLVMContext(), attr->getMinRange().getInt(),
        attr->getMaxRange().getInt()));

  if (auto noInfsFpMath = func.getNoInfsFpMath())
    llvmFunc->addFnAttr("no-infs-fp-math", toolchain::toStringRef(*noInfsFpMath));

  if (auto noNansFpMath = func.getNoNansFpMath())
    llvmFunc->addFnAttr("no-nans-fp-math", toolchain::toStringRef(*noNansFpMath));

  if (auto noSignedZerosFpMath = func.getNoSignedZerosFpMath())
    llvmFunc->addFnAttr("no-signed-zeros-fp-math",
                        toolchain::toStringRef(*noSignedZerosFpMath));

  if (auto denormalFpMath = func.getDenormalFpMath())
    llvmFunc->addFnAttr("denormal-fp-math", *denormalFpMath);

  if (auto denormalFpMathF32 = func.getDenormalFpMathF32())
    llvmFunc->addFnAttr("denormal-fp-math-f32", *denormalFpMathF32);

  if (auto fpContract = func.getFpContract())
    llvmFunc->addFnAttr("fp-contract", *fpContract);

  if (auto instrumentFunctionEntry = func.getInstrumentFunctionEntry())
    llvmFunc->addFnAttr("instrument-function-entry", *instrumentFunctionEntry);

  if (auto instrumentFunctionExit = func.getInstrumentFunctionExit())
    llvmFunc->addFnAttr("instrument-function-exit", *instrumentFunctionExit);

  // First, create all blocks so we can jump to them.
  toolchain::LLVMContext &llvmContext = llvmFunc->getContext();
  for (auto &bb : func) {
    auto *llvmBB = toolchain::BasicBlock::Create(llvmContext);
    llvmBB->insertInto(llvmFunc);
    mapBlock(&bb, llvmBB);
  }

  // Then, convert blocks one by one in topological order to ensure defs are
  // converted before uses.
  auto blocks = getBlocksSortedByDominance(func.getBody());
  for (Block *bb : blocks) {
    CapturingIRBuilder builder(llvmContext,
                               toolchain::TargetFolder(llvmModule->getDataLayout()));
    if (failed(convertBlockImpl(*bb, bb->isEntryBlock(), builder,
                                /*recordInsertions=*/true)))
      return failure();
  }

  // After all blocks have been traversed and values mapped, connect the PHI
  // nodes to the results of preceding blocks.
  detail::connectPHINodes(func.getBody(), *this);

  // Finally, convert dialect attributes attached to the function.
  return convertDialectAttributes(func, {});
}

LogicalResult ModuleTranslation::convertDialectAttributes(
    Operation *op, ArrayRef<toolchain::Instruction *> instructions) {
  for (NamedAttribute attribute : op->getDialectAttrs())
    if (failed(iface.amendOperation(op, instructions, attribute, *this)))
      return failure();
  return success();
}

/// Converts memory effect attributes from `func` and attaches them to
/// `llvmFunc`.
static void convertFunctionMemoryAttributes(LLVMFuncOp func,
                                            toolchain::Function *llvmFunc) {
  if (!func.getMemoryEffects())
    return;

  MemoryEffectsAttr memEffects = func.getMemoryEffectsAttr();

  // Add memory effects incrementally.
  toolchain::MemoryEffects newMemEffects =
      toolchain::MemoryEffects(toolchain::MemoryEffects::Location::ArgMem,
                          convertModRefInfoToLLVM(memEffects.getArgMem()));
  newMemEffects |= toolchain::MemoryEffects(
      toolchain::MemoryEffects::Location::InaccessibleMem,
      convertModRefInfoToLLVM(memEffects.getInaccessibleMem()));
  newMemEffects |=
      toolchain::MemoryEffects(toolchain::MemoryEffects::Location::Other,
                          convertModRefInfoToLLVM(memEffects.getOther()));
  newMemEffects |=
      toolchain::MemoryEffects(toolchain::MemoryEffects::Location::ErrnoMem,
                          convertModRefInfoToLLVM(memEffects.getErrnoMem()));
  newMemEffects |=
      toolchain::MemoryEffects(toolchain::MemoryEffects::Location::TargetMem0,
                          convertModRefInfoToLLVM(memEffects.getTargetMem0()));
  newMemEffects |=
      toolchain::MemoryEffects(toolchain::MemoryEffects::Location::TargetMem1,
                          convertModRefInfoToLLVM(memEffects.getTargetMem1()));
  llvmFunc->setMemoryEffects(newMemEffects);
}

/// Converts function attributes from `func` and attaches them to `llvmFunc`.
static void convertFunctionAttributes(LLVMFuncOp func,
                                      toolchain::Function *llvmFunc) {
  if (func.getNoInlineAttr())
    llvmFunc->addFnAttr(toolchain::Attribute::NoInline);
  if (func.getAlwaysInlineAttr())
    llvmFunc->addFnAttr(toolchain::Attribute::AlwaysInline);
  if (func.getInlineHintAttr())
    llvmFunc->addFnAttr(toolchain::Attribute::InlineHint);
  if (func.getOptimizeNoneAttr())
    llvmFunc->addFnAttr(toolchain::Attribute::OptimizeNone);
  if (func.getConvergentAttr())
    llvmFunc->addFnAttr(toolchain::Attribute::Convergent);
  if (func.getNoUnwindAttr())
    llvmFunc->addFnAttr(toolchain::Attribute::NoUnwind);
  if (func.getWillReturnAttr())
    llvmFunc->addFnAttr(toolchain::Attribute::WillReturn);
  if (TargetFeaturesAttr targetFeatAttr = func.getTargetFeaturesAttr())
    llvmFunc->addFnAttr("target-features", targetFeatAttr.getFeaturesString());
  if (FramePointerKindAttr fpAttr = func.getFramePointerAttr())
    llvmFunc->addFnAttr("frame-pointer", stringifyFramePointerKind(
                                             fpAttr.getFramePointerKind()));
  if (UWTableKindAttr uwTableKindAttr = func.getUwtableKindAttr())
    llvmFunc->setUWTableKind(
        convertUWTableKindToLLVM(uwTableKindAttr.getUwtableKind()));
  convertFunctionMemoryAttributes(func, llvmFunc);
}

/// Converts function attributes from `func` and attaches them to `llvmFunc`.
static void convertFunctionKernelAttributes(LLVMFuncOp func,
                                            toolchain::Function *llvmFunc,
                                            ModuleTranslation &translation) {
  toolchain::LLVMContext &llvmContext = llvmFunc->getContext();

  if (VecTypeHintAttr vecTypeHint = func.getVecTypeHintAttr()) {
    Type type = vecTypeHint.getHint().getValue();
    toolchain::Type *llvmType = translation.convertType(type);
    bool isSigned = vecTypeHint.getIsSigned();
    llvmFunc->setMetadata(
        func.getVecTypeHintAttrName(),
        convertVecTypeHintToMDNode(llvmContext, llvmType, isSigned));
  }

  if (std::optional<ArrayRef<int32_t>> workGroupSizeHint =
          func.getWorkGroupSizeHint()) {
    llvmFunc->setMetadata(
        func.getWorkGroupSizeHintAttrName(),
        convertIntegerArrayToMDNode(llvmContext, *workGroupSizeHint));
  }

  if (std::optional<ArrayRef<int32_t>> reqdWorkGroupSize =
          func.getReqdWorkGroupSize()) {
    llvmFunc->setMetadata(
        func.getReqdWorkGroupSizeAttrName(),
        convertIntegerArrayToMDNode(llvmContext, *reqdWorkGroupSize));
  }

  if (std::optional<uint32_t> intelReqdSubGroupSize =
          func.getIntelReqdSubGroupSize()) {
    llvmFunc->setMetadata(
        func.getIntelReqdSubGroupSizeAttrName(),
        convertIntegerToMDNode(llvmContext,
                               toolchain::APInt(32, *intelReqdSubGroupSize)));
  }
}

static LogicalResult convertParameterAttr(toolchain::AttrBuilder &attrBuilder,
                                          toolchain::Attribute::AttrKind llvmKind,
                                          NamedAttribute namedAttr,
                                          ModuleTranslation &moduleTranslation,
                                          Location loc) {
  return toolchain::TypeSwitch<Attribute, LogicalResult>(namedAttr.getValue())
      .Case<TypeAttr>([&](auto typeAttr) {
        attrBuilder.addTypeAttr(
            llvmKind, moduleTranslation.convertType(typeAttr.getValue()));
        return success();
      })
      .Case<IntegerAttr>([&](auto intAttr) {
        attrBuilder.addRawIntAttr(llvmKind, intAttr.getInt());
        return success();
      })
      .Case<UnitAttr>([&](auto) {
        attrBuilder.addAttribute(llvmKind);
        return success();
      })
      .Case<LLVM::ConstantRangeAttr>([&](auto rangeAttr) {
        attrBuilder.addConstantRangeAttr(
            llvmKind,
            toolchain::ConstantRange(rangeAttr.getLower(), rangeAttr.getUpper()));
        return success();
      })
      .Default([loc](auto) {
        return emitError(loc, "unsupported parameter attribute type");
      });
}

FailureOr<toolchain::AttrBuilder>
ModuleTranslation::convertParameterAttrs(LLVMFuncOp func, int argIdx,
                                         DictionaryAttr paramAttrs) {
  toolchain::AttrBuilder attrBuilder(llvmModule->getContext());
  auto attrNameToKindMapping = getAttrNameToKindMapping();
  Location loc = func.getLoc();

  for (auto namedAttr : paramAttrs) {
    auto it = attrNameToKindMapping.find(namedAttr.getName());
    if (it != attrNameToKindMapping.end()) {
      toolchain::Attribute::AttrKind llvmKind = it->second;
      if (failed(convertParameterAttr(attrBuilder, llvmKind, namedAttr, *this,
                                      loc)))
        return failure();
    } else if (namedAttr.getNameDialect()) {
      if (failed(iface.convertParameterAttr(func, argIdx, namedAttr, *this)))
        return failure();
    }
  }

  return attrBuilder;
}

LogicalResult ModuleTranslation::convertArgAndResultAttrs(
    ArgAndResultAttrsOpInterface attrsOp, toolchain::CallBase *call,
    ArrayRef<unsigned> immArgPositions) {
  // Convert the argument attributes.
  if (ArrayAttr argAttrsArray = attrsOp.getArgAttrsAttr()) {
    unsigned argAttrIdx = 0;
    toolchain::SmallDenseSet<unsigned> immArgPositionsSet(immArgPositions.begin(),
                                                     immArgPositions.end());
    for (unsigned argIdx : toolchain::seq<unsigned>(call->arg_size())) {
      if (argAttrIdx >= argAttrsArray.size())
        break;
      // Skip immediate arguments (they have no entries in argAttrsArray).
      if (immArgPositionsSet.contains(argIdx))
        continue;
      // Skip empty argument attributes.
      auto argAttrs = cast<DictionaryAttr>(argAttrsArray[argAttrIdx++]);
      if (argAttrs.empty())
        continue;
      // Convert and add attributes to the call instruction.
      FailureOr<toolchain::AttrBuilder> attrBuilder =
          convertParameterAttrs(attrsOp->getLoc(), argAttrs);
      if (failed(attrBuilder))
        return failure();
      call->addParamAttrs(argIdx, *attrBuilder);
    }
  }

  // Convert the result attributes.
  if (ArrayAttr resAttrsArray = attrsOp.getResAttrsAttr()) {
    if (!resAttrsArray.empty()) {
      auto resAttrs = cast<DictionaryAttr>(resAttrsArray[0]);
      FailureOr<toolchain::AttrBuilder> attrBuilder =
          convertParameterAttrs(attrsOp->getLoc(), resAttrs);
      if (failed(attrBuilder))
        return failure();
      call->addRetAttrs(*attrBuilder);
    }
  }

  return success();
}

FailureOr<toolchain::AttrBuilder>
ModuleTranslation::convertParameterAttrs(Location loc,
                                         DictionaryAttr paramAttrs) {
  toolchain::AttrBuilder attrBuilder(llvmModule->getContext());
  auto attrNameToKindMapping = getAttrNameToKindMapping();

  for (auto namedAttr : paramAttrs) {
    auto it = attrNameToKindMapping.find(namedAttr.getName());
    if (it != attrNameToKindMapping.end()) {
      toolchain::Attribute::AttrKind llvmKind = it->second;
      if (failed(convertParameterAttr(attrBuilder, llvmKind, namedAttr, *this,
                                      loc)))
        return failure();
    }
  }

  return attrBuilder;
}

LogicalResult ModuleTranslation::convertFunctionSignatures() {
  // Declare all functions first because there may be function calls that form a
  // call graph with cycles, or global initializers that reference functions.
  for (auto function : getModuleBody(mlirModule).getOps<LLVMFuncOp>()) {
    toolchain::FunctionCallee llvmFuncCst = llvmModule->getOrInsertFunction(
        function.getName(),
        cast<toolchain::FunctionType>(convertType(function.getFunctionType())));
    toolchain::Function *llvmFunc = cast<toolchain::Function>(llvmFuncCst.getCallee());
    llvmFunc->setLinkage(convertLinkageToLLVM(function.getLinkage()));
    llvmFunc->setCallingConv(convertCConvToLLVM(function.getCConv()));
    mapFunction(function.getName(), llvmFunc);
    addRuntimePreemptionSpecifier(function.getDsoLocal(), llvmFunc);

    // Convert function attributes.
    convertFunctionAttributes(function, llvmFunc);

    // Convert function kernel attributes to metadata.
    convertFunctionKernelAttributes(function, llvmFunc, *this);

    // Convert function_entry_count attribute to metadata.
    if (std::optional<uint64_t> entryCount = function.getFunctionEntryCount())
      llvmFunc->setEntryCount(entryCount.value());

    // Convert result attributes.
    if (ArrayAttr allResultAttrs = function.getAllResultAttrs()) {
      DictionaryAttr resultAttrs = cast<DictionaryAttr>(allResultAttrs[0]);
      FailureOr<toolchain::AttrBuilder> attrBuilder =
          convertParameterAttrs(function, -1, resultAttrs);
      if (failed(attrBuilder))
        return failure();
      llvmFunc->addRetAttrs(*attrBuilder);
    }

    // Convert argument attributes.
    for (auto [argIdx, llvmArg] : toolchain::enumerate(llvmFunc->args())) {
      if (DictionaryAttr argAttrs = function.getArgAttrDict(argIdx)) {
        FailureOr<toolchain::AttrBuilder> attrBuilder =
            convertParameterAttrs(function, argIdx, argAttrs);
        if (failed(attrBuilder))
          return failure();
        llvmArg.addAttrs(*attrBuilder);
      }
    }

    // Forward the pass-through attributes to LLVM.
    FailureOr<toolchain::AttrBuilder> convertedPassthroughAttrs =
        convertMLIRAttributesToLLVM(function.getLoc(), llvmFunc->getContext(),
                                    function.getPassthroughAttr(),
                                    function.getPassthroughAttrName());
    if (failed(convertedPassthroughAttrs))
      return failure();
    llvmFunc->addFnAttrs(*convertedPassthroughAttrs);

    // Convert visibility attribute.
    llvmFunc->setVisibility(convertVisibilityToLLVM(function.getVisibility_()));

    // Convert the comdat attribute.
    if (std::optional<mlir::SymbolRefAttr> comdat = function.getComdat()) {
      auto selectorOp = cast<ComdatSelectorOp>(
          SymbolTable::lookupNearestSymbolFrom(function, *comdat));
      llvmFunc->setComdat(comdatMapping.lookup(selectorOp));
    }

    if (auto gc = function.getGarbageCollector())
      llvmFunc->setGC(gc->str());

    if (auto unnamedAddr = function.getUnnamedAddr())
      llvmFunc->setUnnamedAddr(convertUnnamedAddrToLLVM(*unnamedAddr));

    if (auto alignment = function.getAlignment())
      llvmFunc->setAlignment(toolchain::MaybeAlign(*alignment));

    // Translate the debug information for this function.
    debugTranslation->translate(function, *llvmFunc);
  }

  return success();
}

LogicalResult ModuleTranslation::convertFunctions() {
  // Convert functions.
  for (auto function : getModuleBody(mlirModule).getOps<LLVMFuncOp>()) {
    // Do not convert external functions, but do process dialect attributes
    // attached to them.
    if (function.isExternal()) {
      if (failed(convertDialectAttributes(function, {})))
        return failure();
      continue;
    }

    if (failed(convertOneFunction(function)))
      return failure();
  }

  return success();
}

LogicalResult ModuleTranslation::convertIFuncs() {
  for (auto op : getModuleBody(mlirModule).getOps<IFuncOp>()) {
    toolchain::Type *type = convertType(op.getIFuncType());
    toolchain::GlobalValue::LinkageTypes linkage =
        convertLinkageToLLVM(op.getLinkage());
    toolchain::Constant *resolver;
    if (auto *resolverFn = lookupFunction(op.getResolver())) {
      resolver = cast<toolchain::Constant>(resolverFn);
    } else {
      Operation *aliasOp = symbolTable().lookupSymbolIn(parentLLVMModule(op),
                                                        op.getResolverAttr());
      resolver = cast<toolchain::Constant>(lookupAlias(aliasOp));
    }

    auto *ifunc =
        toolchain::GlobalIFunc::create(type, op.getAddressSpace(), linkage,
                                  op.getSymName(), resolver, llvmModule.get());
    addRuntimePreemptionSpecifier(op.getDsoLocal(), ifunc);
    ifunc->setUnnamedAddr(convertUnnamedAddrToLLVM(op.getUnnamedAddr()));
    ifunc->setVisibility(convertVisibilityToLLVM(op.getVisibility_()));

    ifuncMapping.try_emplace(op, ifunc);
  }

  return success();
}

LogicalResult ModuleTranslation::convertComdats() {
  for (auto comdatOp : getModuleBody(mlirModule).getOps<ComdatOp>()) {
    for (auto selectorOp : comdatOp.getOps<ComdatSelectorOp>()) {
      toolchain::Module *module = getLLVMModule();
      if (module->getComdatSymbolTable().contains(selectorOp.getSymName()))
        return emitError(selectorOp.getLoc())
               << "comdat selection symbols must be unique even in different "
                  "comdat regions";
      toolchain::Comdat *comdat = module->getOrInsertComdat(selectorOp.getSymName());
      comdat->setSelectionKind(convertComdatToLLVM(selectorOp.getComdat()));
      comdatMapping.try_emplace(selectorOp, comdat);
    }
  }
  return success();
}

LogicalResult ModuleTranslation::convertUnresolvedBlockAddress() {
  for (auto &[blockAddressOp, llvmCst] : unresolvedBlockAddressMapping) {
    BlockAddressAttr blockAddressAttr = blockAddressOp.getBlockAddr();
    toolchain::BasicBlock *llvmBlock = lookupBlockAddress(blockAddressAttr);
    assert(llvmBlock && "expected LLVM blocks to be already translated");

    // Update mapping with new block address constant.
    auto *llvmBlockAddr = toolchain::BlockAddress::get(
        lookupFunction(blockAddressAttr.getFunction().getValue()), llvmBlock);
    llvmCst->replaceAllUsesWith(llvmBlockAddr);
    assert(llvmCst->use_empty() && "expected all uses to be replaced");
    cast<toolchain::GlobalVariable>(llvmCst)->eraseFromParent();
  }
  unresolvedBlockAddressMapping.clear();
  return success();
}

void ModuleTranslation::setAccessGroupsMetadata(AccessGroupOpInterface op,
                                                toolchain::Instruction *inst) {
  if (toolchain::MDNode *node = loopAnnotationTranslation->getAccessGroups(op))
    inst->setMetadata(toolchain::LLVMContext::MD_access_group, node);
}

toolchain::MDNode *
ModuleTranslation::getOrCreateAliasScope(AliasScopeAttr aliasScopeAttr) {
  auto [scopeIt, scopeInserted] =
      aliasScopeMetadataMapping.try_emplace(aliasScopeAttr, nullptr);
  if (!scopeInserted)
    return scopeIt->second;
  toolchain::LLVMContext &ctx = llvmModule->getContext();
  auto dummy = toolchain::MDNode::getTemporary(ctx, {});
  // Convert the domain metadata node if necessary.
  auto [domainIt, insertedDomain] = aliasDomainMetadataMapping.try_emplace(
      aliasScopeAttr.getDomain(), nullptr);
  if (insertedDomain) {
    toolchain::SmallVector<toolchain::Metadata *, 2> operands;
    // Placeholder for potential self-reference.
    operands.push_back(dummy.get());
    if (StringAttr description = aliasScopeAttr.getDomain().getDescription())
      operands.push_back(toolchain::MDString::get(ctx, description));
    domainIt->second = toolchain::MDNode::get(ctx, operands);
    // Self-reference for uniqueness.
    toolchain::Metadata *replacement;
    if (auto stringAttr =
            dyn_cast<StringAttr>(aliasScopeAttr.getDomain().getId()))
      replacement = toolchain::MDString::get(ctx, stringAttr.getValue());
    else
      replacement = domainIt->second;
    domainIt->second->replaceOperandWith(0, replacement);
  }
  // Convert the scope metadata node.
  assert(domainIt->second && "Scope's domain should already be valid");
  toolchain::SmallVector<toolchain::Metadata *, 3> operands;
  // Placeholder for potential self-reference.
  operands.push_back(dummy.get());
  operands.push_back(domainIt->second);
  if (StringAttr description = aliasScopeAttr.getDescription())
    operands.push_back(toolchain::MDString::get(ctx, description));
  scopeIt->second = toolchain::MDNode::get(ctx, operands);
  // Self-reference for uniqueness.
  toolchain::Metadata *replacement;
  if (auto stringAttr = dyn_cast<StringAttr>(aliasScopeAttr.getId()))
    replacement = toolchain::MDString::get(ctx, stringAttr.getValue());
  else
    replacement = scopeIt->second;
  scopeIt->second->replaceOperandWith(0, replacement);
  return scopeIt->second;
}

toolchain::MDNode *ModuleTranslation::getOrCreateAliasScopes(
    ArrayRef<AliasScopeAttr> aliasScopeAttrs) {
  SmallVector<toolchain::Metadata *> nodes;
  nodes.reserve(aliasScopeAttrs.size());
  for (AliasScopeAttr aliasScopeAttr : aliasScopeAttrs)
    nodes.push_back(getOrCreateAliasScope(aliasScopeAttr));
  return toolchain::MDNode::get(getLLVMContext(), nodes);
}

void ModuleTranslation::setAliasScopeMetadata(AliasAnalysisOpInterface op,
                                              toolchain::Instruction *inst) {
  auto populateScopeMetadata = [&](ArrayAttr aliasScopeAttrs, unsigned kind) {
    if (!aliasScopeAttrs || aliasScopeAttrs.empty())
      return;
    toolchain::MDNode *node = getOrCreateAliasScopes(
        toolchain::to_vector(aliasScopeAttrs.getAsRange<AliasScopeAttr>()));
    inst->setMetadata(kind, node);
  };

  populateScopeMetadata(op.getAliasScopesOrNull(),
                        toolchain::LLVMContext::MD_alias_scope);
  populateScopeMetadata(op.getNoAliasScopesOrNull(),
                        toolchain::LLVMContext::MD_noalias);
}

toolchain::MDNode *ModuleTranslation::getTBAANode(TBAATagAttr tbaaAttr) const {
  return tbaaMetadataMapping.lookup(tbaaAttr);
}

void ModuleTranslation::setTBAAMetadata(AliasAnalysisOpInterface op,
                                        toolchain::Instruction *inst) {
  ArrayAttr tagRefs = op.getTBAATagsOrNull();
  if (!tagRefs || tagRefs.empty())
    return;

  // LLVM IR currently does not support attaching more than one TBAA access tag
  // to a memory accessing instruction. It may be useful to support this in
  // future, but for the time being just ignore the metadata if MLIR operation
  // has multiple access tags.
  if (tagRefs.size() > 1) {
    op.emitWarning() << "TBAA access tags were not translated, because LLVM "
                        "IR only supports a single tag per instruction";
    return;
  }

  toolchain::MDNode *node = getTBAANode(cast<TBAATagAttr>(tagRefs[0]));
  inst->setMetadata(toolchain::LLVMContext::MD_tbaa, node);
}

void ModuleTranslation::setDereferenceableMetadata(
    DereferenceableOpInterface op, toolchain::Instruction *inst) {
  DereferenceableAttr derefAttr = op.getDereferenceableOrNull();
  if (!derefAttr)
    return;

  toolchain::MDNode *derefSizeNode = toolchain::MDNode::get(
      getLLVMContext(),
      toolchain::ConstantAsMetadata::get(toolchain::ConstantInt::get(
          toolchain::IntegerType::get(getLLVMContext(), 64), derefAttr.getBytes())));
  unsigned kindId = derefAttr.getMayBeNull()
                        ? toolchain::LLVMContext::MD_dereferenceable_or_null
                        : toolchain::LLVMContext::MD_dereferenceable;
  inst->setMetadata(kindId, derefSizeNode);
}

void ModuleTranslation::setBranchWeightsMetadata(WeightedBranchOpInterface op) {
  SmallVector<uint32_t> weights;
  toolchain::transform(op.getWeights(), std::back_inserter(weights),
                  [](int32_t value) { return static_cast<uint32_t>(value); });
  if (weights.empty())
    return;

  toolchain::Instruction *inst = isa<CallOp>(op) ? lookupCall(op) : lookupBranch(op);
  assert(inst && "expected the operation to have a mapping to an instruction");
  inst->setMetadata(
      toolchain::LLVMContext::MD_prof,
      toolchain::MDBuilder(getLLVMContext()).createBranchWeights(weights));
}

LogicalResult ModuleTranslation::createTBAAMetadata() {
  toolchain::LLVMContext &ctx = llvmModule->getContext();
  toolchain::IntegerType *offsetTy = toolchain::IntegerType::get(ctx, 64);

  // Walk the entire module and create all metadata nodes for the TBAA
  // attributes. The code below relies on two invariants of the
  // `AttrTypeWalker`:
  // 1. Attributes are visited in post-order: Since the attributes create a DAG,
  //    this ensures that any lookups into `tbaaMetadataMapping` for child
  //    attributes succeed.
  // 2. Attributes are only ever visited once: This way we don't leak any
  //    LLVM metadata instances.
  AttrTypeWalker walker;
  walker.addWalk([&](TBAARootAttr root) {
    toolchain::MDNode *node;
    if (StringAttr id = root.getId()) {
      node = toolchain::MDNode::get(ctx, toolchain::MDString::get(ctx, id));
    } else {
      // Anonymous root nodes are self-referencing.
      auto selfRef = toolchain::MDNode::getTemporary(ctx, {});
      node = toolchain::MDNode::get(ctx, {selfRef.get()});
      node->replaceOperandWith(0, node);
    }
    tbaaMetadataMapping.insert({root, node});
  });

  walker.addWalk([&](TBAATypeDescriptorAttr descriptor) {
    SmallVector<toolchain::Metadata *> operands;
    operands.push_back(toolchain::MDString::get(ctx, descriptor.getId()));
    for (TBAAMemberAttr member : descriptor.getMembers()) {
      operands.push_back(tbaaMetadataMapping.lookup(member.getTypeDesc()));
      operands.push_back(toolchain::ConstantAsMetadata::get(
          toolchain::ConstantInt::get(offsetTy, member.getOffset())));
    }

    tbaaMetadataMapping.insert({descriptor, toolchain::MDNode::get(ctx, operands)});
  });

  walker.addWalk([&](TBAATagAttr tag) {
    SmallVector<toolchain::Metadata *> operands;

    operands.push_back(tbaaMetadataMapping.lookup(tag.getBaseType()));
    operands.push_back(tbaaMetadataMapping.lookup(tag.getAccessType()));

    operands.push_back(toolchain::ConstantAsMetadata::get(
        toolchain::ConstantInt::get(offsetTy, tag.getOffset())));
    if (tag.getConstant())
      operands.push_back(
          toolchain::ConstantAsMetadata::get(toolchain::ConstantInt::get(offsetTy, 1)));

    tbaaMetadataMapping.insert({tag, toolchain::MDNode::get(ctx, operands)});
  });

  mlirModule->walk([&](AliasAnalysisOpInterface analysisOpInterface) {
    if (auto attr = analysisOpInterface.getTBAATagsOrNull())
      walker.walk(attr);
  });

  return success();
}

LogicalResult ModuleTranslation::createIdentMetadata() {
  if (auto attr = mlirModule->getAttrOfType<StringAttr>(
          LLVMDialect::getIdentAttrName())) {
    StringRef ident = attr;
    toolchain::LLVMContext &ctx = llvmModule->getContext();
    toolchain::NamedMDNode *namedMd =
        llvmModule->getOrInsertNamedMetadata(LLVMDialect::getIdentAttrName());
    toolchain::MDNode *md = toolchain::MDNode::get(ctx, toolchain::MDString::get(ctx, ident));
    namedMd->addOperand(md);
  }

  return success();
}

LogicalResult ModuleTranslation::createCommandlineMetadata() {
  if (auto attr = mlirModule->getAttrOfType<StringAttr>(
          LLVMDialect::getCommandlineAttrName())) {
    StringRef cmdLine = attr;
    toolchain::LLVMContext &ctx = llvmModule->getContext();
    toolchain::NamedMDNode *nmd = llvmModule->getOrInsertNamedMetadata(
        LLVMDialect::getCommandlineAttrName());
    toolchain::MDNode *md =
        toolchain::MDNode::get(ctx, toolchain::MDString::get(ctx, cmdLine));
    nmd->addOperand(md);
  }

  return success();
}

LogicalResult ModuleTranslation::createDependentLibrariesMetadata() {
  if (auto dependentLibrariesAttr = mlirModule->getDiscardableAttr(
          LLVM::LLVMDialect::getDependentLibrariesAttrName())) {
    auto *nmd =
        llvmModule->getOrInsertNamedMetadata("toolchain.dependent-libraries");
    toolchain::LLVMContext &ctx = llvmModule->getContext();
    for (auto libAttr :
         cast<ArrayAttr>(dependentLibrariesAttr).getAsRange<StringAttr>()) {
      auto *md =
          toolchain::MDNode::get(ctx, toolchain::MDString::get(ctx, libAttr.getValue()));
      nmd->addOperand(md);
    }
  }
  return success();
}

void ModuleTranslation::setLoopMetadata(Operation *op,
                                        toolchain::Instruction *inst) {
  LoopAnnotationAttr attr =
      TypeSwitch<Operation *, LoopAnnotationAttr>(op)
          .Case<LLVM::BrOp, LLVM::CondBrOp>(
              [](auto branchOp) { return branchOp.getLoopAnnotationAttr(); });
  if (!attr)
    return;
  toolchain::MDNode *loopMD =
      loopAnnotationTranslation->translateLoopAnnotation(attr, op);
  inst->setMetadata(toolchain::LLVMContext::MD_loop, loopMD);
}

void ModuleTranslation::setDisjointFlag(Operation *op, toolchain::Value *value) {
  auto iface = cast<DisjointFlagInterface>(op);
  // We do a dyn_cast here in case the value got folded into a constant.
  if (auto disjointInst = dyn_cast<toolchain::PossiblyDisjointInst>(value))
    disjointInst->setIsDisjoint(iface.getIsDisjoint());
}

toolchain::Type *ModuleTranslation::convertType(Type type) {
  return typeTranslator.translateType(type);
}

/// A helper to look up remapped operands in the value remapping table.
SmallVector<toolchain::Value *> ModuleTranslation::lookupValues(ValueRange values) {
  SmallVector<toolchain::Value *> remapped;
  remapped.reserve(values.size());
  for (Value v : values)
    remapped.push_back(lookupValue(v));
  return remapped;
}

toolchain::OpenMPIRBuilder *ModuleTranslation::getOpenMPBuilder() {
  if (!ompBuilder) {
    ompBuilder = std::make_unique<toolchain::OpenMPIRBuilder>(*llvmModule);

    // Flags represented as top-level OpenMP dialect attributes are set in
    // `OpenMPDialectLLVMIRTranslationInterface::amendOperation()`. Here we set
    // the default configuration.
    toolchain::OpenMPIRBuilderConfig config(
        /* IsTargetDevice = */ false, /* IsGPU = */ false,
        /* OpenMPOffloadMandatory = */ false,
        /* HasRequiresReverseOffload = */ false,
        /* HasRequiresUnifiedAddress = */ false,
        /* HasRequiresUnifiedSharedMemory = */ false,
        /* HasRequiresDynamicAllocators = */ false);
    unsigned int defaultAS =
        llvmModule->getDataLayout().getProgramAddressSpace();
    config.setDefaultTargetAS(defaultAS);
    config.setRuntimeCC(llvmModule->getTargetTriple().isSPIRV()
                            ? toolchain::CallingConv::SPIR_FUNC
                            : toolchain::CallingConv::C);
    ompBuilder->setConfig(std::move(config));
    ompBuilder->initialize();
  }
  return ompBuilder.get();
}

toolchain::DILocation *ModuleTranslation::translateLoc(Location loc,
                                                  toolchain::DILocalScope *scope) {
  return debugTranslation->translateLoc(loc, scope);
}

toolchain::DIExpression *
ModuleTranslation::translateExpression(LLVM::DIExpressionAttr attr) {
  return debugTranslation->translateExpression(attr);
}

toolchain::DIGlobalVariableExpression *
ModuleTranslation::translateGlobalVariableExpression(
    LLVM::DIGlobalVariableExpressionAttr attr) {
  return debugTranslation->translateGlobalVariableExpression(attr);
}

toolchain::Metadata *ModuleTranslation::translateDebugInfo(LLVM::DINodeAttr attr) {
  return debugTranslation->translate(attr);
}

toolchain::RoundingMode
ModuleTranslation::translateRoundingMode(LLVM::RoundingMode rounding) {
  return convertRoundingModeToLLVM(rounding);
}

toolchain::fp::ExceptionBehavior ModuleTranslation::translateFPExceptionBehavior(
    LLVM::FPExceptionBehavior exceptionBehavior) {
  return convertFPExceptionBehaviorToLLVM(exceptionBehavior);
}

toolchain::NamedMDNode *
ModuleTranslation::getOrInsertNamedModuleMetadata(StringRef name) {
  return llvmModule->getOrInsertNamedMetadata(name);
}

static std::unique_ptr<toolchain::Module>
prepareLLVMModule(Operation *m, toolchain::LLVMContext &llvmContext,
                  StringRef name) {
  m->getContext()->getOrLoadDialect<LLVM::LLVMDialect>();
  auto llvmModule = std::make_unique<toolchain::Module>(name, llvmContext);
  if (auto dataLayoutAttr =
          m->getDiscardableAttr(LLVM::LLVMDialect::getDataLayoutAttrName())) {
    llvmModule->setDataLayout(cast<StringAttr>(dataLayoutAttr).getValue());
  } else {
    FailureOr<toolchain::DataLayout> llvmDataLayout(toolchain::DataLayout(""));
    if (auto iface = dyn_cast<DataLayoutOpInterface>(m)) {
      if (DataLayoutSpecInterface spec = iface.getDataLayoutSpec()) {
        llvmDataLayout =
            translateDataLayout(spec, DataLayout(iface), m->getLoc());
      }
    } else if (auto mod = dyn_cast<ModuleOp>(m)) {
      if (DataLayoutSpecInterface spec = mod.getDataLayoutSpec()) {
        llvmDataLayout =
            translateDataLayout(spec, DataLayout(mod), m->getLoc());
      }
    }
    if (failed(llvmDataLayout))
      return nullptr;
    llvmModule->setDataLayout(*llvmDataLayout);
  }
  if (auto targetTripleAttr =
          m->getDiscardableAttr(LLVM::LLVMDialect::getTargetTripleAttrName()))
    llvmModule->setTargetTriple(
        toolchain::Triple(cast<StringAttr>(targetTripleAttr).getValue()));

  if (auto asmAttr = m->getDiscardableAttr(
          LLVM::LLVMDialect::getModuleLevelAsmAttrName())) {
    auto asmArrayAttr = dyn_cast<ArrayAttr>(asmAttr);
    if (!asmArrayAttr) {
      m->emitError("expected an array attribute for a module level asm");
      return nullptr;
    }

    for (Attribute elt : asmArrayAttr) {
      auto asmStrAttr = dyn_cast<StringAttr>(elt);
      if (!asmStrAttr) {
        m->emitError(
            "expected a string attribute for each entry of a module level asm");
        return nullptr;
      }
      llvmModule->appendModuleInlineAsm(asmStrAttr.getValue());
    }
  }

  return llvmModule;
}

std::unique_ptr<toolchain::Module>
mlir::translateModuleToLLVMIR(Operation *module, toolchain::LLVMContext &llvmContext,
                              StringRef name, bool disableVerification) {
  if (!satisfiesLLVMModule(module)) {
    module->emitOpError("can not be translated to an LLVMIR module");
    return nullptr;
  }

  std::unique_ptr<toolchain::Module> llvmModule =
      prepareLLVMModule(module, llvmContext, name);
  if (!llvmModule)
    return nullptr;

  LLVM::ensureDistinctSuccessors(module);
  LLVM::legalizeDIExpressionsRecursively(module);

  ModuleTranslation translator(module, std::move(llvmModule));
  toolchain::IRBuilder<toolchain::TargetFolder> llvmBuilder(
      llvmContext,
      toolchain::TargetFolder(translator.getLLVMModule()->getDataLayout()));

  // Convert module before functions and operations inside, so dialect
  // attributes can be used to change dialect-specific global configurations via
  // `amendOperation()`. These configurations can then influence the translation
  // of operations afterwards.
  if (failed(translator.convertOperation(*module, llvmBuilder)))
    return nullptr;

  if (failed(translator.convertComdats()))
    return nullptr;
  if (failed(translator.convertFunctionSignatures()))
    return nullptr;
  if (failed(translator.convertGlobalsAndAliases()))
    return nullptr;
  if (failed(translator.convertIFuncs()))
    return nullptr;
  if (failed(translator.createTBAAMetadata()))
    return nullptr;
  if (failed(translator.createIdentMetadata()))
    return nullptr;
  if (failed(translator.createCommandlineMetadata()))
    return nullptr;
  if (failed(translator.createDependentLibrariesMetadata()))
    return nullptr;

  // Convert other top-level operations if possible.
  for (Operation &o : getModuleBody(module).getOperations()) {
    if (!isa<LLVM::LLVMFuncOp, LLVM::AliasOp, LLVM::GlobalOp,
             LLVM::GlobalCtorsOp, LLVM::GlobalDtorsOp, LLVM::ComdatOp,
             LLVM::IFuncOp>(&o) &&
        !o.hasTrait<OpTrait::IsTerminator>() &&
        failed(translator.convertOperation(o, llvmBuilder))) {
      return nullptr;
    }
  }

  // Operations in function bodies with symbolic references must be converted
  // after the top-level operations they refer to are declared, so we do it
  // last.
  if (failed(translator.convertFunctions()))
    return nullptr;

  // Now that all MLIR blocks are resolved into LLVM ones, patch block address
  // constants to point to the correct blocks.
  if (failed(translator.convertUnresolvedBlockAddress()))
    return nullptr;

  // Add the necessary debug info module flags, if they were not encoded in MLIR
  // beforehand.
  translator.debugTranslation->addModuleFlagsIfNotPresent();

  // Call the OpenMP IR Builder callbacks prior to verifying the module
  if (auto *ompBuilder = translator.getOpenMPBuilder())
    ompBuilder->finalize();

  if (!disableVerification &&
      toolchain::verifyModule(*translator.llvmModule, &toolchain::errs()))
    return nullptr;

  return std::move(translator.llvmModule);
}
