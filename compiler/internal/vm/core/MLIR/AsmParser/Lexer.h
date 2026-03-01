//===- Lexer.h - MLIR Lexer Interface ---------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This file declares the MLIR Lexer class.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_LIB_ASMPARSER_LEXER_H
#define MLIR_LIB_ASMPARSER_LEXER_H

#include "Token.h"
#include "mlir/AsmParser/AsmParser.h"

namespace mlir {
class Location;

/// This class breaks up the current file into a token stream.
class Lexer {
public:
  explicit Lexer(const toolchain::SourceMgr &sourceMgr, MLIRContext *context,
                 AsmParserCodeCompleteContext *codeCompleteContext);

  const toolchain::SourceMgr &getSourceMgr() { return sourceMgr; }

  Token lexToken();

  /// Encode the specified source location information into a Location object
  /// for attachment to the IR or error reporting.
  Location getEncodedSourceLocation(SMLoc loc);

  /// Change the position of the lexer cursor.  The next token we lex will start
  /// at the designated point in the input.
  void resetPointer(const char *newPointer) { curPtr = newPointer; }

  /// Returns the start of the buffer.
  const char *getBufferBegin() { return curBuffer.data(); }

  /// Returns the end of the buffer.
  const char *getBufferEnd() { return curBuffer.end(); }

  /// Return the code completion location of the lexer, or nullptr if there is
  /// none.
  const char *getCodeCompleteLoc() const { return codeCompleteLoc; }

private:
  // Helpers.
  Token formToken(Token::Kind kind, const char *tokStart) {
    return Token(kind, StringRef(tokStart, curPtr - tokStart));
  }

  Token emitError(const char *loc, const Twine &message);

  // Lexer implementation methods.
  Token lexAtIdentifier(const char *tokStart);
  Token lexBareIdentifierOrKeyword(const char *tokStart);
  Token lexEllipsis(const char *tokStart);
  Token lexNumber(const char *tokStart);
  Token lexPrefixedIdentifier(const char *tokStart);
  Token lexString(const char *tokStart);

  /// Skip a comment line, starting with a '//'.
  void skipComment();

  const toolchain::SourceMgr &sourceMgr;
  MLIRContext *context;

  StringRef curBuffer;
  const char *curPtr;

  /// An optional code completion point within the input file, used to indicate
  /// the position of a code completion token.
  const char *codeCompleteLoc;

  Lexer(const Lexer &) = delete;
  void operator=(const Lexer &) = delete;
};

} // namespace mlir

#endif // MLIR_LIB_ASMPARSER_LEXER_H
