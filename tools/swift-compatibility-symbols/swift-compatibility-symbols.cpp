//===--- language-compatibility-symbols.cpp - Emit Clang symbol list ---------===//
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
// A simple program to dynamically generate the list of macros added to
// compatibility headers.
//
//===----------------------------------------------------------------------===//

#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/raw_ostream.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <system_error>

namespace options {

static toolchain::cl::OptionCategory Category("language-compatibility-symbols Options");

static toolchain::cl::opt<std::string>
  OutputFilename("output-filename",
                 toolchain::cl::desc("Filename for the output file"),
                 toolchain::cl::init("-"),
                 toolchain::cl::cat(Category));

}

int main(int argc, char *argv[]) {
  toolchain::cl::HideUnrelatedOptions(options::Category);
  toolchain::cl::ParseCommandLineOptions(argc, argv,
                                    "Codira Compatibility Symbols listing\n");

  std::error_code error;
  toolchain::raw_fd_ostream OS(options::OutputFilename, error, toolchain::sys::fs::CD_CreateAlways);
  if (OS.has_error() || error) {
    toolchain::errs() << "Error when trying to write to output file; error code "
                 << error.message() << "\n";
    return EXIT_FAILURE;
  }

  toolchain::SmallVector<toolchain::StringRef, 40> symbols;
#define CLANG_MACRO_DEFINED(NAME) \
  symbols.push_back(NAME);

#define CLANG_MACRO(NAME, ARGS, VALUE) \
  CLANG_MACRO_DEFINED(NAME)

#define CLANG_MACRO_BODY(NAME, BODY) \
  CLANG_MACRO_DEFINED(NAME)

#define CLANG_MACRO_ALTERNATIVE(NAME, ARGS, CONDITION, VALUE, ALTERNATIVE) \
  CLANG_MACRO_DEFINED(NAME)

#define CLANG_MACRO_CONDITIONAL(NAME, ARGS, CONDITION, VALUE) \
  CLANG_MACRO_DEFINED(NAME)

#define CLANG_MACRO_OBJC(NAME, ARGS, VALUE) \
  CLANG_MACRO_DEFINED(NAME)

#define CLANG_MACRO_CXX(NAME, ARGS, VALUE, ALTERNATIVE) \
  CLANG_MACRO_DEFINED(NAME)

#define CLANG_MACRO_CXX_BODY(NAME, BODY) \
  CLANG_MACRO_DEFINED(NAME)

#include "language/PrintAsClang/ClangMacros.def"

  std::sort(symbols.begin(), symbols.end());

  for (const toolchain::StringRef sym : symbols) {
    OS << sym << "\n";
  }

  return EXIT_SUCCESS;
}

