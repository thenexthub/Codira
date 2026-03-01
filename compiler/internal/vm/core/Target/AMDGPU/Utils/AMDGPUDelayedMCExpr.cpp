//===- AMDGPUDelayedMCExpr.cpp - Delayed MCExpr resolve ---------*- C++ -*-===//
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

#include "AMDGPUDelayedMCExpr.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCValue.h"

using namespace vm::core;

static msgpack::DocNode getNode(msgpack::DocNode DN, msgpack::Type Type,
                                MCValue Val) {
  msgpack::Document *Doc = DN.getDocument();
  switch (Type) {
  default:
    return Doc->getEmptyNode();
  case msgpack::Type::Int:
    return Doc->getNode(static_cast<int64_t>(Val.getConstant()));
  case msgpack::Type::UInt:
    return Doc->getNode(static_cast<uint64_t>(Val.getConstant()));
  case msgpack::Type::Boolean:
    return Doc->getNode(static_cast<bool>(Val.getConstant()));
  }
}

void DelayedMCExprs::assignDocNode(msgpack::DocNode &DN, msgpack::Type Type,
                                   const MCExpr *ExprValue) {
  MCValue Res;
  if (ExprValue->evaluateAsRelocatable(Res, nullptr)) {
    if (Res.isAbsolute()) {
      DN = getNode(DN, Type, Res);
      return;
    }
  }

  DelayedExprs.emplace_back(DN, Type, ExprValue);
}

bool DelayedMCExprs::resolveDelayedExpressions() {
  while (!DelayedExprs.empty()) {
    Expr DE = DelayedExprs.front();
    MCValue Res;

    if (!DE.ExprValue->evaluateAsRelocatable(Res, nullptr) || !Res.isAbsolute())
      return false;

    DelayedExprs.pop_front();
    DE.DN = getNode(DE.DN, DE.Type, Res);
  }

  return true;
}

void DelayedMCExprs::clear() { DelayedExprs.clear(); }

bool DelayedMCExprs::empty() { return DelayedExprs.empty(); }
