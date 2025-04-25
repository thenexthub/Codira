//===--- ASTDumper.h - Swift AST Dumper flags -------------------*- C++ -*-===//
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
// This file defines types that are used to control the level of detail printed
// by the AST dumper.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_AST_AST_DUMPER_H
#define SWIFT_AST_AST_DUMPER_H

namespace language {

/// Describes the nature of requests that should be kicked off, if any, to
/// compute members and top-level decls when dumping an AST.
enum class ASTDumpMemberLoading {
  /// Dump cached members if available; if they are not, do not kick off any
  /// parsing or type-checking requests.
  None,

  /// Dump parsed members, kicking off a parsing request if necessary to compute
  /// them, but not performing additional type-checking.
  Parsed,

  /// Dump all fully-type checked members, kicking off any requests necessary to
  /// compute them.
  TypeChecked,
};

} // namespace language

#endif // SWIFT_AST_AST_DUMPER_H