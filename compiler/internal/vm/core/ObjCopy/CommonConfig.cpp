//===- CommonConfig.cpp ---------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "vm/core/ObjCopy/CommonConfig.h"
#include "vm/core/Support/Errc.h"

using namespace vm::core;
using namespace vm::core::objcopy;

Expected<NameOrPattern>
NameOrPattern::create(StringRef Pattern, MatchStyle MS,
                      function_ref<Error(Error)> ErrorCallback) {
  switch (MS) {
  case MatchStyle::Literal:
    return NameOrPattern(Pattern);
  case MatchStyle::Wildcard: {
    bool IsPositiveMatch = !Pattern.consume_front("!");
    Expected<GlobPattern> GlobOrErr = GlobPattern::create(Pattern);

    // If we couldn't create it as a glob, report the error, but try again
    // with a literal if the error reporting is non-fatal.
    if (!GlobOrErr) {
      if (Error E = ErrorCallback(GlobOrErr.takeError()))
        return std::move(E);
      return create(Pattern, MatchStyle::Literal, ErrorCallback);
    }

    return NameOrPattern(std::make_shared<GlobPattern>(*GlobOrErr),
                         IsPositiveMatch);
  }
  case MatchStyle::Regex: {
    Regex RegEx(Pattern);
    std::string Err;
    if (!RegEx.isValid(Err))
      return createStringError(errc::invalid_argument,
                               "cannot compile regular expression \'" +
                                   Pattern + "\': " + Err);
    SmallVector<char, 32> Data;
    return NameOrPattern(std::make_shared<Regex>(
        ("^" + Pattern.ltrim('^').rtrim('$') + "$").toStringRef(Data)));
  }
  }
  llvm_unreachable("Unhandled toolchain.objcopy.MatchStyle enum");
}
