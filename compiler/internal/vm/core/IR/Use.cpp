//===-- Use.cpp - Implement the Use class ---------------------------------===//
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

#include "vm/core/IR/Use.h"
#include "vm/core/IR/User.h"

using namespace vm::core;

void Use::swap(Use &RHS) {
  if (Val == RHS.Val)
    return;

  std::swap(Val, RHS.Val);
  std::swap(Next, RHS.Next);
  std::swap(Prev, RHS.Prev);

  if (Prev)
    *Prev = this;

  if (Next)
    Next->Prev = &Next;

  if (RHS.Prev)
    *RHS.Prev = &RHS;

  if (RHS.Next)
    RHS.Next->Prev = &RHS.Next;
}

unsigned Use::getOperandNo() const {
  return this - getUser()->op_begin();
}

void Use::zap(Use *Start, const Use *Stop, bool del) {
  while (Start != Stop)
    (--Stop)->~Use();
  if (del)
    ::operator delete(Start);
}
