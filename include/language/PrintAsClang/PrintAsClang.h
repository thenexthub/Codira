//===--- PrintAsClang.h - Emit a header file for a Codira AST ----*- C++ -*-===//
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

#ifndef LANGUAGE_PRINTASCLANG_H
#define LANGUAGE_PRINTASCLANG_H

#include "language/Basic/Toolchain.h"
#include "language/AST/AttrKind.h"
#include "language/AST/Identifier.h"

namespace clang {
class HeaderSearch;
}

namespace language {
class FrontendOptions;
class IRGenOptions;
class ModuleDecl;
class ValueDecl;

/// Print the exposed declarations in a module into a Clang header.
///
/// The Objective-C compatible declarations are printed into a block that
/// ensures that those declarations are only usable when the header is
/// compiled in Objective-C mode.
/// The C++ compatible declarations are printed into a block that ensures
/// that those declarations are only usable when the header is compiled in
/// C++ mode.
///
/// Returns true on error.
bool printAsClangHeader(raw_ostream &out, ModuleDecl *M,
                        StringRef bridgingHeader,
                        const FrontendOptions &frontendOpts,
                        const IRGenOptions &irGenOpts,
                        clang::HeaderSearch &headerSearchInfo);
}

#endif
