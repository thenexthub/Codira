//===--- ASTScriptConfiguration.cpp ---------------------------------------===//
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
///
/// Configuration parsing for an AST script.
///
//===----------------------------------------------------------------------===//

#include "ASTScriptConfiguration.h"

#include "language/Basic/QuotedString.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;
using namespace scripting;

std::unique_ptr<ASTScriptConfiguration>
ASTScriptConfiguration::parse(CompilerInstance &compiler,
                              ArrayRef<const char *> args) {
  bool hadError = false;

  std::unique_ptr<ASTScriptConfiguration> result(
    new ASTScriptConfiguration(compiler));

#define emitError(WHAT) do {                                                  \
    toolchain::errs() << "error: " << WHAT << "\n";                                \
    hadError = true;                                                          \
  } while (0)

  auto popArg = [&](const char *what) -> StringRef {
    // Gracefully handle the case of running out of arguments.
    if (args.empty()) {
      assert(what && "expected explanation here!");
      emitError(what);
      return "";
    }

    auto arg = args.front();
    args = args.slice(1);
    return arg;
  };

  auto setScriptFile = [&](StringRef filename) {
    if (!result->ScriptFile.empty()) {
      emitError("multiple script files ("
                  << QuotedString(result->ScriptFile)
                  << ", "
                  << QuotedString(filename)
                  << ")");
    }
    result->ScriptFile = filename;
  };

  // Parse the arguments.
  while (!hadError && !args.empty()) {
    StringRef arg = popArg(nullptr);
    if (!arg.starts_with("-")) {
      setScriptFile(arg);
    } else if (arg == "-f") {
      StringRef filename = popArg("expected path after -f");
      if (!hadError)
        setScriptFile(filename);
    } else if (arg == "-h" || arg == "--help") {
      toolchain::errs()
        << "usage: language-ast-script <script-args> -- <compiler-args\n"
           "accepted script arguments:\n"
           "  <filename>\n"
           "  -f <filename>\n"
           "    Specify the file to use as the script; required argument\n";
      hadError = true;
    } else {
      emitError("unknown argument " << QuotedString(arg));
    }
  }

  // Check well-formedness.
  if (!hadError) {
    if (result->ScriptFile.empty()) {
      emitError("script file is required");
    }
  }

  if (hadError)
    result.reset();
  return result;
}
