//===-- language/Compability-rt/runtime/assign-impl.h ------------------*- C++ -*-===//
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

#ifndef FLANG_RT_RUNTIME_ASSIGN_IMPL_H_
#define FLANG_RT_RUNTIME_ASSIGN_IMPL_H_

#include "flang/Runtime/freestanding-tools.h"

namespace language::Compability::runtime {
class Descriptor;
class Terminator;

// Assign one object to another via allocate statement from source specifier.
// Note that if allocate object and source expression have the same rank, the
// value of the allocate object becomes the value provided; otherwise the value
// of each element of allocate object becomes the value provided (9.7.1.2(7)).
#ifdef RT_DEVICE_COMPILATION
RT_API_ATTRS void DoFromSourceAssign(Descriptor &, const Descriptor &,
    Terminator &, MemmoveFct memmoveFct = &MemmoveWrapper);
#else
RT_API_ATTRS void DoFromSourceAssign(Descriptor &, const Descriptor &,
    Terminator &, MemmoveFct memmoveFct = &language::Compability::runtime::memmove);
#endif

} // namespace language::Compability::runtime
#endif // FLANG_RT_RUNTIME_ASSIGN_IMPL_H_
