//===- Args.h ---------------------------------------------------*- C++ -*-===//
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

#ifndef LLD_ARGS_H
#define LLD_ARGS_H

#include "lld/Common/LLVM.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/MemoryBuffer.h"
#include <vector>

namespace llvm {
namespace opt {
class InputArgList;
}
} // namespace llvm

namespace lld {
namespace args {

int getCGOptLevel(int optLevelLTO);

int64_t getInteger(llvm::opt::InputArgList &args, unsigned key,
                   int64_t Default);

int64_t getHex(llvm::opt::InputArgList &args, unsigned key, int64_t Default);

llvm::SmallVector<StringRef, 0> getStrings(llvm::opt::InputArgList &args,
                                           int id);

uint64_t getZOptionValue(llvm::opt::InputArgList &args, int id, StringRef key,
                         uint64_t Default);

std::vector<StringRef> getLines(MemoryBufferRef mb);

StringRef getFilenameWithoutExe(StringRef path);

} // namespace args
} // namespace lld

#endif
