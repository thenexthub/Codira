//===-- AVRMCExpr.cpp - AVR specific MC expression classes ----------------===//
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

#include "AVRMCExpr.h"

#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"

namespace vm::core {

const AVRMCExpr *AVRMCExpr::create(Specifier Kind, const MCExpr *Expr,
                                   bool Negated, MCContext &Ctx) {
  return new (Ctx) AVRMCExpr(Kind, Expr, Negated);
}

} // namespace vm::core
