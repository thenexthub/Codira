//===--- PrettyStackTrace.h - Generic stack-trace prettifiers ---*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_PRETTYSTACKTRACE_H
#define LANGUAGE_BASIC_PRETTYSTACKTRACE_H

#include "toolchain/Support/PrettyStackTrace.h"
#include "toolchain/ADT/StringRef.h"

namespace toolchain {
  class MemoryBuffer;
}

namespace language {

/// A PrettyStackTraceEntry for performing an action involving a StringRef.
///
/// The message is:
///   While <action> "<string>"\n
class PrettyStackTraceStringAction : public toolchain::PrettyStackTraceEntry {
  const char *Action;
  toolchain::StringRef TheString;
public:
  PrettyStackTraceStringAction(const char *action, toolchain::StringRef string)
    : Action(action), TheString(string) {}
  void print(toolchain::raw_ostream &OS) const override;
};

/// A PrettyStackTraceEntry to dump the contents of a file.
class PrettyStackTraceFileContents : public toolchain::PrettyStackTraceEntry {
  const toolchain::MemoryBuffer &Buffer;
public:
  explicit PrettyStackTraceFileContents(const toolchain::MemoryBuffer &buffer)
    : Buffer(buffer) {}
  void print(toolchain::raw_ostream &OS) const override;
};

/// A PrettyStackTraceEntry to print the version of the compiler.
class PrettyStackTraceCodiraVersion : public toolchain::PrettyStackTraceEntry {
public:
  void print(toolchain::raw_ostream &OS) const override;
};

} // end namespace language

#endif // LANGUAGE_BASIC_PRETTYSTACKTRACE_H
