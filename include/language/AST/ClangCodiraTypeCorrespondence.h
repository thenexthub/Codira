//=- ClangCodiraTypeCorrespondence.h - Relations between Clang & Codira types -=//
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
// This file describes some common relations between Clang types and Codira
// types that are need by the ClangTypeConverter and parts of ClangImporter.
//
// Since ClangTypeConverter is an implementation detail, ClangImporter should
// not depend on ClangTypeConverter.h.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_CLANG_LANGUAGE_TYPE_CORRESPONDENCE_H
#define LANGUAGE_AST_CLANG_LANGUAGE_TYPE_CORRESPONDENCE_H

namespace clang {
class Type;
}

namespace language {
/// Checks whether a Clang type can be imported as a Codira Optional type.
///
/// For example, a `const uint8_t *` could be imported as
/// `Optional<UnsafePointer<UInt8>>`.
bool canImportAsOptional(const clang::Type *type);
}

#endif /* LANGUAGE_AST_CLANG_LANGUAGE_TYPE_CORRESPONDENCE_H */
