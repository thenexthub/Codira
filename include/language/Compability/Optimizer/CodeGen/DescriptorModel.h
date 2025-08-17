//===-- DescriptorModel.h -- model of descriptors for codegen ---*- C++ -*-===//
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
// LLVM IR dialect models of C++ types.
//
// This supplies a set of model builders to decompose the C declaration of a
// descriptor (as encoded in ISO_Fortran_binding.h and elsewhere) and
// reconstruct that type in the LLVM IR dialect.
//
// TODO: It is understood that this is deeply incorrect as far as building a
// portability layer for cross-compilation as these reflected types are those of
// the build machine and not necessarily that of either the host or the target.
// This assumption that build == host == target is actually pervasive across the
// compiler (https://toolchain.org/PR52418).
//
//===----------------------------------------------------------------------===//

#ifndef OPTIMIZER_DESCRIPTOR_MODEL_H
#define OPTIMIZER_DESCRIPTOR_MODEL_H

#include "language/Compability/Common/ISO_Fortran_binding_wrapper.h"
#include "language/Compability/Runtime/descriptor-consts.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/IR/BuiltinTypes.h"
#include "toolchain/Support/ErrorHandling.h"
#include <tuple>

namespace fir {

using TypeBuilderFunc = mlir::Type (*)(mlir::MLIRContext *);

/// Get the LLVM IR dialect model for building a particular C++ type, `T`.
template <typename T>
static TypeBuilderFunc getModel();

template <>
constexpr TypeBuilderFunc getModel<void *>() {
  return [](mlir::MLIRContext *context) -> mlir::Type {
    return mlir::LLVM::LLVMPointerType::get(context);
  };
}
template <>
constexpr TypeBuilderFunc getModel<unsigned>() {
  return [](mlir::MLIRContext *context) -> mlir::Type {
    return mlir::IntegerType::get(context, sizeof(unsigned) * 8);
  };
}
template <>
constexpr TypeBuilderFunc getModel<int>() {
  return [](mlir::MLIRContext *context) -> mlir::Type {
    return mlir::IntegerType::get(context, sizeof(int) * 8);
  };
}
template <>
constexpr TypeBuilderFunc getModel<unsigned long>() {
  return [](mlir::MLIRContext *context) -> mlir::Type {
    return mlir::IntegerType::get(context, sizeof(unsigned long) * 8);
  };
}
template <>
constexpr TypeBuilderFunc getModel<unsigned long long>() {
  return [](mlir::MLIRContext *context) -> mlir::Type {
    return mlir::IntegerType::get(context, sizeof(unsigned long long) * 8);
  };
}
template <>
constexpr TypeBuilderFunc getModel<long long>() {
  return [](mlir::MLIRContext *context) -> mlir::Type {
    return mlir::IntegerType::get(context, sizeof(long long) * 8);
  };
}
template <>
constexpr TypeBuilderFunc getModel<language::Compability::ISO::CFI_rank_t>() {
  return [](mlir::MLIRContext *context) -> mlir::Type {
    return mlir::IntegerType::get(context,
                                  sizeof(language::Compability::ISO::CFI_rank_t) * 8);
  };
}
template <>
constexpr TypeBuilderFunc getModel<language::Compability::ISO::CFI_type_t>() {
  return [](mlir::MLIRContext *context) -> mlir::Type {
    return mlir::IntegerType::get(context,
                                  sizeof(language::Compability::ISO::CFI_type_t) * 8);
  };
}
template <>
constexpr TypeBuilderFunc getModel<long>() {
  return [](mlir::MLIRContext *context) -> mlir::Type {
    return mlir::IntegerType::get(context, sizeof(long) * 8);
  };
}
template <>
constexpr TypeBuilderFunc getModel<language::Compability::ISO::CFI_dim_t>() {
  return [](mlir::MLIRContext *context) -> mlir::Type {
    auto indexTy = getModel<language::Compability::ISO::CFI_index_t>()(context);
    return mlir::LLVM::LLVMArrayType::get(indexTy, 3);
  };
}
template <>
constexpr TypeBuilderFunc
getModel<language::Compability::ISO::cfi_internal::FlexibleArray<language::Compability::ISO::CFI_dim_t>>() {
  return getModel<language::Compability::ISO::CFI_dim_t>();
}

//===----------------------------------------------------------------------===//
// Descriptor reflection
//===----------------------------------------------------------------------===//

/// Get the type model of the field number `Field` in an ISO CFI descriptor.
template <int Field>
static constexpr TypeBuilderFunc getDescFieldTypeModel() {
  language::Compability::ISO::CFI_cdesc_t dummyDesc{};
  // check that the descriptor is exactly 8 fields as specified in CFI_cdesc_t
  // in flang/include/flang/ISO_Fortran_binding.h.
  auto [a, b, c, d, e, f, g, h] = dummyDesc;
  auto tup = std::tie(a, b, c, d, e, f, g, h);
  auto field = std::get<Field>(tup);
  return getModel<decltype(field)>();
}

/// An extended descriptor is defined by a class in runtime/descriptor.h. The
/// three fields in the class are hard-coded here, unlike the reflection used on
/// the ISO parts, which are a POD.
template <int Field>
static constexpr TypeBuilderFunc getExtendedDescFieldTypeModel() {
  if constexpr (Field == 8) {
    return getModel<void *>();
  } else if constexpr (Field == 9) {
    return getModel<language::Compability::runtime::typeInfo::TypeParameterValue>();
  } else {
    toolchain_unreachable("extended ISO descriptor only has 10 fields");
  }
}

} // namespace fir

#endif // OPTIMIZER_DESCRIPTOR_MODEL_H
