//===--- PrintCodiraToClangCoreScaffold.h - Print core decls -----*- C++ -*-===//
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

#ifndef LANGUAGE_PRINTASCLANG_PRINTLANGUAGETOCLANGCORESCAFFOLD_H
#define LANGUAGE_PRINTASCLANG_PRINTLANGUAGETOCLANGCORESCAFFOLD_H

#include "toolchain/Support/raw_ostream.h"

namespace language {

class ASTContext;
class PrimitiveTypeMapping;
class CodiraToClangInteropContext;

/// Print out the core declarations required by C/C++ that are part of the core
/// Codira stdlib code.
void printCodiraToClangCoreScaffold(CodiraToClangInteropContext &ctx,
                                   ASTContext &astContext,
                                   PrimitiveTypeMapping &typeMapping,
                                   toolchain::raw_ostream &os);

} // end namespace language

#endif
