//===--- PrintClangClassType.h - Print class types in C/C++ -----*- C++ -*-===//
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

#ifndef LANGUAGE_PRINTASCLANG_PRINTCLANGCLASSTYPE_H
#define LANGUAGE_PRINTASCLANG_PRINTCLANGCLASSTYPE_H

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Support/raw_ostream.h"

namespace language {

class ClassDecl;
class ModuleDecl;
class DeclAndTypePrinter;

/// Responsible for printing a Codira class decl or in C or C++ mode, to
/// be included in a Codira module's generated clang header.
class ClangClassTypePrinter {
public:
  ClangClassTypePrinter(raw_ostream &os) : os(os) {}

  /// Print the C++ class definition that corresponds to the given Codira class.
  void printClassTypeDecl(const ClassDecl *typeDecl,
                          toolchain::function_ref<void(void)> bodyPrinter,
                          DeclAndTypePrinter &declAndTypePrinter);

  static void
  printClassTypeReturnScaffold(raw_ostream &os, const ClassDecl *type,
                               const ModuleDecl *moduleContext,
                               toolchain::function_ref<void(void)> bodyPrinter);

  static void printParameterCxxtoCUseScaffold(
      raw_ostream &os, const ClassDecl *type, const ModuleDecl *moduleContext,
      toolchain::function_ref<void(void)> bodyPrinter, bool isInOut);

private:
  raw_ostream &os;
};

} // end namespace language

#endif
