//===- TypeToLLVM.cpp - type translation from MLIR to LLVM IR -===//
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

#include "mlir/Target/LLVMIR/TypeToLLVM.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/IR/BuiltinTypes.h"

#include "vm/core/ADT/TypeSwitch.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/Type.h"

using namespace mlir;

namespace mlir {
namespace LLVM {
namespace detail {
/// Support for translating MLIR LLVM dialect types to LLVM IR.
class TypeToLLVMIRTranslatorImpl {
public:
  /// Constructs a class creating types in the given LLVM context.
  TypeToLLVMIRTranslatorImpl(toolchain::LLVMContext &context) : context(context) {}

  /// Translates a single type.
  toolchain::Type *translateType(Type type) {
    // If the conversion is already known, just return it.
    if (knownTranslations.count(type))
      return knownTranslations.lookup(type);

    // Dispatch to an appropriate function.
    toolchain::Type *translated =
        toolchain::TypeSwitch<Type, toolchain::Type *>(type)
            .Case([this](LLVM::LLVMVoidType) {
              return toolchain::Type::getVoidTy(context);
            })
            .Case(
                [this](Float16Type) { return toolchain::Type::getHalfTy(context); })
            .Case([this](BFloat16Type) {
              return toolchain::Type::getBFloatTy(context);
            })
            .Case(
                [this](Float32Type) { return toolchain::Type::getFloatTy(context); })
            .Case([this](Float64Type) {
              return toolchain::Type::getDoubleTy(context);
            })
            .Case([this](Float80Type) {
              return toolchain::Type::getX86_FP80Ty(context);
            })
            .Case([this](Float128Type) {
              return toolchain::Type::getFP128Ty(context);
            })
            .Case([this](LLVM::LLVMPPCFP128Type) {
              return toolchain::Type::getPPC_FP128Ty(context);
            })
            .Case([this](LLVM::LLVMTokenType) {
              return toolchain::Type::getTokenTy(context);
            })
            .Case([this](LLVM::LLVMLabelType) {
              return toolchain::Type::getLabelTy(context);
            })
            .Case([this](LLVM::LLVMMetadataType) {
              return toolchain::Type::getMetadataTy(context);
            })
            .Case([this](LLVM::LLVMX86AMXType) {
              return toolchain::Type::getX86_AMXTy(context);
            })
            .Case<LLVM::LLVMArrayType, IntegerType, LLVM::LLVMFunctionType,
                  LLVM::LLVMPointerType, LLVM::LLVMStructType, VectorType,
                  LLVM::LLVMTargetExtType, PtrLikeTypeInterface>(
                [this](auto type) { return this->translate(type); })
            .DefaultUnreachable("unknown LLVM dialect type");

    // Cache the result of the conversion and return.
    knownTranslations.try_emplace(type, translated);
    return translated;
  }

private:
  /// Translates the given array type.
  toolchain::Type *translate(LLVM::LLVMArrayType type) {
    return toolchain::ArrayType::get(translateType(type.getElementType()),
                                type.getNumElements());
  }

  /// Translates the given function type.
  toolchain::Type *translate(LLVM::LLVMFunctionType type) {
    SmallVector<toolchain::Type *, 8> paramTypes;
    translateTypes(type.getParams(), paramTypes);
    return toolchain::FunctionType::get(translateType(type.getReturnType()),
                                   paramTypes, type.isVarArg());
  }

  /// Translates the given integer type.
  toolchain::Type *translate(IntegerType type) {
    return toolchain::IntegerType::get(context, type.getWidth());
  }

  /// Translates the given pointer type.
  toolchain::Type *translate(LLVM::LLVMPointerType type) {
    return toolchain::PointerType::get(context, type.getAddressSpace());
  }

  /// Translates the given structure type, supports both identified and literal
  /// structs. This will _create_ a new identified structure every time, use
  /// `convertType` if a structure with the same name must be looked up instead.
  toolchain::Type *translate(LLVM::LLVMStructType type) {
    SmallVector<toolchain::Type *, 8> subtypes;
    if (!type.isIdentified()) {
      translateTypes(type.getBody(), subtypes);
      return toolchain::StructType::get(context, subtypes, type.isPacked());
    }

    toolchain::StructType *structType =
        toolchain::StructType::create(context, type.getName());
    // Mark the type we just created as known so that recursive calls can pick
    // it up and use directly.
    knownTranslations.try_emplace(type, structType);
    if (type.isOpaque())
      return structType;

    translateTypes(type.getBody(), subtypes);
    structType->setBody(subtypes, type.isPacked());
    return structType;
  }

  /// Translates the given built-in vector type compatible with LLVM.
  toolchain::Type *translate(VectorType type) {
    assert(LLVM::isCompatibleVectorType(type) &&
           "expected compatible with LLVM vector type");
    if (type.isScalable())
      return toolchain::ScalableVectorType::get(translateType(type.getElementType()),
                                           type.getNumElements());
    return toolchain::FixedVectorType::get(translateType(type.getElementType()),
                                      type.getNumElements());
  }

  /// Translates the given target extension type.
  toolchain::Type *translate(LLVM::LLVMTargetExtType type) {
    SmallVector<toolchain::Type *> typeParams;
    translateTypes(type.getTypeParams(), typeParams);
    return toolchain::TargetExtType::get(context, type.getExtTypeName(), typeParams,
                                    type.getIntParams());
  }

  /// Translates the given ptr type.
  toolchain::Type *translate(PtrLikeTypeInterface type) {
    auto memSpace =
        dyn_cast<LLVM::LLVMAddrSpaceAttrInterface>(type.getMemorySpace());
    assert(memSpace && "expected pointer with an LLVM address space");
    assert(!type.hasPtrMetadata() && "expected pointer without metadata");
    return toolchain::PointerType::get(context, memSpace.getAddressSpace());
  }

  /// Translates a list of types.
  void translateTypes(ArrayRef<Type> types,
                      SmallVectorImpl<toolchain::Type *> &result) {
    result.reserve(result.size() + types.size());
    for (auto type : types)
      result.push_back(translateType(type));
  }

  /// Reference to the context in which the LLVM IR types are created.
  toolchain::LLVMContext &context;

  /// Map of known translation. This serves a double purpose: caches translation
  /// results to avoid repeated recursive calls and makes sure identified
  /// structs with the same name (that is, equal) are resolved to an existing
  /// type instead of creating a new type.
  toolchain::DenseMap<Type, toolchain::Type *> knownTranslations;
};
} // namespace detail
} // namespace LLVM
} // namespace mlir

LLVM::TypeToLLVMIRTranslator::TypeToLLVMIRTranslator(toolchain::LLVMContext &context)
    : impl(new detail::TypeToLLVMIRTranslatorImpl(context)) {}

LLVM::TypeToLLVMIRTranslator::~TypeToLLVMIRTranslator() = default;

toolchain::Type *LLVM::TypeToLLVMIRTranslator::translateType(Type type) {
  return impl->translateType(type);
}

unsigned LLVM::TypeToLLVMIRTranslator::getPreferredAlignment(
    Type type, const toolchain::DataLayout &layout) {
  return layout.getPrefTypeAlign(translateType(type)).value();
}
