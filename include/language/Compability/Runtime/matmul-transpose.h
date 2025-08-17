//===-- language/Compability/Runtime/matmul-transpose.h ----------------*- C++ -*-===//
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

// API for optimised MATMUL(TRANSPOSE(a), b)

#ifndef LANGUAGE_COMPABILITY_RUNTIME_MATMUL_TRANSPOSE_H_
#define LANGUAGE_COMPABILITY_RUNTIME_MATMUL_TRANSPOSE_H_
#include "language/Compability/Common/float128.h"
#include "language/Compability/Common/uint128.h"
#include "language/Compability/Runtime/entry-names.h"
namespace language::Compability::runtime {
class Descriptor;
extern "C" {

// The most general MATMUL(TRANSPOSE()).  All type and shape information is
// taken from the arguments' descriptors, and the result is dynamically
// allocated.
void RTDECL(MatmulTranspose)(Descriptor &, const Descriptor &,
    const Descriptor &, const char *sourceFile = nullptr, int line = 0);

// A non-allocating variant; the result's descriptor must be established
// and have a valid base address.
void RTDECL(MatmulTransposeDirect)(const Descriptor &, const Descriptor &,
    const Descriptor &, const char *sourceFile = nullptr, int line = 0);

// MATMUL(TRANSPOSE()) versions specialized by the categories of the operand
// types. The KIND and shape information is taken from the argument's
// descriptors.
#define MATMUL_INSTANCE(XCAT, XKIND, YCAT, YKIND) \
  void RTDECL(MatmulTranspose##XCAT##XKIND##YCAT##YKIND)(Descriptor & result, \
      const Descriptor &x, const Descriptor &y, const char *sourceFile, \
      int line);
#define MATMUL_DIRECT_INSTANCE(XCAT, XKIND, YCAT, YKIND) \
  void RTDECL(MatmulTransposeDirect##XCAT##XKIND##YCAT##YKIND)( \
      Descriptor & result, const Descriptor &x, const Descriptor &y, \
      const char *sourceFile, int line);

#define MATMUL_FORCE_ALL_TYPES 0

#include "matmul-instances.inc"

} // extern "C"
} // namespace language::Compability::runtime
#endif // FORTRAN_RUNTIME_MATMUL_TRANSPOSE_H_
