//===-- language/Compability/Runtime/assign.h --------------------------*- C++ -*-===//
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

// External APIs for data assignment (both intrinsic assignment and TBP defined
// generic ASSIGNMENT(=)).  Should be called by lowering for any assignments
// possibly needing special handling.  Intrinsic assignment to non-allocatable
// variables whose types are intrinsic need not come through here (though they
// may do so).  Assignments to allocatables, and assignments whose types may be
// polymorphic or are monomorphic and of derived types with finalization,
// allocatable components, or components with type-bound defined assignments, in
// the original type or the types of its non-pointer components (recursively)
// must arrive here.
//
// Non-type-bound generic INTERFACE ASSIGNMENT(=) is resolved in semantics and
// need not be handled here in the runtime apart from derived type components;
// ditto for type conversions on intrinsic assignments.

#ifndef LANGUAGE_COMPABILITY_RUNTIME_ASSIGN_H_
#define LANGUAGE_COMPABILITY_RUNTIME_ASSIGN_H_

#include "language/Compability/Runtime/entry-names.h"
#include "language/Compability/Runtime/freestanding-tools.h"

namespace language::Compability::runtime {
class Descriptor;
class Terminator;

enum AssignFlags {
  NoAssignFlags = 0,
  MaybeReallocate = 1 << 0,
  NeedFinalization = 1 << 1,
  CanBeDefinedAssignment = 1 << 2,
  ComponentCanBeDefinedAssignment = 1 << 3,
  ExplicitLengthCharacterLHS = 1 << 4,
  PolymorphicLHS = 1 << 5,
  DeallocateLHS = 1 << 6,
  UpdateLHSBounds = 1 << 7,
};

#ifdef RT_DEVICE_COMPILATION
RT_API_ATTRS void Assign(Descriptor &to, const Descriptor &from,
    Terminator &terminator, int flags, MemmoveFct memmoveFct = &MemmoveWrapper);
#else
RT_API_ATTRS void Assign(Descriptor &to, const Descriptor &from,
    Terminator &terminator, int flags,
    MemmoveFct memmoveFct = &language::Compability::runtime::memmove);
#endif

extern "C" {

// API for lowering assignment
void RTDECL(Assign)(Descriptor &to, const Descriptor &from,
    const char *sourceFile = nullptr, int sourceLine = 0);
// This variant has no finalization, defined assignment, or allocatable
// reallocation.
void RTDECL(AssignTemporary)(Descriptor &to, const Descriptor &from,
    const char *sourceFile = nullptr, int sourceLine = 0);

// Establish "temp" descriptor as an allocatable descriptor with the same type,
// rank, and length parameters as "var" and copy "var" to it using
// AssignTemporary.
void RTDECL(CopyInAssign)(Descriptor &temp, const Descriptor &var,
    const char *sourceFile = nullptr, int sourceLine = 0);
// When "var" is provided, copy "temp" to it assuming "var" is already
// initialized. Destroy and deallocate "temp" in all cases.
void RTDECL(CopyOutAssign)(Descriptor *var, Descriptor &temp,
    const char *sourceFile = nullptr, int sourceLine = 0);
// This variant is for assignments to explicit-length CHARACTER left-hand
// sides that might need to handle truncation or blank-fill, and
// must maintain the character length even if an allocatable array
// is reallocated.
void RTDECL(AssignExplicitLengthCharacter)(Descriptor &to,
    const Descriptor &from, const char *sourceFile = nullptr,
    int sourceLine = 0);
// This variant is assignments to whole polymorphic allocatables.
void RTDECL(AssignPolymorphic)(Descriptor &to, const Descriptor &from,
    const char *sourceFile = nullptr, int sourceLine = 0);
} // extern "C"
} // namespace language::Compability::runtime
#endif // FORTRAN_RUNTIME_ASSIGN_H_
