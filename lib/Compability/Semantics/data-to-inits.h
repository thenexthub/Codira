//===-- lib/Semantics/data-to-inits.h -------------------------------------===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_DATA_TO_INITS_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_DATA_TO_INITS_H_

#include "language/Compability/Common/interval.h"
#include "language/Compability/Evaluate/fold-designator.h"
#include "language/Compability/Evaluate/initial-image.h"
#include "language/Compability/Support/default-kinds.h"
#include <list>
#include <map>

namespace language::Compability::parser {
struct DataStmtSet;
struct DataStmtValue;
} // namespace language::Compability::parser
namespace language::Compability::evaluate {
class ExpressionAnalyzer;
}
namespace language::Compability::semantics {

class Symbol;

struct SymbolDataInitialization {
  using Range = common::Interval<common::ConstantSubscript>;
  explicit SymbolDataInitialization(std::size_t bytes) : image{bytes} {}
  SymbolDataInitialization(SymbolDataInitialization &&) = default;

  void NoteInitializedRange(Range range) {
    if (initializedRanges.empty() ||
        !initializedRanges.back().AnnexIfPredecessor(range)) {
      initializedRanges.emplace_back(range);
    }
  }
  void NoteInitializedRange(
      common::ConstantSubscript offset, std::size_t size) {
    NoteInitializedRange(Range{offset, size});
  }
  void NoteInitializedRange(evaluate::OffsetSymbol offsetSymbol) {
    NoteInitializedRange(offsetSymbol.offset(), offsetSymbol.size());
  }

  evaluate::InitialImage image;
  std::list<Range> initializedRanges;
};

using DataInitializations = std::map<const Symbol *, SymbolDataInitialization>;

// Matches DATA statement variables with their values and checks
// compatibility.
void AccumulateDataInitializations(DataInitializations &,
    evaluate::ExpressionAnalyzer &, const parser::DataStmtSet &);

// For legacy DATA-style initialization extension: integer n(2)/1,2/
void AccumulateDataInitializations(DataInitializations &,
    evaluate::ExpressionAnalyzer &, const Symbol &,
    const std::list<common::Indirection<parser::DataStmtValue>> &);

void ConvertToInitializers(
    DataInitializations &, evaluate::ExpressionAnalyzer &);

} // namespace language::Compability::semantics
#endif // FORTRAN_SEMANTICS_DATA_TO_INITS_H_
