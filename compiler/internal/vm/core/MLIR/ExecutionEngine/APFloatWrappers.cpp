//===- APFloatWrappers.cpp - Software Implementation of FP Arithmetics --- ===//
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
// This file exposes the APFloat infrastructure to MLIR programs as a runtime
// library. APFloat is a software implementation of floating point arithmetics.
//
// On the MLIR side, floating-point values must be bitcasted to 64-bit integers
// before calling a runtime function. If a floating-point type has less than
// 64 bits, it must be zero-extended to 64 bits after bitcasting it to an
// integer.
//
// Runtime functions receive the floating-point operands of the arithmeic
// operation in the form of 64-bit integers, along with the APFloat semantics
// in the form of a 32-bit integer, which will be interpreted as an
// APFloatBase::Semantics enum value.
//
#include "vm/core/ADT/APFloat.h"
#include "vm/core/ADT/APSInt.h"
#include "vm/core/Support/Debug.h"

#ifdef _WIN32
#ifndef MLIR_APFLOAT_WRAPPERS_EXPORT
#ifdef mlir_apfloat_wrappers_EXPORTS
// We are building this library
#define MLIR_APFLOAT_WRAPPERS_EXPORT __declspec(dllexport)
#else
// We are using this library
#define MLIR_APFLOAT_WRAPPERS_EXPORT __declspec(dllimport)
#endif // mlir_apfloat_wrappers_EXPORTS
#endif // MLIR_APFLOAT_WRAPPERS_EXPORT
#else
// Non-windows: use visibility attributes.
#define MLIR_APFLOAT_WRAPPERS_EXPORT __attribute__((visibility("default")))
#endif // _WIN32

/// Binary operations without rounding mode.
#define APFLOAT_BINARY_OP(OP)                                                  \
  MLIR_APFLOAT_WRAPPERS_EXPORT int64_t _mlir_apfloat_##OP(                     \
      int32_t semantics, uint64_t a, uint64_t b) {                             \
    const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(        \
        static_cast<toolchain::APFloatBase::Semantics>(semantics));                 \
    unsigned bitWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);           \
    toolchain::APFloat lhs(sem, toolchain::APInt(bitWidth, a));                          \
    toolchain::APFloat rhs(sem, toolchain::APInt(bitWidth, b));                          \
    lhs.OP(rhs);                                                               \
    return lhs.bitcastToAPInt().getZExtValue();                                \
  }

/// Binary operations with rounding mode.
#define APFLOAT_BINARY_OP_ROUNDING_MODE(OP, ROUNDING_MODE)                     \
  MLIR_APFLOAT_WRAPPERS_EXPORT uint64_t _mlir_apfloat_##OP(                    \
      int32_t semantics, uint64_t a, uint64_t b) {                             \
    const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(        \
        static_cast<toolchain::APFloatBase::Semantics>(semantics));                 \
    unsigned bitWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);           \
    toolchain::APFloat lhs(sem, toolchain::APInt(bitWidth, a));                          \
    toolchain::APFloat rhs(sem, toolchain::APInt(bitWidth, b));                          \
    lhs.OP(rhs, ROUNDING_MODE);                                                \
    return lhs.bitcastToAPInt().getZExtValue();                                \
  }

extern "C" {

#define BIN_OPS_WITH_ROUNDING(X)                                               \
  X(add, toolchain::RoundingMode::NearestTiesToEven)                                \
  X(subtract, toolchain::RoundingMode::NearestTiesToEven)                           \
  X(multiply, toolchain::RoundingMode::NearestTiesToEven)                           \
  X(divide, toolchain::RoundingMode::NearestTiesToEven)

BIN_OPS_WITH_ROUNDING(APFLOAT_BINARY_OP_ROUNDING_MODE)
#undef BIN_OPS_WITH_ROUNDING
#undef APFLOAT_BINARY_OP_ROUNDING_MODE

APFLOAT_BINARY_OP(remainder)

#undef APFLOAT_BINARY_OP

MLIR_APFLOAT_WRAPPERS_EXPORT void printApFloat(int32_t semantics, uint64_t a) {
  const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(semantics));
  unsigned bitWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);
  toolchain::APFloat x(sem, toolchain::APInt(bitWidth, a));
  double d = x.convertToDouble();
  fprintf(stdout, "%lg", d);
}

MLIR_APFLOAT_WRAPPERS_EXPORT uint64_t
_mlir_apfloat_convert(int32_t inSemantics, int32_t outSemantics, uint64_t a) {
  const toolchain::fltSemantics &inSem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(inSemantics));
  const toolchain::fltSemantics &outSem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(outSemantics));
  unsigned bitWidthIn = toolchain::APFloatBase::semanticsSizeInBits(inSem);
  toolchain::APFloat val(inSem, toolchain::APInt(bitWidthIn, a));
  // TODO: Custom rounding modes are not supported yet.
  bool losesInfo;
  val.convert(outSem, toolchain::RoundingMode::NearestTiesToEven, &losesInfo);
  toolchain::APInt result = val.bitcastToAPInt();
  return result.getZExtValue();
}

MLIR_APFLOAT_WRAPPERS_EXPORT uint64_t _mlir_apfloat_convert_to_int(
    int32_t semantics, int32_t resultWidth, bool isUnsigned, uint64_t a) {
  const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(semantics));
  unsigned inputWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);
  toolchain::APFloat val(sem, toolchain::APInt(inputWidth, a));
  toolchain::APSInt result(resultWidth, isUnsigned);
  bool isExact;
  // TODO: Custom rounding modes are not supported yet.
  val.convertToInteger(result, toolchain::RoundingMode::NearestTiesToEven, &isExact);
  // This function always returns uint64_t, regardless of the desired result
  // width. It does not matter whether we zero-extend or sign-extend the APSInt
  // to 64 bits because the generated IR in arith-to-apfloat will truncate the
  // result to the desired result width.
  return result.getZExtValue();
}

MLIR_APFLOAT_WRAPPERS_EXPORT uint64_t _mlir_apfloat_convert_from_int(
    int32_t semantics, int32_t inputWidth, bool isUnsigned, uint64_t a) {
  toolchain::APInt val(inputWidth, a, /*isSigned=*/!isUnsigned);
  const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(semantics));
  toolchain::APFloat result(sem);
  // TODO: Custom rounding modes are not supported yet.
  result.convertFromAPInt(val, /*IsSigned=*/!isUnsigned,
                          toolchain::RoundingMode::NearestTiesToEven);
  return result.bitcastToAPInt().getZExtValue();
}

MLIR_APFLOAT_WRAPPERS_EXPORT int8_t _mlir_apfloat_compare(int32_t semantics,
                                                          uint64_t a,
                                                          uint64_t b) {
  const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(semantics));
  unsigned bitWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);
  toolchain::APFloat x(sem, toolchain::APInt(bitWidth, a));
  toolchain::APFloat y(sem, toolchain::APInt(bitWidth, b));
  return static_cast<int8_t>(x.compare(y));
}

MLIR_APFLOAT_WRAPPERS_EXPORT uint64_t _mlir_apfloat_neg(int32_t semantics,
                                                        uint64_t a) {
  const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(semantics));
  unsigned bitWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);
  toolchain::APFloat x(sem, toolchain::APInt(bitWidth, a));
  x.changeSign();
  return x.bitcastToAPInt().getZExtValue();
}

MLIR_APFLOAT_WRAPPERS_EXPORT uint64_t _mlir_apfloat_abs(int32_t semantics,
                                                        uint64_t a) {
  const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(semantics));
  unsigned bitWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);
  toolchain::APFloat x(sem, toolchain::APInt(bitWidth, a));
  return abs(x).bitcastToAPInt().getZExtValue();
}

MLIR_APFLOAT_WRAPPERS_EXPORT bool _mlir_apfloat_isfinite(int32_t semantics,
                                                         uint64_t a) {
  const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(semantics));
  unsigned bitWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);
  toolchain::APFloat x(sem, toolchain::APInt(bitWidth, a));
  return x.isFinite();
}

MLIR_APFLOAT_WRAPPERS_EXPORT bool _mlir_apfloat_isinfinite(int32_t semantics,
                                                           uint64_t a) {
  const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(semantics));
  unsigned bitWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);
  toolchain::APFloat x(sem, toolchain::APInt(bitWidth, a));
  return x.isInfinity();
}

MLIR_APFLOAT_WRAPPERS_EXPORT bool _mlir_apfloat_isnormal(int32_t semantics,
                                                         uint64_t a) {
  const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(semantics));
  unsigned bitWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);
  toolchain::APFloat x(sem, toolchain::APInt(bitWidth, a));
  return x.isNormal();
}

MLIR_APFLOAT_WRAPPERS_EXPORT bool _mlir_apfloat_isnan(int32_t semantics,
                                                      uint64_t a) {
  const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(semantics));
  unsigned bitWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);
  toolchain::APFloat x(sem, toolchain::APInt(bitWidth, a));
  return x.isNaN();
}

MLIR_APFLOAT_WRAPPERS_EXPORT uint64_t
_mlir_apfloat_fused_multiply_add(int32_t semantics, uint64_t operand,
                                 uint64_t multiplicand, uint64_t addend) {
  const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(
      static_cast<toolchain::APFloatBase::Semantics>(semantics));
  unsigned bitWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);
  toolchain::APFloat operand_(sem, toolchain::APInt(bitWidth, operand));
  toolchain::APFloat multiplicand_(sem, toolchain::APInt(bitWidth, multiplicand));
  toolchain::APFloat addend_(sem, toolchain::APInt(bitWidth, addend));
  toolchain::detail::opStatus stat = operand_.fusedMultiplyAdd(
      multiplicand_, addend_, toolchain::RoundingMode::NearestTiesToEven);
  assert(stat == toolchain::APFloatBase::opOK &&
         "expected fusedMultiplyAdd status to be OK");
  (void)stat;
  return operand_.bitcastToAPInt().getZExtValue();
}

/// Min/max operations.
#define APFLOAT_MIN_MAX_OP(OP)                                                 \
  MLIR_APFLOAT_WRAPPERS_EXPORT uint64_t _mlir_apfloat_##OP(                    \
      int32_t semantics, uint64_t a, uint64_t b) {                             \
    const toolchain::fltSemantics &sem = toolchain::APFloatBase::EnumToSemantics(        \
        static_cast<toolchain::APFloatBase::Semantics>(semantics));                 \
    unsigned bitWidth = toolchain::APFloatBase::semanticsSizeInBits(sem);           \
    toolchain::APFloat lhs(sem, toolchain::APInt(bitWidth, a));                          \
    toolchain::APFloat rhs(sem, toolchain::APInt(bitWidth, b));                          \
    toolchain::APFloat result = toolchain::OP(lhs, rhs);                                 \
    return result.bitcastToAPInt().getZExtValue();                             \
  }

APFLOAT_MIN_MAX_OP(minimum)
APFLOAT_MIN_MAX_OP(maximum)
APFLOAT_MIN_MAX_OP(minnum)
APFLOAT_MIN_MAX_OP(maxnum)

#undef APFLOAT_MIN_MAX_OP
}
