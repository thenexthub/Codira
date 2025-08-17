//===-- NoopLattice.h -------------------------------------------*- C++ -*-===//
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
//  This file defines the lattice with exactly one element.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_NOOP_LATTICE_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_NOOP_LATTICE_H

#include "language/Core/Analysis/FlowSensitive/DataflowLattice.h"
#include "language/Core/Support/Compiler.h"
#include "toolchain/ADT/Any.h"
#include <ostream>

namespace language::Core {
namespace dataflow {

/// Trivial lattice for dataflow analysis with exactly one element.
///
/// Useful for analyses that only need the Environment and nothing more.
class NoopLattice {
public:
  bool operator==(const NoopLattice &Other) const { return true; }

  LatticeJoinEffect join(const NoopLattice &Other) {
    return LatticeJoinEffect::Unchanged;
  }
};

inline std::ostream &operator<<(std::ostream &OS, const NoopLattice &) {
  return OS << "noop";
}

} // namespace dataflow
} // namespace language::Core

namespace toolchain {
// This needs to be exported for ClangAnalysisFlowSensitiveTests so any_cast
// uses the correct address of Any::TypeId from the clang shared library instead
// of creating one in the test executable. when building with
// CLANG_LINK_CLANG_DYLIB
extern template struct CLANG_TEMPLATE_ABI
    Any::TypeId<language::Core::dataflow::NoopLattice>;
} // namespace toolchain

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_NOOP_LATTICE_H
