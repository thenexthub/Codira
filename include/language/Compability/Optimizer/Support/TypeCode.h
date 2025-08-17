//===-- Optimizer/Support/TypeCode.h ----------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_TYPECODE_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_TYPECODE_H

#include "language/Compability/Common/ISO_Fortran_binding_wrapper.h"
#include "toolchain/Support/ErrorHandling.h"

namespace fir {

//===----------------------------------------------------------------------===//
// Translations of category and bitwidths to the type codes defined in flang's
// ISO_Fortran_binding.h.
//===----------------------------------------------------------------------===//

inline int characterBitsToTypeCode(unsigned bitwidth) {
  // clang-format off
  switch (bitwidth) {
  case 8:  return CFI_type_char;
  case 16: return CFI_type_char16_t;
  case 32: return CFI_type_char32_t;
  default: toolchain_unreachable("unsupported character size");
  }
  // clang-format on
}

inline int complexBitsToTypeCode(unsigned bitwidth) {
  // clang-format off
  switch (bitwidth) {
  case 16:  return CFI_type_half_float_Complex; // CFI_type_bfloat_Complex ?
  case 32:  return CFI_type_float_Complex;
  case 64:  return CFI_type_double_Complex;
  case 80:  return CFI_type_extended_double_Complex;
  case 128: return CFI_type_float128_Complex;
  default:  toolchain_unreachable("unsupported complex size");
  }
  // clang-format on
}

inline int integerBitsToTypeCode(unsigned bitwidth) {
  // clang-format off
  switch (bitwidth) {
  case 8:   return CFI_type_int8_t;
  case 16:  return CFI_type_int16_t;
  case 32:  return CFI_type_int32_t;
  case 64:  return CFI_type_int64_t;
  case 128: return CFI_type_int128_t;
  default:  toolchain_unreachable("unsupported integer size");
  }
  // clang-format on
}

inline int logicalBitsToTypeCode(unsigned bitwidth) {
  // clang-format off
  switch (bitwidth) {
  case 8: return CFI_type_Bool;
  case 16: return CFI_type_int_least16_t;
  case 32: return CFI_type_int_least32_t;
  case 64: return CFI_type_int_least64_t;
  default: toolchain_unreachable("unsupported logical size");
  }
  // clang-format on
}

inline int realBitsToTypeCode(unsigned bitwidth) {
  // clang-format off
  switch (bitwidth) {
  case 16:  return CFI_type_half_float; // CFI_type_bfloat ?
  case 32:  return CFI_type_float;
  case 64:  return CFI_type_double;
  case 80:  return CFI_type_extended_double;
  case 128: return CFI_type_float128;
  default:  toolchain_unreachable("unsupported real size");
  }
  // clang-format on
}

static constexpr int derivedToTypeCode() { return CFI_type_struct; }

} // namespace fir

#endif // FORTRAN_OPTIMIZER_SUPPORT_TYPECODE_H
