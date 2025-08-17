//===-- language/Compability/Runtime/stop.h ----------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_RUNTIME_STOP_H_
#define LANGUAGE_COMPABILITY_RUNTIME_STOP_H_

#include "language/Compability/Runtime/c-or-cpp.h"
#include "language/Compability/Runtime/entry-names.h"
#include "language/Compability/Runtime/extensions.h"
#include <stdlib.h>

FORTRAN_EXTERN_C_BEGIN

// Program-initiated image stop
NORETURN RT_API_ATTRS void RTNAME(StopStatement)(
    int code DEFAULT_VALUE(EXIT_SUCCESS), bool isErrorStop DEFAULT_VALUE(false),
    bool quiet DEFAULT_VALUE(false));
NORETURN RT_API_ATTRS void RTNAME(StopStatementText)(const char *, size_t,
    bool isErrorStop DEFAULT_VALUE(false), bool quiet DEFAULT_VALUE(false));
void RTNAME(PauseStatement)(NO_ARGUMENTS);
void RTNAME(PauseStatementInt)(int);
void RTNAME(PauseStatementText)(const char *, size_t);
NORETURN void RTNAME(FailImageStatement)(NO_ARGUMENTS);
NORETURN void RTNAME(ProgramEndStatement)(NO_ARGUMENTS);

// Extensions
NORETURN void RTNAME(Exit)(int status DEFAULT_VALUE(EXIT_SUCCESS));
NORETURN void RTNAME(Abort)(NO_ARGUMENTS);
void FORTRAN_PROCEDURE_NAME(backtrace)(NO_ARGUMENTS);

// Crash with an error message when the program dynamically violates a Fortran
// constraint.
NORETURN RT_API_ATTRS void RTNAME(ReportFatalUserError)(
    const char *message, const char *source, int line);

FORTRAN_EXTERN_C_END

#endif // FORTRAN_RUNTIME_STOP_H_
