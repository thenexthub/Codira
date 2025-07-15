//===--- EditorPlaceholder.h - Handling for editor placeholders -*- C++ -*-===//
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
///
/// \file
/// Provides info about editor placeholders, <#such as this#>.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_EDITORPLACEHOLDER_H
#define LANGUAGE_BASIC_EDITORPLACEHOLDER_H

#include "toolchain/ADT/StringRef.h"
#include <optional>

namespace language {

enum class EditorPlaceholderKind {
  Basic,
  Typed,
};

struct EditorPlaceholderData {
  /// Placeholder kind.
  EditorPlaceholderKind Kind;
  /// The part that is displayed in the editor.
  toolchain::StringRef Display;
  /// If kind is \c Typed, this is the type string for the placeholder.
  toolchain::StringRef Type;
  /// If kind is \c Typed, this is the type string to be considered for
  /// placeholder expansion.
  /// It can be same as \c Type or different if \c Type is a typealias.
  toolchain::StringRef TypeForExpansion;
};

/// Deconstructs a placeholder string and returns info about it.
/// \returns None if the \c PlaceholderText is not a valid placeholder string.
std::optional<EditorPlaceholderData>
parseEditorPlaceholder(toolchain::StringRef PlaceholderText);

/// Checks if an identifier with the given text is an editor placeholder
bool isEditorPlaceholder(toolchain::StringRef IdentifierText);
} // end namespace language

#endif // LANGUAGE_BASIC_EDITORPLACEHOLDER_H
