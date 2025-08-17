//===-- language/Compability/Runtime/derived-api.h ---------------------*- C++ -*-===//
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

// API for lowering to use for operations on derived type objects.
// Initialiaztion and finalization are implied for pointer and allocatable
// ALLOCATE()/DEALLOCATE() respectively, so these APIs should be used only for
// local variables.  Whole allocatable assignment should use AllocatableAssign()
// instead of this Assign().

#ifndef LANGUAGE_COMPABILITY_RUNTIME_DERIVED_API_H_
#define LANGUAGE_COMPABILITY_RUNTIME_DERIVED_API_H_

#include "language/Compability/Runtime/entry-names.h"

namespace language::Compability::runtime {
class Descriptor;

namespace typeInfo {
class DerivedType;
}

extern "C" {

// Initializes and allocates an object's components, if it has a derived type
// with any default component initialization or automatic components.
// The descriptor must be initialized and non-null.
void RTDECL(Initialize)(
    const Descriptor &, const char *sourceFile = nullptr, int sourceLine = 0);

// Initializes an object clone from the original object.
// Each allocatable member of the clone is allocated with the same bounds as
// in the original object, if it is also allocated in it.
// The descriptor must be initialized and non-null.
void RTDECL(InitializeClone)(const Descriptor &, const Descriptor &,
    const char *sourceFile = nullptr, int sourceLine = 0);

// Finalizes an object and its components.  Deallocates any
// allocatable/automatic components.  Does not deallocate the descriptor's
// storage.
void RTDECL(Destroy)(const Descriptor &);

// Finalizes the object and its components.
void RTDECL(Finalize)(
    const Descriptor &, const char *sourceFile = nullptr, int sourceLine = 0);

/// Deallocates any allocatable/automatic components.
/// Does not deallocate the descriptor's storage.
/// Does not perform any finalization.
void RTDECL(DestroyWithoutFinalization)(const Descriptor &);

// Intrinsic or defined assignment, with scalar expansion but not type
// conversion.
void RTDECL(Assign)(const Descriptor &, const Descriptor &,
    const char *sourceFile = nullptr, int sourceLine = 0);

// Perform the test of the CLASS IS type guard statement of the SELECT TYPE
// construct.
bool RTDECL(ClassIs)(const Descriptor &, const typeInfo::DerivedType &);

// Perform the test of the SAME_TYPE_AS intrinsic.
bool RTDECL(SameTypeAs)(const Descriptor &, const Descriptor &);

// Perform the test of the EXTENDS_TYPE_OF intrinsic.
bool RTDECL(ExtendsTypeOf)(const Descriptor &, const Descriptor &);

} // extern "C"
} // namespace language::Compability::runtime
#endif // FORTRAN_RUNTIME_DERIVED_API_H_
