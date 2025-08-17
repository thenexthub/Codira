//===-- lib/Semantics/check-coarray.h ---------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_CHECK_COARRAY_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_CHECK_COARRAY_H_

#include "language/Compability/Semantics/semantics.h"
#include <list>

namespace language::Compability::semantics {

class CoarrayChecker : public virtual BaseChecker {
public:
  CoarrayChecker(SemanticsContext &context) : context_{context} {}
  void Leave(const parser::ChangeTeamStmt &);
  void Leave(const parser::EndChangeTeamStmt &);
  void Leave(const parser::SyncAllStmt &);
  void Leave(const parser::SyncImagesStmt &);
  void Leave(const parser::SyncMemoryStmt &);
  void Leave(const parser::SyncTeamStmt &);
  void Leave(const parser::NotifyWaitStmt &);
  void Leave(const parser::EventPostStmt &);
  void Leave(const parser::EventWaitStmt &);
  void Leave(const parser::LockStmt &);
  void Leave(const parser::UnlockStmt &);
  void Leave(const parser::CriticalStmt &);
  void Leave(const parser::ImageSelector &);
  void Leave(const parser::FormTeamStmt &);

  void Enter(const parser::CriticalConstruct &);
  void Enter(const parser::ChangeTeamConstruct &);

private:
  SemanticsContext &context_;

  void CheckNamesAreDistinct(const std::list<parser::CoarrayAssociation> &);
  void Say2(const parser::CharBlock &, parser::MessageFixedText &&,
      const parser::CharBlock &, parser::MessageFixedText &&);
};
} // namespace language::Compability::semantics
#endif // FORTRAN_SEMANTICS_CHECK_COARRAY_H_
