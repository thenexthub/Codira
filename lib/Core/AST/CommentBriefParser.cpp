//===--- CommentBriefParser.cpp - Dumb comment parser ---------------------===//
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

#include "language/Core/AST/CommentBriefParser.h"
#include "language/Core/AST/CommentCommandTraits.h"
#include "language/Core/Basic/CharInfo.h"

namespace language::Core {
namespace comments {

namespace {

/// Convert all whitespace into spaces, remove leading and trailing spaces,
/// compress multiple spaces into one.
void cleanupBrief(std::string &S) {
  bool PrevWasSpace = true;
  std::string::iterator O = S.begin();
  for (std::string::iterator I = S.begin(), E = S.end();
       I != E; ++I) {
    const char C = *I;
    if (language::Core::isWhitespace(C)) {
      if (!PrevWasSpace) {
        *O++ = ' ';
        PrevWasSpace = true;
      }
    } else {
      *O++ = C;
      PrevWasSpace = false;
    }
  }
  if (O != S.begin() && *(O - 1) == ' ')
    --O;

  S.resize(O - S.begin());
}

bool isWhitespace(StringRef Text) {
  return toolchain::all_of(Text, language::Core::isWhitespace);
}
} // unnamed namespace

BriefParser::BriefParser(Lexer &L, const CommandTraits &Traits) :
    L(L), Traits(Traits) {
  // Get lookahead token.
  ConsumeToken();
}

std::string BriefParser::Parse() {
  std::string FirstParagraphOrBrief;
  std::string ReturnsParagraph;
  bool InFirstParagraph = true;
  bool InBrief = false;
  bool InReturns = false;

  while (Tok.isNot(tok::eof)) {
    if (Tok.is(tok::text)) {
      if (InFirstParagraph || InBrief)
        FirstParagraphOrBrief += Tok.getText();
      else if (InReturns)
        ReturnsParagraph += Tok.getText();
      ConsumeToken();
      continue;
    }

    if (Tok.is(tok::backslash_command) || Tok.is(tok::at_command)) {
      const CommandInfo *Info = Traits.getCommandInfo(Tok.getCommandID());
      if (Info->IsBriefCommand) {
        FirstParagraphOrBrief.clear();
        InBrief = true;
        ConsumeToken();
        continue;
      }
      if (Info->IsReturnsCommand) {
        InReturns = true;
        InBrief = false;
        InFirstParagraph = false;
        ReturnsParagraph += "Returns ";
        ConsumeToken();
        continue;
      }
      // Block commands implicitly start a new paragraph.
      if (Info->IsBlockCommand) {
        // We found an implicit paragraph end.
        InFirstParagraph = false;
        if (InBrief)
          break;
      }
    }

    if (Tok.is(tok::newline)) {
      if (InFirstParagraph || InBrief)
        FirstParagraphOrBrief += ' ';
      else if (InReturns)
        ReturnsParagraph += ' ';
      ConsumeToken();

      // If the next token is a whitespace only text, ignore it.  Thus we allow
      // two paragraphs to be separated by line that has only whitespace in it.
      //
      // We don't need to add a space to the parsed text because we just added
      // a space for the newline.
      if (Tok.is(tok::text)) {
        if (isWhitespace(Tok.getText()))
          ConsumeToken();
      }

      if (Tok.is(tok::newline)) {
        ConsumeToken();
        // We found a paragraph end.  This ends the brief description if
        // \command or its equivalent was explicitly used.
        // Stop scanning text because an explicit \paragraph is the
        // preferred one.
        if (InBrief)
          break;
        // End first paragraph if we found some non-whitespace text.
        if (InFirstParagraph && !isWhitespace(FirstParagraphOrBrief))
          InFirstParagraph = false;
        // End the \\returns paragraph because we found the paragraph end.
        InReturns = false;
      }
      continue;
    }

    // We didn't handle this token, so just drop it.
    ConsumeToken();
  }

  cleanupBrief(FirstParagraphOrBrief);
  if (!FirstParagraphOrBrief.empty())
    return FirstParagraphOrBrief;

  cleanupBrief(ReturnsParagraph);
  return ReturnsParagraph;
}

} // end namespace comments
} // end namespace language::Core


