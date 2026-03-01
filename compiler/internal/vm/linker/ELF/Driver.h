//===- Driver.h -------------------------------------------------*- C++ -*-===//
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

#ifndef LLD_ELF_DRIVER_H
#define LLD_ELF_DRIVER_H

#include "lld/Common/LLVM.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/ArgList.h"
#include <optional>

namespace lld::elf {
struct Ctx;

// Parses command line options.
class ELFOptTable : public llvm::opt::GenericOptTable {
public:
  ELFOptTable();
  llvm::opt::InputArgList parse(Ctx &, ArrayRef<const char *> argv);
};

// Create enum with OPT_xxx values for each option in Options.td
enum {
  OPT_INVALID = 0,
#define OPTION(...) LLVM_MAKE_OPT_ID(__VA_ARGS__),
#include "Options.inc"
#undef OPTION
};

void printHelp(Ctx &ctx);
std::string createResponseFile(const llvm::opt::InputArgList &args);

std::optional<std::string> findFromSearchPaths(Ctx &, StringRef path);
std::optional<std::string> searchScript(Ctx &, StringRef path);
std::optional<std::string> searchLibraryBaseName(Ctx &, StringRef path);
std::optional<std::string> searchLibrary(Ctx &, StringRef path);

} // namespace lld::elf

#endif
