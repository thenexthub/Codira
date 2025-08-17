//===-- language/Compability/Runtime/command.h -------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_RUNTIME_COMMAND_H_
#define LANGUAGE_COMPABILITY_RUNTIME_COMMAND_H_

#include "language/Compability/Runtime/entry-names.h"
#include <cstdint>

#ifdef _MSC_VER
// On Windows* OS GetCurrentProcessId returns DWORD aka uint32_t
typedef std::uint32_t pid_t;
#else
#include "sys/types.h" //pid_t
#endif

namespace language::Compability::runtime {
class Descriptor;

extern "C" {
// 16.9.51 COMMAND_ARGUMENT_COUNT
//
// Lowering may need to cast the result to match the precision of the default
// integer kind.
std::int32_t RTNAME(ArgumentCount)();

// Calls getpid()
pid_t RTNAME(GetPID)();

// 16.9.82 GET_COMMAND
// Try to get the value of the whole command. All of the parameters are
// optional.
// Return a STATUS as described in the standard.
std::int32_t RTNAME(GetCommand)(const Descriptor *command = nullptr,
    const Descriptor *length = nullptr, const Descriptor *errmsg = nullptr,
    const char *sourceFile = nullptr, int line = 0);

// 16.9.83 GET_COMMAND_ARGUMENT
// Try to get the value of the n'th argument.
// Returns a STATUS as described in the standard.
std::int32_t RTNAME(GetCommandArgument)(std::int32_t n,
    const Descriptor *argument = nullptr, const Descriptor *length = nullptr,
    const Descriptor *errmsg = nullptr, const char *sourceFile = nullptr,
    int line = 0);

// 16.9.84 GET_ENVIRONMENT_VARIABLE
// Try to get the value of the environment variable specified by NAME.
// Returns a STATUS as described in the standard.
std::int32_t RTNAME(GetEnvVariable)(const Descriptor &name,
    const Descriptor *value = nullptr, const Descriptor *length = nullptr,
    bool trim_name = true, const Descriptor *errmsg = nullptr,
    const char *sourceFile = nullptr, int line = 0);

// Calls getcwd()
std::int32_t RTNAME(GetCwd)(
    const Descriptor &cwd, const char *sourceFile, int line);

// Calls hostnm()
std::int32_t RTNAME(Hostnm)(
    const Descriptor &res, const char *sourceFile, int line);

std::int32_t RTNAME(PutEnv)(
    const char *str, size_t str_length, const char *sourceFile, int line);

// Calls unlink()
std::int32_t RTNAME(Unlink)(
    const char *path, size_t pathLength, const char *sourceFile, int line);

} // extern "C"

} // namespace language::Compability::runtime

#endif // FORTRAN_RUNTIME_COMMAND_H_
