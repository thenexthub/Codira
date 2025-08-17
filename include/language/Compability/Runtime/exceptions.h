//===-- language/Compability/Runtime/exceptions.h ----------------*- C++ -*-===//
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

// Support for floating point exceptions and related floating point environment
// functionality.

#ifndef LANGUAGE_COMPABILITY_RUNTIME_EXCEPTIONS_H_
#define LANGUAGE_COMPABILITY_RUNTIME_EXCEPTIONS_H_

#include "language/Compability/Runtime/entry-names.h"
#include <cinttypes>
#include <cstddef>

namespace language::Compability::runtime {

class Descriptor;

extern "C" {

// Map a set of IEEE_FLAG_TYPE exception values to a libm fenv.h excepts value.
// This mapping is done at runtime to support cross compilation.
std::uint32_t RTNAME(MapException)(std::uint32_t excepts);

// Exception processing functions that call the corresponding libm functions,
// and also include support for denormal exceptions where available.
void RTNAME(feclearexcept)(std::uint32_t excepts);
void RTNAME(feraiseexcept)(std::uint32_t excepts);
std::uint32_t RTNAME(fetestexcept)(std::uint32_t excepts);
void RTNAME(fedisableexcept)(std::uint32_t excepts);
void RTNAME(feenableexcept)(std::uint32_t excepts);
std::uint32_t RTNAME(fegetexcept)(void);

// Check if the processor has the ability to control whether to halt
// or continue exeuction when a given exception is raised.
bool RTNAME(SupportHalting)(uint32_t except);

// Get and set the ieee underflow mode if supported; otherwise nops.
bool RTNAME(GetUnderflowMode)(void);
void RTNAME(SetUnderflowMode)(bool flag);

// Get the byte size of ieee_modes_type and ieee_status_type data.
std::size_t RTNAME(GetModesTypeSize)(void);
std::size_t RTNAME(GetStatusTypeSize)(void);

} // extern "C"
} // namespace language::Compability::runtime
#endif // FORTRAN_RUNTIME_EXCEPTIONS_H_
