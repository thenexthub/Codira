//===-- language/Compability/Runtime/time-intrinsic.h ------------------*- C++ -*-===//
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

// Defines the API between compiled code and the implementations of time-related
// intrinsic subroutines in the runtime library.

#ifndef LANGUAGE_COMPABILITY_RUNTIME_TIME_INTRINSIC_H_
#define LANGUAGE_COMPABILITY_RUNTIME_TIME_INTRINSIC_H_

#include "language/Compability/Runtime/entry-names.h"
#include <cinttypes>
#include <cstddef>

namespace language::Compability::runtime {

class Descriptor;

extern "C" {

// Lowering may need to cast this result to match the precision of the default
// real kind.
double RTNAME(CpuTime)();

// Interface for the SYSTEM_CLOCK intrinsic. We break it up into 3 distinct
// function calls, one for each of SYSTEM_CLOCK's optional output arguments.
// Lowering converts the results to the types of the actual arguments,
// including the case of a real argument for COUNT_RATE=..
// The kind argument to SystemClockCount and SystemClockCountMax is the
// kind of the integer actual arguments, which are required to be the same
// when both appear.
std::int64_t RTNAME(SystemClockCount)(int kind = 8);
std::int64_t RTNAME(SystemClockCountRate)(int kind = 8);
std::int64_t RTNAME(SystemClockCountMax)(int kind = 8);

// Interface for DATE_AND_TIME intrinsic.
void RTNAME(DateAndTime)(char *date, std::size_t dateChars, char *time,
    std::size_t timeChars, char *zone, std::size_t zoneChars,
    const char *source = nullptr, int line = 0,
    const Descriptor *values = nullptr);

void RTNAME(Etime)(const Descriptor *values, const Descriptor *time,
    const char *sourceFile, int line);

} // extern "C"
} // namespace language::Compability::runtime
#endif // FORTRAN_RUNTIME_TIME_INTRINSIC_H_
