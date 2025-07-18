//===---- InlinableText.cpp - Extract inlinable source text -----*- C++ -*-===//
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
#include "InlinableText.h"
#include "language/AST/ASTBridging.h"
#include "language/AST/ASTContext.h"
#include "language/AST/ASTNode.h"
#include "language/AST/ASTVisitor.h"
#include "language/AST/ASTWalker.h"
#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/Basic/Assertions.h"
#include "language/Bridging/ASTGen.h"
#include "language/Parse/Lexer.h"

#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/SmallString.h"

using namespace language;

#if LANGUAGE_BUILD_LANGUAGE_SYNTAX
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C" BridgedStringRef
language_ASTGen_extractInlinableText(BridgedASTContext ctx,
                                  BridgedStringRef sourceText);
#pragma clang diagnostic pop
#else
/// Appends the textual contents of the provided source range, stripping
/// the contents of comments that appear in the source.
///
/// Given that comments are treated as whitespace, this also appends a
/// space or newline (depending if the comment was multi-line and itself
/// had newlines in the body) in place of the comment, to avoid fusing tokens
/// together.
static void appendRange(
  SourceManager &sourceMgr, SourceLoc start, SourceLoc end,
  SmallVectorImpl<char> &scratch) {
  unsigned bufferID = sourceMgr.findBufferContainingLoc(start);
  unsigned offset = sourceMgr.getLocOffsetInBuffer(start, bufferID);
  unsigned endOffset = sourceMgr.getLocOffsetInBuffer(end, bufferID);

  // Strip comments from the chunk before adding it by re-lexing the range.
  LangOptions FakeLangOpts;
  Lexer lexer(FakeLangOpts, sourceMgr, bufferID, nullptr, LexerMode::Codira,
    HashbangMode::Disallowed, CommentRetentionMode::ReturnAsTokens,
    offset, endOffset);

  SourceLoc nonCommentStart = start;
  Token token;

  // Re-lex the range, and skip the full text of `tok::comment` tokens.
  while (!token.is(tok::eof)) {
    lexer.lex(token);

    // Skip over #sourceLocation's in the file.
    if (token.is(tok::pound_sourceLocation)) {

      // Append the text leading up to the #sourceLocation
      auto charRange = CharSourceRange(
        sourceMgr, nonCommentStart, token.getLoc());
      StringRef text = sourceMgr.extractText(charRange);
      scratch.append(text.begin(), text.end());

      // Skip to the right paren. We know the AST is already valid, so there's
      // definitely a right paren.
      while (!token.is(tok::r_paren)) {
        lexer.lex(token);
      }

      nonCommentStart = Lexer::getLocForEndOfToken(sourceMgr, token.getLoc());
    }

    if (token.is(tok::comment)) {
      // Grab the start of the full comment token (with leading trivia as well)
      SourceLoc commentLoc = token.getLoc();

      // Find the end of the token (with trailing trivia)
      SourceLoc endLoc = Lexer::getLocForEndOfToken(sourceMgr, token.getLoc());

      // The comment token's range includes leading/trailing whitespace, so trim
      // whitespace and only strip the portions of the comment that are not
      // whitespace.
      CharSourceRange range = CharSourceRange(sourceMgr, commentLoc, endLoc);
      StringRef fullTokenText = sourceMgr.extractText(range);
      unsigned leadingWhitespace = fullTokenText.size() - 
        fullTokenText.ltrim().size();
      if (leadingWhitespace > 0) {
        commentLoc = commentLoc.getAdvancedLoc(leadingWhitespace);
      }

      unsigned trailingWhitespace = fullTokenText.size() -
        fullTokenText.rtrim().size();
      if (trailingWhitespace > 0) {
        endLoc = endLoc.getAdvancedLoc(-trailingWhitespace);
      }

      // First, extract the text up to the start of the comment, including the
      // whitespace.
      auto charRange = CharSourceRange(sourceMgr, nonCommentStart, commentLoc);
      StringRef text = sourceMgr.extractText(charRange);
      scratch.append(text.begin(), text.end());

      // Next, search through the comment text to see if it's a block comment
      // with a newline. If so we need to re-insert a newline to avoid fusing
      // multi-line tokens together.
      auto commentTextRange = CharSourceRange(sourceMgr, commentLoc, endLoc);
      StringRef commentText = sourceMgr.extractText(commentTextRange);
      bool hasNewline = commentText.find_first_of("\n\r") != StringRef::npos;

      // Use a newline as a filler character if the comment itself had a newline
      // in it.
      char filler = hasNewline ? '\n' : ' ';

      // Append a single whitespace filler character, to avoid fusing tokens.
      scratch.push_back(filler);

      // Start the next region after the contents of the comment.
      nonCommentStart = endLoc;
    }
  }

  if (nonCommentStart.isValid() && nonCommentStart != end) {
    auto charRange = CharSourceRange(sourceMgr, nonCommentStart, end);
    StringRef text = sourceMgr.extractText(charRange);
    scratch.append(text.begin(), text.end());
  }
}
#endif // LANGUAGE_BUILD_LANGUAGE_SYNTAX

StringRef language::extractInlinableText(ASTContext &ctx, ASTNode node,
                                      SmallVectorImpl<char> &scratch) {
  SourceManager &sourceMgr = ctx.SourceMgr;

#if LANGUAGE_BUILD_LANGUAGE_SYNTAX
  CharSourceRange sourceTextRange =
      Lexer::getCharSourceRangeFromSourceRange(
        sourceMgr, node.getSourceRange());
  StringRef sourceText = sourceMgr.extractText(sourceTextRange);
  auto resultText = language_ASTGen_extractInlinableText(ctx, sourceText);

  scratch.clear();
  scratch.insert(scratch.begin(),
                 resultText.unbridged().begin(),
                 resultText.unbridged().end());
  language_ASTGen_freeBridgedString(resultText);
  return { scratch.data(), scratch.size() };
#else
  // Get the full start and end of the provided node, as character locations.
  SourceLoc start = node.getStartLoc();
  SourceLoc end = Lexer::getLocForEndOfToken(sourceMgr, node.getEndLoc());
  appendRange(sourceMgr, start, end, scratch);

  return { scratch.data(), scratch.size() };
#endif
}
