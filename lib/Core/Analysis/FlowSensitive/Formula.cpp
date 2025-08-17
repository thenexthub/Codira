//===- Formula.cpp ----------------------------------------------*- C++ -*-===//
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

#include "language/Core/Analysis/FlowSensitive/Formula.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/ErrorHandling.h"
#include <cassert>

namespace language::Core::dataflow {

const Formula &Formula::create(toolchain::BumpPtrAllocator &Alloc, Kind K,
                               ArrayRef<const Formula *> Operands,
                               unsigned Value) {
  assert(Operands.size() == numOperands(K));
  if (Value != 0) // Currently, formulas have values or operands, not both.
    assert(numOperands(K) == 0);
  void *Mem = Alloc.Allocate(sizeof(Formula) +
                                 Operands.size() * sizeof(Operands.front()),
                             alignof(Formula));
  Formula *Result = new (Mem) Formula();
  Result->FormulaKind = K;
  Result->Value = Value;
  // Operands are stored as `const Formula *`s after the formula itself.
  // We don't need to construct an object as pointers are trivial types.
  // Formula is alignas(const Formula *), so alignment is satisfied.
  toolchain::copy(Operands, reinterpret_cast<const Formula **>(Result + 1));
  return *Result;
}

static toolchain::StringLiteral sigil(Formula::Kind K) {
  switch (K) {
  case Formula::AtomRef:
  case Formula::Literal:
    return "";
  case Formula::Not:
    return "!";
  case Formula::And:
    return " & ";
  case Formula::Or:
    return " | ";
  case Formula::Implies:
    return " => ";
  case Formula::Equal:
    return " = ";
  }
  toolchain_unreachable("unhandled formula kind");
}

void Formula::print(toolchain::raw_ostream &OS, const AtomNames *Names) const {
  if (Names && kind() == AtomRef)
    if (auto It = Names->find(getAtom()); It != Names->end()) {
      OS << It->second;
      return;
    }

  switch (numOperands(kind())) {
  case 0:
    switch (kind()) {
    case AtomRef:
      OS << getAtom();
      break;
    case Literal:
      OS << (literal() ? "true" : "false");
      break;
    default:
      toolchain_unreachable("unhandled formula kind");
    }
    break;
  case 1:
    OS << sigil(kind());
    operands()[0]->print(OS, Names);
    break;
  case 2:
    OS << '(';
    operands()[0]->print(OS, Names);
    OS << sigil(kind());
    operands()[1]->print(OS, Names);
    OS << ')';
    break;
  default:
    toolchain_unreachable("unhandled formula arity");
  }
}

} // namespace language::Core::dataflow
