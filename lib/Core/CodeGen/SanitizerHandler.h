//===-- SanitizerHandler.h - Definition of sanitizer handlers ---*- C++ -*-===//
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
// This is the internal per-function state used for toolchain translation.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_SANITIZER_HANDLER_H
#define LANGUAGE_CORE_LIB_CODEGEN_SANITIZER_HANDLER_H

#define LIST_SANITIZER_CHECKS                                                  \
  SANITIZER_CHECK(AddOverflow, add_overflow, 0, "Integer addition overflowed") \
  SANITIZER_CHECK(BuiltinUnreachable, builtin_unreachable, 0,                  \
                  "_builtin_unreachable(), execution reached an unreachable "  \
                  "program point")                                             \
  SANITIZER_CHECK(CFICheckFail, cfi_check_fail, 0,                             \
                  "Control flow integrity check failed")                       \
  SANITIZER_CHECK(DivremOverflow, divrem_overflow, 0,                          \
                  "Integer divide or remainder overflowed")                    \
  SANITIZER_CHECK(DynamicTypeCacheMiss, dynamic_type_cache_miss, 0,            \
                  "Dynamic type cache miss, member call made on an object "    \
                  "whose dynamic type differs from the expected type")         \
  SANITIZER_CHECK(FloatCastOverflow, float_cast_overflow, 0,                   \
                  "Floating-point to integer conversion overflowed")           \
  SANITIZER_CHECK(FunctionTypeMismatch, function_type_mismatch, 0,             \
                  "Function called with mismatched signature")                 \
  SANITIZER_CHECK(ImplicitConversion, implicit_conversion, 0,                  \
                  "Implicit integer conversion overflowed or lost data")       \
  SANITIZER_CHECK(InvalidBuiltin, invalid_builtin, 0,                          \
                  "Invalid use of builtin function")                           \
  SANITIZER_CHECK(InvalidObjCCast, invalid_objc_cast, 0,                       \
                  "Invalid Objective-C cast")                                  \
  SANITIZER_CHECK(LoadInvalidValue, load_invalid_value, 0,                     \
                  "Loaded an invalid or uninitialized value for the type")     \
  SANITIZER_CHECK(MissingReturn, missing_return, 0,                            \
                  "Execution reached the end of a value-returning function "   \
                  "without returning a value")                                 \
  SANITIZER_CHECK(MulOverflow, mul_overflow, 0,                                \
                  "Integer multiplication overflowed")                         \
  SANITIZER_CHECK(NegateOverflow, negate_overflow, 0,                          \
                  "Integer negation overflowed")                               \
  SANITIZER_CHECK(                                                             \
      NullabilityArg, nullability_arg, 0,                                      \
      "Passing null as an argument which is annotated with _Nonnull")          \
  SANITIZER_CHECK(NullabilityReturn, nullability_return, 1,                    \
                  "Returning null from a function with a return type "         \
                  "annotated with _Nonnull")                                   \
  SANITIZER_CHECK(NonnullArg, nonnull_arg, 0,                                  \
                  "Passing null pointer as an argument which is declared to "  \
                  "never be null")                                             \
  SANITIZER_CHECK(NonnullReturn, nonnull_return, 1,                            \
                  "Returning null pointer from a function which is declared "  \
                  "to never return null")                                      \
  SANITIZER_CHECK(OutOfBounds, out_of_bounds, 0, "Array index out of bounds")  \
  SANITIZER_CHECK(PointerOverflow, pointer_overflow, 0,                        \
                  "Pointer arithmetic overflowed bounds")                      \
  SANITIZER_CHECK(ShiftOutOfBounds, shift_out_of_bounds, 0,                    \
                  "Shift exponent is too large for the type")                  \
  SANITIZER_CHECK(SubOverflow, sub_overflow, 0,                                \
                  "Integer subtraction overflowed")                            \
  SANITIZER_CHECK(TypeMismatch, type_mismatch, 1,                              \
                  "Type mismatch in operation")                                \
  SANITIZER_CHECK(AlignmentAssumption, alignment_assumption, 0,                \
                  "Alignment assumption violated")                             \
  SANITIZER_CHECK(                                                             \
      VLABoundNotPositive, vla_bound_not_positive, 0,                          \
      "Variable length array bound evaluates to non-positive value")           \
  SANITIZER_CHECK(BoundsSafety, bounds_safety, 0,                              \
                  "") // BoundsSafety Msg is empty because it is not considered
                      // part of UBSan; therefore, no trap reason is emitted for
                      // this case.

enum SanitizerHandler {
#define SANITIZER_CHECK(Enum, Name, Version, Msg) Enum,
  LIST_SANITIZER_CHECKS
#undef SANITIZER_CHECK
};

#endif // LANGUAGE_CORE_LIB_CODEGEN_SANITIZER_HANDLER_H
