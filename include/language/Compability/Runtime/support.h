//===-- language/Compability/Runtime/support.h -------------------------*- C++ -*-===//
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

// Defines APIs for runtime support code for lowering.
#ifndef LANGUAGE_COMPABILITY_RUNTIME_SUPPORT_H_
#define LANGUAGE_COMPABILITY_RUNTIME_SUPPORT_H_

#include "language/Compability/Common/ISO_Fortran_binding_wrapper.h"
#include "language/Compability/Runtime/entry-names.h"
#include <cstddef>
#include <cstdint>

namespace language::Compability::runtime {

class Descriptor;

namespace typeInfo {
class DerivedType;
}

enum class LowerBoundModifier : int {
  Preserve = 0,
  SetToOnes = 1,
  SetToZeroes = 2
};

extern "C" {

// Predicate: is the storage described by a Descriptor contiguous in memory?
bool RTDECL(IsContiguous)(const Descriptor &);

// Predicate: is the storage described by a Descriptor contiguous in memory
// up to the given dimension?
bool RTDECL(IsContiguousUpTo)(const Descriptor &, int);

// Predicate: is this descriptor describing an assumed-size array?
bool RTDECL(IsAssumedSize)(const Descriptor &);

// Copy "from" descriptor into "to" descriptor and update "to" dynamic type,
// CFI_attribute, and lower bounds according to the other arguments.
// "newDynamicType" may be a null pointer in which case "to" dynamic type is the
// one of "from".
void RTDECL(CopyAndUpdateDescriptor)(Descriptor &to, const Descriptor &from,
    const typeInfo::DerivedType *newDynamicType,
    ISO::CFI_attribute_t newAttribute, enum LowerBoundModifier newLowerBounds);

} // extern "C"
} // namespace language::Compability::runtime
#endif // FORTRAN_RUNTIME_SUPPORT_H_
