//===- TypeFromLLVM.cpp - type translation from LLVM to MLIR IR -===//
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

#include "mlir/Target/LLVMIR/TypeFromLLVM.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"

#include "vm/core/ADT/TypeSwitch.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/Type.h"

using namespace mlir;

namespace mlir {
namespace LLVM {
namespace detail {
/// Support for translating LLVM IR types to MLIR LLVM dialect types.
class TypeFromLLVMIRTranslatorImpl {
public:
  /// Constructs a class creating types in the given MLIR context.
  TypeFromLLVMIRTranslatorImpl(MLIRContext &context,
                               bool importStructsAsLiterals)
      : context(context), importStructsAsLiterals(importStructsAsLiterals) {}

  /// Translates the given type.
  Type translateType(toolchain::Type *type) {
    if (knownTranslations.count(type))
      return knownTranslations.lookup(type);

    Type translated =
        toolchain::TypeSwitch<toolchain::Type *, Type>(type)
            .Case<toolchain::ArrayType, toolchain::FunctionType, toolchain::IntegerType,
                  toolchain::PointerType, toolchain::StructType, toolchain::FixedVectorType,
                  toolchain::ScalableVectorType, toolchain::TargetExtType>(
                [this](auto *type) { return this->translate(type); })
            .Default([this](toolchain::Type *type) {
              return translatePrimitiveType(type);
            });
    knownTranslations.try_emplace(type, translated);
    return translated;
  }

private:
  /// Translates the given primitive, i.e. non-parametric in MLIR nomenclature,
  /// type.
  Type translatePrimitiveType(toolchain::Type *type) {
    if (type->isVoidTy())
      return LLVM::LLVMVoidType::get(&context);
    if (type->isHalfTy())
      return Float16Type::get(&context);
    if (type->isBFloatTy())
      return BFloat16Type::get(&context);
    if (type->isFloatTy())
      return Float32Type::get(&context);
    if (type->isDoubleTy())
      return Float64Type::get(&context);
    if (type->isFP128Ty())
      return Float128Type::get(&context);
    if (type->isX86_FP80Ty())
      return Float80Type::get(&context);
    if (type->isX86_AMXTy())
      return LLVM::LLVMX86AMXType::get(&context);
    if (type->isPPC_FP128Ty())
      return LLVM::LLVMPPCFP128Type::get(&context);
    if (type->isLabelTy())
      return LLVM::LLVMLabelType::get(&context);
    if (type->isMetadataTy())
      return LLVM::LLVMMetadataType::get(&context);
    if (type->isTokenTy())
      return LLVM::LLVMTokenType::get(&context);
    llvm_unreachable("not a primitive type");
  }

  /// Translates the given array type.
  Type translate(toolchain::ArrayType *type) {
    return LLVM::LLVMArrayType::get(translateType(type->getElementType()),
                                    type->getNumElements());
  }

  /// Translates the given function type.
  Type translate(toolchain::FunctionType *type) {
    SmallVector<Type, 8> paramTypes;
    translateTypes(type->params(), paramTypes);
    return LLVM::LLVMFunctionType::get(translateType(type->getReturnType()),
                                       paramTypes, type->isVarArg());
  }

  /// Translates the given integer type.
  Type translate(toolchain::IntegerType *type) {
    return IntegerType::get(&context, type->getBitWidth());
  }

  /// Translates the given pointer type.
  Type translate(toolchain::PointerType *type) {
    return LLVM::LLVMPointerType::get(&context, type->getAddressSpace());
  }

  /// Translates the given structure type.
  Type translate(toolchain::StructType *type) {
    SmallVector<Type, 8> subtypes;
    if (type->isLiteral() || importStructsAsLiterals) {
      translateTypes(type->subtypes(), subtypes);
      return LLVM::LLVMStructType::getLiteral(&context, subtypes,
                                              type->isPacked());
    }

    if (type->isOpaque())
      return LLVM::LLVMStructType::getOpaque(type->getName(), &context);

    // With opaque pointers, types in LLVM can't be recursive anymore. Note that
    // using getIdentified is not possible, as type names in LLVM are not
    // guaranteed to be unique.
    translateTypes(type->subtypes(), subtypes);
    LLVM::LLVMStructType translated = LLVM::LLVMStructType::getNewIdentified(
        &context, type->getName(), subtypes, type->isPacked());
    knownTranslations.try_emplace(type, translated);
    return translated;
  }

  /// Translates the given fixed-vector type.
  Type translate(toolchain::FixedVectorType *type) {
    return VectorType::get(type->getNumElements(),
                           translateType(type->getElementType()));
  }

  /// Translates the given scalable-vector type.
  Type translate(toolchain::ScalableVectorType *type) {
    return VectorType::get(type->getMinNumElements(),
                           translateType(type->getElementType()),
                           /*scalableDims=*/true);
  }

  /// Translates the given target extension type.
  Type translate(toolchain::TargetExtType *type) {
    SmallVector<Type> typeParams;
    translateTypes(type->type_params(), typeParams);

    return LLVM::LLVMTargetExtType::get(&context, type->getName(), typeParams,
                                        type->int_params());
  }

  /// Translates a list of types.
  void translateTypes(ArrayRef<toolchain::Type *> types,
                      SmallVectorImpl<Type> &result) {
    result.reserve(result.size() + types.size());
    for (toolchain::Type *type : types)
      result.push_back(translateType(type));
  }

  /// Map of known translations. Serves as a cache and as recursion stopper for
  /// translating recursive structs.
  toolchain::DenseMap<toolchain::Type *, Type> knownTranslations;

  /// The context in which MLIR types are created.
  MLIRContext &context;

  /// Controls if structs should be imported as literal structs, i.e., nameless
  /// structs.
  bool importStructsAsLiterals;
};

} // namespace detail
} // namespace LLVM
} // namespace mlir

LLVM::TypeFromLLVMIRTranslator::TypeFromLLVMIRTranslator(
    MLIRContext &context, bool importStructsAsLiterals)
    : impl(std::make_unique<detail::TypeFromLLVMIRTranslatorImpl>(
          context, importStructsAsLiterals)) {}

LLVM::TypeFromLLVMIRTranslator::~TypeFromLLVMIRTranslator() = default;

Type LLVM::TypeFromLLVMIRTranslator::translateType(toolchain::Type *type) {
  return impl->translateType(type);
}
