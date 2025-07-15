//===--- CodeCompletionResultPrinter.h --------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_IDE_CODECOMPLETIONRESULTPRINTER_H
#define LANGUAGE_IDE_CODECOMPLETIONRESULTPRINTER_H

#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/Support/Allocator.h"

namespace language {

class NullTerminatedStringRef;

namespace ide {

class CodeCompletionResult;
class CodeCompletionString;

void printCodeCompletionResultDescription(const CodeCompletionResult &Result,
                                          toolchain::raw_ostream &OS,
                                          bool leadingPunctuation);

void printCodeCompletionResultDescriptionAnnotated(
    const CodeCompletionResult &Result, toolchain::raw_ostream &OS,
    bool leadingPunctuation);

void printCodeCompletionResultTypeName(
    const CodeCompletionResult &Result, toolchain::raw_ostream &OS);

void printCodeCompletionResultTypeNameAnnotated(
    const CodeCompletionResult &Result, toolchain::raw_ostream &OS);

void printCodeCompletionResultSourceText(
    const CodeCompletionResult &Result, toolchain::raw_ostream &OS);

/// Print 'FilterName' from \p str into memory managed by \p Allocator and
/// return it as \c NullTerminatedStringRef .
NullTerminatedStringRef
getCodeCompletionResultFilterName(const CodeCompletionString *Str,
                                  toolchain::BumpPtrAllocator &Allocator);

} // namespace ide
} // namespace language

#endif
