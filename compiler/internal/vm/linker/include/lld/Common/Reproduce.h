//===- Reproduce.h - Utilities for creating reproducers ---------*- C++ -*-===//
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

#ifndef LLD_COMMON_REPRODUCE_H
#define LLD_COMMON_REPRODUCE_H

#include "lld/Common/LLVM.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"

namespace llvm {
namespace opt { class Arg; }
}

namespace lld {

// Makes a given pathname an absolute path first, and then remove
// beginning /. For example, "../foo.o" is converted to "home/john/foo.o",
// assuming that the current directory is "/home/john/bar".
std::string relativeToRoot(StringRef path);

// Quote a given string if it contains a space character.
std::string quote(StringRef s);

// Returns the string form of the given argument.
std::string toString(const llvm::opt::Arg &arg);
}

#endif
