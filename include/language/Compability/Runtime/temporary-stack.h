//===-- language/Compability/Runtime/temporary-stack.h -----------------*- C++ -*-===//
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
// Runtime functions for storing a dynamically resizable number of temporaries.
// For use in HLFIR lowering.
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_RUNTIME_TEMPORARY_STACK_H_
#define LANGUAGE_COMPABILITY_RUNTIME_TEMPORARY_STACK_H_

#include "language/Compability/Runtime/entry-names.h"
#include <stdint.h>

namespace language::Compability::runtime {
class Descriptor;
extern "C" {

// Stores both the descriptor and a copy of the value in a dynamically resizable
// data structure identified by opaquePtr. All value stacks must be destroyed
// at the end of their lifetime and not used afterwards.
// Popped descriptors point to the copy of the value, not the original address
// of the value. This copy is dynamically allocated, it is up to the caller to
// free the value pointed to by the box. The copy operation is a simple memcpy.
// The sourceFile and line number used when creating the stack are shared for
// all operations.
// Opaque pointers returned from these are incompatible with those returned by
// the flavours for storing descriptors.
[[nodiscard]] void *RTNAME(CreateValueStack)(
    const char *sourceFile = nullptr, int line = 0);
void RTNAME(PushValue)(void *opaquePtr, const Descriptor &value);
// Note: retValue should be large enough to hold the right number of dimensions,
// and the optional descriptor addendum
void RTNAME(PopValue)(void *opaquePtr, Descriptor &retValue);
// Return the i'th element into retValue (which must be the right size). An
// exact copy of this descriptor remains in this storage so this one should not
// be deallocated
void RTNAME(ValueAt)(void *opaquePtr, uint64_t i, Descriptor &retValue);
void RTNAME(DestroyValueStack)(void *opaquePtr);

// Stores descriptors value in a dynamically resizable data structure identified
// by opaquePtr. All descriptor stacks must be destroyed at the end of their
// lifetime and not used afterwards.
// Popped descriptors are identical to those which were pushed.
// The sourceFile and line number used when creating the stack are shared for
// all operations.
// Opaque pointers returned from these are incompatible with those returned by
// the flavours for storing both descriptors and values.
[[nodiscard]] void *RTNAME(CreateDescriptorStack)(
    const char *sourceFile = nullptr, int line = 0);
void RTNAME(PushDescriptor)(void *opaquePtr, const Descriptor &value);
// Note: retValue should be large enough to hold the right number of dimensions,
// and the optional descriptor addendum
void RTNAME(PopDescriptor)(void *opaquePtr, Descriptor &retValue);
// Return the i'th element into retValue (which must be the right size). An
// exact copy of this descriptor remains in this storage so this one should not
// be deallocated
void RTNAME(DescriptorAt)(void *opaquePtr, uint64_t i, Descriptor &retValue);
void RTNAME(DestroyDescriptorStack)(void *opaquePtr);

} // extern "C"
} // namespace language::Compability::runtime

#endif // FORTRAN_RUNTIME_TEMPORARY_STACK_H_
