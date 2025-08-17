//===- DebugSupport.cpp -----------------------------------------*- C++ -*-===//
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
//  This file defines functions which generate more readable forms of data
//  structures used in the dataflow analyses, for debugging purposes.
//
//===----------------------------------------------------------------------===//

#include <utility>

#include "language/Core/Analysis/FlowSensitive/DebugSupport.h"
#include "language/Core/Analysis/FlowSensitive/Solver.h"
#include "language/Core/Analysis/FlowSensitive/Value.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/ErrorHandling.h"

namespace language::Core {
namespace dataflow {

toolchain::StringRef debugString(Value::Kind Kind) {
  switch (Kind) {
  case Value::Kind::Integer:
    return "Integer";
  case Value::Kind::Pointer:
    return "Pointer";
  case Value::Kind::AtomicBool:
    return "AtomicBool";
  case Value::Kind::TopBool:
    return "TopBool";
  case Value::Kind::FormulaBool:
    return "FormulaBool";
  }
  toolchain_unreachable("Unhandled value kind");
}

toolchain::raw_ostream &operator<<(toolchain::raw_ostream &OS,
                              Solver::Result::Assignment Assignment) {
  switch (Assignment) {
  case Solver::Result::Assignment::AssignedFalse:
    return OS << "False";
  case Solver::Result::Assignment::AssignedTrue:
    return OS << "True";
  }
  toolchain_unreachable("Booleans can only be assigned true/false");
}

toolchain::StringRef debugString(Solver::Result::Status Status) {
  switch (Status) {
  case Solver::Result::Status::Satisfiable:
    return "Satisfiable";
  case Solver::Result::Status::Unsatisfiable:
    return "Unsatisfiable";
  case Solver::Result::Status::TimedOut:
    return "TimedOut";
  }
  toolchain_unreachable("Unhandled SAT check result status");
}

toolchain::raw_ostream &operator<<(toolchain::raw_ostream &OS, const Solver::Result &R) {
  OS << debugString(R.getStatus()) << "\n";
  if (auto Solution = R.getSolution()) {
    std::vector<std::pair<Atom, Solver::Result::Assignment>> Sorted = {
        Solution->begin(), Solution->end()};
    toolchain::sort(Sorted);
    for (const auto &Entry : Sorted)
      OS << Entry.first << " = " << Entry.second << "\n";
  }
  return OS;
}

} // namespace dataflow
} // namespace language::Core
