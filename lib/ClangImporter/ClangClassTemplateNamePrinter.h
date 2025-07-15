//===--- ClangClassTemplateNamePrinter.h ------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CLANG_TEMPLATE_NAME_PRINTER_H
#define LANGUAGE_CLANG_TEMPLATE_NAME_PRINTER_H

#include "ImportName.h"
#include "language/AST/ASTContext.h"
#include "clang/AST/DeclTemplate.h"

namespace language {
namespace importer {

/// Returns a Codira representation of a C++ class template specialization name,
/// e.g. "vector<CWideChar, allocator<CWideChar>>".
///
/// This expands the entire tree of template instantiation names recursively.
/// While printing deep instantiation levels might not increase readability, it
/// is important to do because the C++ templated class names get mangled,
/// therefore they must be unique for different instantiations.
///
/// This function does not instantiate any templates and does not modify the AST
/// in any way.
std::string printClassTemplateSpecializationName(
    const clang::ClassTemplateSpecializationDecl *decl, ASTContext &languageCtx,
    NameImporter *nameImporter, ImportNameVersion version);

} // namespace importer
} // namespace language

#endif // LANGUAGE_CLANG_TEMPLATE_NAME_PRINTER_H
