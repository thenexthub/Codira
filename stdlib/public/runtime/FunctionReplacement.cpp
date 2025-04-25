//===--- FunctionReplacement.cpp ------------------------------------------===//
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

#include "language/Runtime/FunctionReplacement.h"
#include "languageTLSContext.h"

using namespace language;
using namespace language::runtime;

char *swift::swift_getFunctionReplacement(char **ReplFnPtr, char *CurrFn) {
  char *ReplFn = *ReplFnPtr;
  char *RawReplFn = ReplFn;

#if SWIFT_PTRAUTH
  RawReplFn = ptrauth_strip(RawReplFn, ptrauth_key_function_pointer);
#endif
  if (RawReplFn == CurrFn)
    return nullptr;

  auto &ctx = SwiftTLSContext::get();
  if (ctx.CallOriginalOfReplacedFunction) {
    ctx.CallOriginalOfReplacedFunction = false;
    return nullptr;
  }
  return ReplFn;
}

char *swift::swift_getOrigOfReplaceable(char **OrigFnPtr) {
  char *OrigFn = *OrigFnPtr;
  SwiftTLSContext::get().CallOriginalOfReplacedFunction = true;
  return OrigFn;
}
