//===--- PrettyStackTrace.cpp - Generic PrettyStackTraceEntries -----------===//
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
//
//  This file implements several PrettyStackTraceEntries that probably
//  ought to be in LLVM.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/PrettyStackTrace.h"
#include "language/Basic/QuotedString.h"
#include "language/Basic/Version.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

void PrettyStackTraceStringAction::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << ' ' << QuotedString(TheString) << '\n';
}

void PrettyStackTraceFileContents::print(toolchain::raw_ostream &out) const {
  out << "Contents of " << Buffer.getBufferIdentifier() << ":\n---\n"
      << Buffer.getBuffer();
  if (!Buffer.getBuffer().ends_with("\n"))
    out << '\n';
  out << "---\n";
}

void PrettyStackTraceCodiraVersion::print(toolchain::raw_ostream &out) const {
  out << version::getCodiraFullVersion() << '\n';
}
