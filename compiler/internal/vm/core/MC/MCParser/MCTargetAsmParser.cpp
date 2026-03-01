//===-- MCTargetAsmParser.cpp - Target Assembly Parser --------------------===//
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

#include "vm/core/MC/MCParser/MCTargetAsmParser.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCObjectStreamer.h"
#include "vm/core/MC/MCRegister.h"

using namespace vm::core;

MCTargetAsmParser::MCTargetAsmParser(MCTargetOptions const &MCOptions,
                                     const MCSubtargetInfo &STI,
                                     const MCInstrInfo &MII)
    : MCOptions(MCOptions), STI(&STI), MII(MII) {}

MCTargetAsmParser::~MCTargetAsmParser() = default;

MCSubtargetInfo &MCTargetAsmParser::copySTI() {
  MCSubtargetInfo &STICopy = getContext().getSubtargetCopy(getSTI());
  STI = &STICopy;
  // The returned STI will likely be modified. Create a new fragment to prevent
  // mixing STI values within a fragment.
  auto &S = getStreamer();
  if (S.isObj() && S.getCurrentFragment())
    static_cast<MCObjectStreamer &>(S).newFragment();
  return STICopy;
}

const MCSubtargetInfo &MCTargetAsmParser::getSTI() const {
  return *STI;
}

ParseStatus MCTargetAsmParser::parseDirective(AsmToken DirectiveID) {
  SMLoc StartTokLoc = getTok().getLoc();
  // Delegate to ParseDirective by default for transition period. Once the
  // transition is over, this method should just return NoMatch.
  bool Res = ParseDirective(DirectiveID);

  // Some targets erroneously report success after emitting an error.
  if (getParser().hasPendingError())
    return ParseStatus::Failure;

  // ParseDirective returns true if there was an error or if the directive is
  // not target-specific. Disambiguate the two cases by comparing position of
  // the lexer before and after calling the method: if no tokens were consumed,
  // there was no match, otherwise there was a failure.
  if (!Res)
    return ParseStatus::Success;
  if (getTok().getLoc() != StartTokLoc)
    return ParseStatus::Failure;
  return ParseStatus::NoMatch;
}

bool MCTargetAsmParser::areEqualRegs(const MCParsedAsmOperand &Op1,
                                     const MCParsedAsmOperand &Op2) const {
  return Op1.isReg() && Op2.isReg() && Op1.getReg() == Op2.getReg();
}
