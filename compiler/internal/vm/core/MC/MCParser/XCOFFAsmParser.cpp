//===- XCOFFAsmParser.cpp - XCOFF Assembly Parser
//-----------------------------===//
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

#include "vm/core/BinaryFormat/XCOFF.h"
#include "vm/core/MC/MCParser/MCAsmParser.h"
#include "vm/core/MC/MCParser/MCAsmParserExtension.h"

using namespace vm::core;

namespace {

class XCOFFAsmParser : public MCAsmParserExtension {
  MCAsmParser *Parser = nullptr;
  AsmLexer *Lexer = nullptr;

  template <bool (XCOFFAsmParser::*HandlerMethod)(StringRef, SMLoc)>
  void addDirectiveHandler(StringRef Directive) {
    MCAsmParser::ExtensionDirectiveHandler Handler =
        std::make_pair(this, HandleDirective<XCOFFAsmParser, HandlerMethod>);

    getParser().addDirectiveHandler(Directive, Handler);
  }

public:
  XCOFFAsmParser() = default;

  void Initialize(MCAsmParser &P) override {
    Parser = &P;
    Lexer = &Parser->getLexer();
    // Call the base implementation.
    MCAsmParserExtension::Initialize(*Parser);

    addDirectiveHandler<&XCOFFAsmParser::ParseDirectiveCSect>(".csect");
  }
  bool ParseDirectiveCSect(StringRef, SMLoc);
};

} // end anonymous namespace

// .csect QualName [, Number ]
bool XCOFFAsmParser::ParseDirectiveCSect(StringRef, SMLoc) {
  report_fatal_error("XCOFFAsmParser directive not yet supported!");
  return false;
}

MCAsmParserExtension *toolchain::createXCOFFAsmParser() {
  return new XCOFFAsmParser;
}
