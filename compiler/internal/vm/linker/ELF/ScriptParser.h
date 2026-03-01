//===- ScriptParser.h -------------------------------------------*- C++ -*-===//
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

#ifndef LLD_ELF_SCRIPT_PARSER_H
#define LLD_ELF_SCRIPT_PARSER_H

#include "lld/Common/LLVM.h"
#include "llvm/Support/MemoryBufferRef.h"

namespace lld::elf {
struct Ctx;

// Parses a linker script. Calling this function updates
// lld::elf::config and lld::elf::script.
void readLinkerScript(Ctx &ctx, MemoryBufferRef mb);

// Parses a version script.
void readVersionScript(Ctx &ctx, MemoryBufferRef mb);

void readDynamicList(Ctx &ctx, MemoryBufferRef mb);

// Parses the defsym expression.
void readDefsym(Ctx &ctx, MemoryBufferRef mb);

bool hasWildcard(StringRef s);

} // namespace lld::elf

#endif
