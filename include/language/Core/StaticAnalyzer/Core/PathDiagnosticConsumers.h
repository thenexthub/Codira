//===--- PathDiagnosticConsumers.h - Path Diagnostic Clients ----*- C++ -*-===//
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
//  This file defines the interface to create different path diagostic clients.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHDIAGNOSTICCONSUMERS_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHDIAGNOSTICCONSUMERS_H

#include "language/Core/Analysis/PathDiagnostic.h"

#include <string>
#include <vector>

namespace language::Core {

class MacroExpansionContext;
class Preprocessor;

namespace cross_tu {
class CrossTranslationUnitContext;
}

namespace ento {

class PathDiagnosticConsumer;
using PathDiagnosticConsumers =
    std::vector<std::unique_ptr<PathDiagnosticConsumer>>;

#define ANALYSIS_DIAGNOSTICS(NAME, CMDFLAG, DESC, CREATEFN)                    \
  void CREATEFN(PathDiagnosticConsumerOptions Diagopts,                        \
                PathDiagnosticConsumers &C, const std::string &Prefix,         \
                const Preprocessor &PP,                                        \
                const cross_tu::CrossTranslationUnitContext &CTU,              \
                const MacroExpansionContext &MacroExpansions);
#include "language/Core/StaticAnalyzer/Core/Analyses.def"

} // end 'ento' namespace
} // end 'clang' namespace

#endif
