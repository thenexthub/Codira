//===--- ASTScriptConfiguration.h - AST script configuration ----*- C++ -*-===//
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
// Types for configuring an AST script invocation.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SCRIPTING_ASTSCRIPTCONFIGURATION_H
#define LANGUAGE_SCRIPTING_ASTSCRIPTCONFIGURATION_H

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/StringRef.h"

namespace language {
class CompilerInstance;

namespace scripting {

/// A configuration for working with an ASTScript.
class ASTScriptConfiguration {
  ASTScriptConfiguration(CompilerInstance &compiler) : Compiler(compiler) {}
public:
  CompilerInstance &Compiler;
  StringRef ScriptFile;

  /// Attempt to parse this configuration.
  ///
  /// Returns null if there's a problem.
  static std::unique_ptr<ASTScriptConfiguration>
  parse(CompilerInstance &compiler, ArrayRef<const char *> args);
};

}
}

#endif
