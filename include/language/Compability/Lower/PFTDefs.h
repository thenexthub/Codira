//===-- Lower/PFTDefs.h -- shared PFT info ----------------------*- C++ -*-===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_PFTDEFS_H
#define LANGUAGE_COMPABILITY_LOWER_PFTDEFS_H

#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallSet.h"
#include "toolchain/ADT/StringRef.h"

namespace mlir {
class Block;
}

namespace language::Compability {
namespace semantics {
class Symbol;
class SemanticsContext;
class Scope;
} // namespace semantics

namespace evaluate {
template <typename A>
class Expr;
struct SomeType;
} // namespace evaluate

namespace common {
template <typename A>
class Reference;
}

namespace lower {

bool definedInCommonBlock(const semantics::Symbol &sym);
bool symbolIsGlobal(const semantics::Symbol &sym);
bool defaultRecursiveFunctionSetting();

namespace pft {

struct Evaluation;

using SomeExpr = language::Compability::evaluate::Expr<language::Compability::evaluate::SomeType>;
using SymbolRef = language::Compability::common::Reference<const language::Compability::semantics::Symbol>;
using Label = std::uint64_t;
using LabelSet = toolchain::SmallSet<Label, 4>;
using SymbolLabelMap = toolchain::DenseMap<SymbolRef, LabelSet>;
using LabelEvalMap = toolchain::DenseMap<Label, Evaluation *>;

} // namespace pft
} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_PFTDEFS_H
