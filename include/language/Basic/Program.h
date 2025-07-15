//===--- Program.h ----------------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_PROGRAM_H
#define LANGUAGE_BASIC_PROGRAM_H

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/ErrorOr.h"
#include "toolchain/Support/Program.h"
#include <optional>

namespace language {

/// This function executes the program using the arguments provided,
/// preferring to reexecute the current process, if supported.
///
/// \param Program Path of the program to be executed
/// \param args A vector of strings that are passed to the program. The first
/// element should be the name of the program. The list *must* be terminated by
/// a null char * entry.
/// \param env An optional vector of strings to use for the program's
/// environment. If not provided, the current program's environment will be
/// used.
///
/// \returns Typically, this function will not return, as the current process
/// will no longer exist, or it will call exit() if the program was successfully
/// executed. In the event of an error, this function will return a negative
/// value indicating a failure to execute.
int ExecuteInPlace(const char *Program, const char **args,
                   const char **env = nullptr);

struct ChildProcessInfo {
  toolchain::sys::ProcessInfo ProcessInfo;
  int Write;
  int Read;

  ChildProcessInfo(toolchain::sys::ProcessInfo ProcessInfo, int Write, int Read)
      : ProcessInfo(ProcessInfo), Write(Write), Read(Read) {}
};

/// This function executes the program using the argument provided.
/// Establish pipes between the current process and the spawned process.
/// Returned a \c ChildProcessInfo where WriteFileDescriptor is piped to
/// child's STDIN, and ReadFileDescriptor is piped from child's STDOUT.
///
/// \param program Path of the program to be executed
/// \param args An array of strings that are passed to the program. The first
/// element should be the name of the program.
/// \param env An optional array of 'key=value 'strings to use for the program's
/// environment.
toolchain::ErrorOr<language::ChildProcessInfo> ExecuteWithPipe(
    toolchain::StringRef program, toolchain::ArrayRef<toolchain::StringRef> args,
    std::optional<toolchain::ArrayRef<toolchain::StringRef>> env = std::nullopt);

} // end namespace language

#endif // LANGUAGE_BASIC_PROGRAM_H
