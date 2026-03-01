//===- MCAsmParserExtension.cpp - Asm Parser Hooks ------------------------===//
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

#include "vm/core/MC/MCParser/MCAsmParserExtension.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCParser/AsmLexer.h"
#include "vm/core/MC/MCStreamer.h"

using namespace vm::core;

MCAsmParserExtension::MCAsmParserExtension() = default;

MCAsmParserExtension::~MCAsmParserExtension() = default;

void MCAsmParserExtension::Initialize(MCAsmParser &Parser) {
  this->Parser = &Parser;
}

/// parseDirectiveCGProfile
///  ::= .cg_profile identifier, identifier, <number>
bool MCAsmParserExtension::parseDirectiveCGProfile(StringRef, SMLoc) {
  StringRef From;
  SMLoc FromLoc = getLexer().getLoc();
  if (getParser().parseIdentifier(From))
    return TokError("expected identifier in directive");

  if (getLexer().isNot(AsmToken::Comma))
    return TokError("expected a comma");
  Lex();

  StringRef To;
  SMLoc ToLoc = getLexer().getLoc();
  if (getParser().parseIdentifier(To))
    return TokError("expected identifier in directive");

  if (getLexer().isNot(AsmToken::Comma))
    return TokError("expected a comma");
  Lex();

  int64_t Count;
  if (getParser().parseIntToken(Count))
    return true;

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  MCSymbol *FromSym = getContext().parseSymbol(From);
  MCSymbol *ToSym = getContext().parseSymbol(To);

  getStreamer().emitCGProfileEntry(
      MCSymbolRefExpr::create(FromSym, getContext(), FromLoc),
      MCSymbolRefExpr::create(ToSym, getContext(), ToLoc), Count);
  return false;
}

bool MCAsmParserExtension::maybeParseUniqueID(int64_t &UniqueID) {
  AsmLexer &L = getLexer();
  if (L.isNot(AsmToken::Comma))
    return false;
  Lex();
  StringRef UniqueStr;
  if (getParser().parseIdentifier(UniqueStr))
    return TokError("expected identifier");
  if (UniqueStr != "unique")
    return TokError("expected 'unique'");
  if (L.isNot(AsmToken::Comma))
    return TokError("expected commma");
  Lex();
  if (getParser().parseAbsoluteExpression(UniqueID))
    return true;
  if (UniqueID < 0)
    return TokError("unique id must be positive");
  if (!isUInt<32>(UniqueID) || UniqueID == ~0U)
    return TokError("unique id is too large");
  return false;
}
