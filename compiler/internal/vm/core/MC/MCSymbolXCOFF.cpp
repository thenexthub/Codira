//===- lib/MC/MCSymbolXCOFF.cpp - XCOFF Code Symbol Representation --------===//
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

#include "vm/core/MC/MCSectionXCOFF.h"

using namespace vm::core;

MCSectionXCOFF *MCSymbolXCOFF::getRepresentedCsect() const {
  assert(RepresentedCsect &&
         "Trying to get csect representation of this symbol but none was set.");
  assert(getSymbolTableName() == RepresentedCsect->getSymbolTableName() &&
         "SymbolTableNames need to be the same for this symbol and its csect "
         "representation.");
  return RepresentedCsect;
}

void MCSymbolXCOFF::setRepresentedCsect(MCSectionXCOFF *C) {
  assert(C && "Assigned csect should not be null.");
  assert((!RepresentedCsect || RepresentedCsect == C) &&
         "Trying to set a csect that doesn't match the one that this symbol is "
         "already mapped to.");
  assert(getSymbolTableName() == C->getSymbolTableName() &&
         "SymbolTableNames need to be the same for this symbol and its csect "
         "representation.");
  RepresentedCsect = C;
}
