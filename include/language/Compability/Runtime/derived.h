//===-- language/Compability-rt/runtime/derived.h ----------------------*- C++ -*-===//
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

// Internal runtime utilities for derived type operations.

#ifndef FLANG_RT_RUNTIME_DERIVED_H_
#define FLANG_RT_RUNTIME_DERIVED_H_

#include "language/Compability/Common/api-attrs.h"

namespace language::Compability::runtime::typeInfo {
class DerivedType;
}

namespace language::Compability::runtime {
class Descriptor;
class Terminator;

// Perform default component initialization, allocate automatic components.
// Returns a STAT= code (0 when all's well).
RT_API_ATTRS int Initialize(const Descriptor &, const typeInfo::DerivedType &,
    Terminator &, bool hasStat = false, const Descriptor *errMsg = nullptr);

// Initializes an object clone from the original object.
// Each allocatable member of the clone is allocated with the same bounds as
// in the original object, if it is also allocated in it.
// Returns a STAT= code (0 when all's well).
RT_API_ATTRS int InitializeClone(const Descriptor &, const Descriptor &,
    const typeInfo::DerivedType &, Terminator &, bool hasStat = false,
    const Descriptor *errMsg = nullptr);

// Call FINAL subroutines, if any
RT_API_ATTRS void Finalize(
    const Descriptor &, const typeInfo::DerivedType &derived, Terminator *);

// Call FINAL subroutines, deallocate allocatable & automatic components.
// Does not deallocate the original descriptor.
RT_API_ATTRS void Destroy(const Descriptor &, bool finalize,
    const typeInfo::DerivedType &, Terminator *);

// Return true if the passed descriptor is for a derived type
// entity that has a dynamic (allocatable, automatic) component.
RT_API_ATTRS bool HasDynamicComponent(const Descriptor &);

} // namespace language::Compability::runtime
#endif // FLANG_RT_RUNTIME_DERIVED_H_
