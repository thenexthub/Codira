//===--- CommentBriefParser.h - Dumb comment parser -------------*- C++ -*-===//
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
//  This file defines a very simple Doxygen comment parser.
//
//===----------------------------------------------------------------------===//


#ifndef LANGUAGE_CORE_AST_COMMENTBRIEFPARSER_H
#define LANGUAGE_CORE_AST_COMMENTBRIEFPARSER_H

#include "language/Core/AST/CommentLexer.h"

namespace language::Core {
namespace comments {

/// A very simple comment parser that extracts "a brief description".
///
/// Due to a variety of comment styles, it considers the following as "a brief
/// description", in order of priority:
/// \li a \or \\short command,
/// \li the first paragraph,
/// \li a \\result or \\return or \\returns paragraph.
class BriefParser {
  Lexer &L;

  const CommandTraits &Traits;

  /// Current lookahead token.
  Token Tok;

  SourceLocation ConsumeToken() {
    SourceLocation Loc = Tok.getLocation();
    L.lex(Tok);
    return Loc;
  }

public:
  BriefParser(Lexer &L, const CommandTraits &Traits);

  /// Return the best "brief description" we can find.
  std::string Parse();
};

} // end namespace comments
} // end namespace language::Core

#endif

