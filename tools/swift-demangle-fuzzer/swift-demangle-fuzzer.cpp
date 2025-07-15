//===--- language-demangle-fuzzer.cpp - Codira fuzzer -------------------------===//
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
// This program tries to fuzz the demangler shipped as part of the language
// compiler.
// For this to work you need to pass --enable-sanitizer-coverage to build-script
// otherwise the fuzzer doesn't have coverage information to make progress
// (making the whole fuzzing operation really ineffective).
// It is recommended to use the tool together with another sanitizer to expose
// more bugs (asan, lsan, etc...)
//
//===----------------------------------------------------------------------===//

#include "language/Demangling/Demangle.h"
#include "language/Demangling/ManglingMacros.h"
#include <stddef.h>
#include <stdint.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  language::Demangle::Context DCtx;
  language::Demangle::DemangleOptions Opts;
  std::string NullTermStr((const char *)Data, Size);
  language::Demangle::NodePointer pointer = DCtx.demangleSymbolAsNode(NullTermStr);
  return 0; // Non-zero return values are reserved for future use.
}
