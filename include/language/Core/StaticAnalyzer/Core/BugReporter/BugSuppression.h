//===- BugSuppression.h - Suppression interface -----------------*- C++ -*-===//
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
//  This file defines BugSuppression, a simple interface class encapsulating
//  all user provided in-code suppressions.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_BUGREPORTER_SUPPRESSION_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_BUGREPORTER_SUPPRESSION_H

#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallVector.h"

namespace language::Core {
class ASTContext;
class Decl;

namespace ento {
class BugReport;
class PathDiagnosticLocation;

class BugSuppression {
public:
  explicit BugSuppression(const ASTContext &ACtx) : ACtx(ACtx) {}

  using DiagnosticIdentifierList = toolchain::ArrayRef<toolchain::StringRef>;

  /// Return true if the given bug report was explicitly suppressed by the user.
  bool isSuppressed(const BugReport &);

  /// Return true if the bug reported at the given location was explicitly
  /// suppressed by the user.
  bool isSuppressed(const PathDiagnosticLocation &Location,
                    const Decl *DeclWithIssue,
                    DiagnosticIdentifierList DiagnosticIdentification);

private:
  // Overly pessimistic number, to be honest.
  static constexpr unsigned EXPECTED_NUMBER_OF_SUPPRESSIONS = 8;
  using CachedRanges =
      toolchain::SmallVector<SourceRange, EXPECTED_NUMBER_OF_SUPPRESSIONS>;

  toolchain::DenseMap<const Decl *, CachedRanges> CachedSuppressionLocations;

  const ASTContext &ACtx;
};

} // end namespace ento
} // end namespace language::Core

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_BUGREPORTER_SUPPRESSION_H
