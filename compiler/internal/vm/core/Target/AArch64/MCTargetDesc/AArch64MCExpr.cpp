//===-- AArch64MCExpr.cpp - AArch64 specific MC expression classes --------===//
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

#include "AArch64MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCStreamer.h"

using namespace vm::core;

const AArch64AuthMCExpr *AArch64AuthMCExpr::create(const MCExpr *Expr,
                                                   uint16_t Discriminator,
                                                   AArch64PACKey::ID Key,
                                                   bool HasAddressDiversity,
                                                   MCContext &Ctx, SMLoc Loc) {
  return new (Ctx)
      AArch64AuthMCExpr(Expr, Discriminator, Key, HasAddressDiversity, Loc);
}
