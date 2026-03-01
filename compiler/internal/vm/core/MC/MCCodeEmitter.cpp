//===- MCCodeEmitter.cpp - Instruction Encoding ---------------------------===//
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

#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
#include <string>

using namespace vm::core;

MCCodeEmitter::MCCodeEmitter() = default;

MCCodeEmitter::~MCCodeEmitter() = default;

void MCCodeEmitter::reportUnsupportedInst(const MCInst &Inst) {
  std::string Msg;
  raw_string_ostream OS(Msg);
  OS << "Unsupported instruction : " << Inst;
  reportFatalInternalError(Msg.c_str());
}

void MCCodeEmitter::reportUnsupportedOperand(const MCInst &Inst,
                                             unsigned OpNum) {
  std::string Msg;
  raw_string_ostream OS(Msg);
  OS << "Unsupported instruction operand : \"" << Inst << "\"[" << OpNum << "]";
  reportFatalInternalError(Msg.c_str());
}
