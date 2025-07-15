//===--- DocumentationCategory.h - Accessors for @_documentation ----------===//
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

#ifndef LANGUAGE_SYMBOLGRAPHGEN_DOCUMENTATIONCATEGORY_H
#define LANGUAGE_SYMBOLGRAPHGEN_DOCUMENTATIONCATEGORY_H

#include "language/AST/Decl.h"

#include "toolchain/Support/Compiler.h"

namespace language {
namespace symbolgraphgen {

TOOLCHAIN_ATTRIBUTE_USED
static StringRef documentationMetadataForDecl(const Decl *D) {
  if (!D) return {};

  if (const auto *DC = D->getAttrs().getAttribute<DocumentationAttr>()) {
    return DC->Metadata;
  }

  return {};
}

TOOLCHAIN_ATTRIBUTE_USED
static std::optional<AccessLevel>
documentationVisibilityForDecl(const Decl *D) {
  if (!D)
    return std::nullopt;

  if (const auto *DC = D->getAttrs().getAttribute<DocumentationAttr>()) {
    return DC->Visibility;
  }

  return std::nullopt;
}

} // namespace symbolgraphgen
} // namespace language

#endif // LANGUAGE_SYMBOLGRAPHGEN_DOCUMENTATIONCATEGORY_H
