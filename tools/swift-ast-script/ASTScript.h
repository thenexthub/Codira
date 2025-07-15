//===--- ASTScript.h - AST script type --------------------------*- C++ -*-===//
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
// The AST for a language-ast-script script.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SCRIPTING_ASTSCRIPT_H
#define LANGUAGE_SCRIPTING_ASTSCRIPT_H

#include "language/Basic/Toolchain.h"

namespace language {
namespace scripting {
class ASTScriptConfiguration;

class ASTScript {
  ASTScriptConfiguration &Config;

public:
  ASTScript(ASTScriptConfiguration &config) : Config(config) {}

  static std::unique_ptr<ASTScript> parse(ASTScriptConfiguration &config);

  bool execute() const;
};

}
}

#endif
