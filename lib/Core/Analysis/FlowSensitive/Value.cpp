//===-- Value.cpp -----------------------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//  This file defines support functions for the `Value` type.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Analysis/FlowSensitive/Value.h"

namespace language::Core {
namespace dataflow {

static bool areEquivalentIndirectionValues(const Value &Val1,
                                           const Value &Val2) {
  if (auto *IndVal1 = dyn_cast<PointerValue>(&Val1)) {
    auto *IndVal2 = cast<PointerValue>(&Val2);
    return &IndVal1->getPointeeLoc() == &IndVal2->getPointeeLoc();
  }
  return false;
}

bool areEquivalentValues(const Value &Val1, const Value &Val2) {
  if (&Val1 == &Val2)
    return true;
  if (Val1.getKind() != Val2.getKind())
    return false;
  // If values are distinct and have properties, we don't consider them equal,
  // leaving equality up to the user model.
  if (!Val1.properties().empty() || !Val2.properties().empty())
    return false;
  if (isa<TopBoolValue>(&Val1))
    return true;
  return areEquivalentIndirectionValues(Val1, Val2);
}

raw_ostream &operator<<(raw_ostream &OS, const Value &Val) {
  switch (Val.getKind()) {
  case Value::Kind::Integer:
    return OS << "Integer(@" << &Val << ")";
  case Value::Kind::Pointer:
    return OS << "Pointer(" << &cast<PointerValue>(Val).getPointeeLoc() << ")";
  case Value::Kind::TopBool:
    return OS << "TopBool(" << cast<TopBoolValue>(Val).getAtom() << ")";
  case Value::Kind::AtomicBool:
    return OS << "AtomicBool(" << cast<AtomicBoolValue>(Val).getAtom() << ")";
  case Value::Kind::FormulaBool:
    return OS << "FormulaBool(" << cast<FormulaBoolValue>(Val).formula() << ")";
  }
  toolchain_unreachable("Unknown language::Core::dataflow::Value::Kind enum");
}

} // namespace dataflow
} // namespace language::Core
