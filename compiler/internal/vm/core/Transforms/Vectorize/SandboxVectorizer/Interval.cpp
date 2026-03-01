//===- Interval.cpp -------------------------------------------------------===//
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

#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Interval.h"
#include "vm/core/SandboxIR/Instruction.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/DependencyGraph.h"

namespace vm::core::sandboxir {

template <typename T> bool Interval<T>::disjoint(const Interval &Other) const {
  if (Other.empty())
    return true;
  if (empty())
    return true;
  return Other.Bottom->comesBefore(Top) || Bottom->comesBefore(Other.Top);
}

#ifndef NDEBUG
template <typename T> void Interval<T>::print(raw_ostream &OS) const {
  auto *Top = top();
  auto *Bot = bottom();
  OS << "Top: ";
  if (Top != nullptr)
    OS << *Top;
  else
    OS << "nullptr";
  OS << "\n";

  OS << "Bot: ";
  if (Bot != nullptr)
    OS << *Bot;
  else
    OS << "nullptr";
  OS << "\n";
}
template <typename T> void Interval<T>::dump() const { print(dbgs()); }
#endif

template class LLVM_EXPORT_TEMPLATE Interval<Instruction>;
template class LLVM_EXPORT_TEMPLATE Interval<MemDGNode>;

} // namespace vm::core::sandboxir
