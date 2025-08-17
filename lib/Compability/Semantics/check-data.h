//===-------lib/Semantics/check-data.h ------------------------------------===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_CHECK_DATA_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_CHECK_DATA_H_

#include "data-to-inits.h"
#include "language/Compability/Common/interval.h"
#include "language/Compability/Evaluate/fold-designator.h"
#include "language/Compability/Evaluate/initial-image.h"
#include "language/Compability/Semantics/expression.h"
#include "language/Compability/Semantics/semantics.h"
#include <list>
#include <map>
#include <vector>

namespace language::Compability::parser {
struct DataStmtRepeat;
struct DataStmtObject;
struct DataIDoObject;
class DataStmtImpliedDo;
struct DataStmtSet;
} // namespace language::Compability::parser

namespace language::Compability::semantics {

class DataChecker : public virtual BaseChecker {
public:
  explicit DataChecker(SemanticsContext &context) : exprAnalyzer_{context} {}
  void Leave(const parser::DataStmtObject &);
  void Leave(const parser::DataIDoObject &);
  void Enter(const parser::DataImpliedDo &);
  void Leave(const parser::DataImpliedDo &);
  void Leave(const parser::DataStmtSet &);
  // These cases are for legacy DATA-like /initializations/
  void Leave(const parser::ComponentDecl &);
  void Leave(const parser::EntityDecl &);

  // After all DATA statements have been processed, converts their
  // initializations into per-symbol static initializers.
  void CompileDataInitializationsIntoInitializers();

private:
  ConstantSubscript GetRepetitionCount(const parser::DataStmtRepeat &);
  template <typename T> void CheckIfConstantSubscript(const T &);
  void CheckSubscript(const parser::SectionSubscript &);
  bool CheckAllSubscriptsInDataRef(const parser::DataRef &, parser::CharBlock);
  template <typename A> void LegacyDataInit(const A &);

  DataInitializations inits_;
  evaluate::ExpressionAnalyzer exprAnalyzer_;
  bool currentSetHasFatalErrors_{false};
};
} // namespace language::Compability::semantics
#endif // FORTRAN_SEMANTICS_CHECK_DATA_H_
