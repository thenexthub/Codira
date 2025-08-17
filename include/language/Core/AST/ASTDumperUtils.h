//===--- ASTDumperUtils.h - Printing of AST nodes -------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This file implements AST utilities for traversal down the tree.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_ASTDUMPERUTILS_H
#define LANGUAGE_CORE_AST_ASTDUMPERUTILS_H

#include "toolchain/Support/raw_ostream.h"

namespace language::Core {

/// Used to specify the format for printing AST dump information.
enum ASTDumpOutputFormat {
  ADOF_Default,
  ADOF_JSON
};

// Colors used for various parts of the AST dump
// Do not use bold yellow for any text.  It is hard to read on white screens.

struct TerminalColor {
  toolchain::raw_ostream::Colors Color;
  bool Bold;
};

// Red           - CastColor
// Green         - TypeColor
// Bold Green    - DeclKindNameColor, UndeserializedColor
// Yellow        - AddressColor, LocationColor
// Blue          - CommentColor, NullColor, IndentColor
// Bold Blue     - AttrColor
// Bold Magenta  - StmtColor
// Cyan          - ValueKindColor, ObjectKindColor
// Bold Cyan     - ValueColor, DeclNameColor

// Decl kind names (VarDecl, FunctionDecl, etc)
static const TerminalColor DeclKindNameColor = {toolchain::raw_ostream::GREEN, true};
// Attr names (CleanupAttr, GuardedByAttr, etc)
static const TerminalColor AttrColor = {toolchain::raw_ostream::BLUE, true};
// Statement names (DeclStmt, ImplicitCastExpr, etc)
static const TerminalColor StmtColor = {toolchain::raw_ostream::MAGENTA, true};
// Comment names (FullComment, ParagraphComment, TextComment, etc)
static const TerminalColor CommentColor = {toolchain::raw_ostream::BLUE, false};

// Type names (int, float, etc, plus user defined types)
static const TerminalColor TypeColor = {toolchain::raw_ostream::GREEN, false};

// Pointer address
static const TerminalColor AddressColor = {toolchain::raw_ostream::YELLOW, false};
// Source locations
static const TerminalColor LocationColor = {toolchain::raw_ostream::YELLOW, false};

// lvalue/xvalue
static const TerminalColor ValueKindColor = {toolchain::raw_ostream::CYAN, false};
// bitfield/objcproperty/objcsubscript/vectorcomponent
static const TerminalColor ObjectKindColor = {toolchain::raw_ostream::CYAN, false};
// contains-errors
static const TerminalColor ErrorsColor = {toolchain::raw_ostream::RED, true};

// Null statements
static const TerminalColor NullColor = {toolchain::raw_ostream::BLUE, false};

// Undeserialized entities
static const TerminalColor UndeserializedColor = {toolchain::raw_ostream::GREEN,
                                                  true};

// CastKind from CastExpr's
static const TerminalColor CastColor = {toolchain::raw_ostream::RED, false};

// Value of the statement
static const TerminalColor ValueColor = {toolchain::raw_ostream::CYAN, true};
// Decl names
static const TerminalColor DeclNameColor = {toolchain::raw_ostream::CYAN, true};

// Indents ( `, -. | )
static const TerminalColor IndentColor = {toolchain::raw_ostream::BLUE, false};

class ColorScope {
  toolchain::raw_ostream &OS;
  const bool ShowColors;

public:
  ColorScope(toolchain::raw_ostream &OS, bool ShowColors, TerminalColor Color)
      : OS(OS), ShowColors(ShowColors) {
    if (ShowColors)
      OS.changeColor(Color.Color, Color.Bold);
  }
  ~ColorScope() {
    if (ShowColors)
      OS.resetColor();
  }
};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_ASTDUMPERUTILS_H
