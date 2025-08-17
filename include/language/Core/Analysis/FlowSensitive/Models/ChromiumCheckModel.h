//===-- ChromiumCheckModel.h ------------------------------------*- C++ -*-===//
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
// This file defines a dataflow model for Chromium's family of CHECK functions.
//
//===----------------------------------------------------------------------===//
#ifndef CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_CHROMIUMCHECKMODEL_H
#define CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_CHROMIUMCHECKMODEL_H

#include "language/Core/AST/DeclCXX.h"
#include "language/Core/Analysis/FlowSensitive/DataflowAnalysis.h"
#include "language/Core/Analysis/FlowSensitive/DataflowEnvironment.h"
#include "toolchain/ADT/DenseSet.h"

namespace language::Core {
namespace dataflow {

/// Models the behavior of Chromium's CHECK, DCHECK, etc. macros, so that code
/// after a call to `*CHECK` can rely on the condition being true.
class ChromiumCheckModel : public DataflowModel {
public:
  ChromiumCheckModel() = default;
  bool transfer(const CFGElement &Element, Environment &Env) override;

private:
  /// Declarations for `::logging::CheckError::.*Check`, lazily initialized.
  toolchain::SmallDenseSet<const CXXMethodDecl *> CheckDecls;
};

} // namespace dataflow
} // namespace language::Core

#endif // CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_CHROMIUMCHECKMODEL_H
