//- ClangCodiraTypeCorrespondence.cpp - Relations between Clang & Codira types -//
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
// See description in ClangCodiraTypeCorrespondence.h.
//
//===----------------------------------------------------------------------===//

#include "language/AST/ClangCodiraTypeCorrespondence.h"
#include "language/Basic/Assertions.h"
#include "clang/AST/Type.h"

bool language::canImportAsOptional(const clang::Type *type) {
  // Note: this mimics ImportHint::canImportAsOptional.

  // Includes CoreFoundation types such as CFStringRef (== struct CFString *).
  return type && (type->isPointerType()
                  || type->isBlockPointerType()
                  || type->isObjCObjectPointerType());
}
