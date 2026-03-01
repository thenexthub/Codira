//==- AArch64PBQPRegAlloc.h - AArch64 specific PBQP constraints --*- C++ -*-==//
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

#ifndef LLVM_LIB_TARGET_AARCH64_AARCH64PBQPREGALOC_H
#define LLVM_LIB_TARGET_AARCH64_AARCH64PBQPREGALOC_H

#include "vm/core/ADT/SetVector.h"
#include "vm/core/CodeGen/PBQPRAConstraint.h"

namespace vm::core {

class TargetRegisterInfo;

/// Add the accumulator chaining constraint to a PBQP graph
class A57ChainingConstraint : public PBQPRAConstraint {
public:
  // Add A57 specific constraints to the PBQP graph.
  void apply(PBQPRAGraph &G) override;

private:
  SmallSetVector<unsigned, 32> Chains;
  const TargetRegisterInfo *TRI;

  // Add the accumulator chaining constraint, inside the chain, i.e. so that
  // parity(Rd) == parity(Ra).
  // \return true if a constraint was added
  bool addIntraChainConstraint(PBQPRAGraph &G, unsigned Rd, unsigned Ra);

  // Add constraints between existing chains
  void addInterChainConstraint(PBQPRAGraph &G, unsigned Rd, unsigned Ra);
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AARCH64_AARCH64PBQPREGALOC_H
