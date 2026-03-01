//===--- COFFDirectiveParser.h - JITLink coff directive parser --*- C++ -*-===//
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
//
// MSVC COFF directive parser
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_EXECUTIONENGINE_JITLINK_COFFDIRECTIVEPARSER_H
#define LLVM_EXECUTIONENGINE_JITLINK_COFFDIRECTIVEPARSER_H

#include "vm/core/ExecutionEngine/JITLink/JITLink.h"
#include "vm/core/Option/Arg.h"
#include "vm/core/Option/ArgList.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/StringSaver.h"
#include "vm/core/TargetParser/Triple.h"

namespace vm::core {
namespace jitlink {

enum {
  COFF_OPT_INVALID = 0,
#define OPTION(...) LLVM_MAKE_OPT_ID_WITH_ID_PREFIX(COFF_OPT_, __VA_ARGS__),
#include "COFFOptions.inc"
#undef OPTION
};

/// Parser for the MSVC specific preprocessor directives.
/// https://docs.microsoft.com/en-us/cpp/preprocessor/comment-c-cpp?view=msvc-160
class COFFDirectiveParser {
public:
  Expected<opt::InputArgList> parse(StringRef Str);

private:
  toolchain::BumpPtrAllocator bAlloc;
  toolchain::StringSaver saver{bAlloc};
};

} // end namespace jitlink
} // end namespace vm::core

#endif // LLVM_EXECUTIONENGINE_JITLINK_COFFDIRECTIVEPARSER_H
