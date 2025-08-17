//===--- CheckerRegistration.cpp - Registration for the Analyzer Checkers -===//
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
// Defines the registration function for the analyzer checkers.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Frontend/AnalyzerHelpFlags.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Frontend/CheckerRegistry.h"
#include <memory>

using namespace language::Core;
using namespace ento;

void ento::printCheckerHelp(raw_ostream &out, CompilerInstance &CI) {
  out << "OVERVIEW: Clang Static Analyzer Checkers List\n\n";
  out << "USAGE: -analyzer-checker <CHECKER or PACKAGE,...>\n\n";

  auto CheckerMgr = std::make_unique<CheckerManager>(
      CI.getAnalyzerOpts(), CI.getLangOpts(), CI.getDiagnostics(),
      CI.getFrontendOpts().Plugins);

  CheckerMgr->getCheckerRegistryData().printCheckerWithDescList(
      CI.getAnalyzerOpts(), out);
}

void ento::printEnabledCheckerList(raw_ostream &out, CompilerInstance &CI) {
  out << "OVERVIEW: Clang Static Analyzer Enabled Checkers List\n\n";

  auto CheckerMgr = std::make_unique<CheckerManager>(
      CI.getAnalyzerOpts(), CI.getLangOpts(), CI.getDiagnostics(),
      CI.getFrontendOpts().Plugins);

  CheckerMgr->getCheckerRegistryData().printEnabledCheckerList(out);
}

void ento::printCheckerConfigList(raw_ostream &out, CompilerInstance &CI) {

  auto CheckerMgr = std::make_unique<CheckerManager>(
      CI.getAnalyzerOpts(), CI.getLangOpts(), CI.getDiagnostics(),
      CI.getFrontendOpts().Plugins);

  CheckerMgr->getCheckerRegistryData().printCheckerOptionList(
      CI.getAnalyzerOpts(), out);
}

void ento::printAnalyzerConfigList(raw_ostream &out) {
  // FIXME: This message sounds scary, should be scary, but incorrectly states
  // that all configs are super dangerous. In reality, many of them should be
  // accessible to the user. We should create a user-facing subset of config
  // options under a different frontend flag.
  out << R"(
OVERVIEW: Clang Static Analyzer -analyzer-config Option List

The following list of configurations are meant for development purposes only, as
some of the variables they define are set to result in the most optimal
analysis. Setting them to other values may drastically change how the analyzer
behaves, and may even result in instabilities, crashes!

USAGE: -analyzer-config <OPTION1=VALUE,OPTION2=VALUE,...>
       -analyzer-config OPTION1=VALUE, -analyzer-config OPTION2=VALUE, ...
OPTIONS:
)";

  using OptionAndDescriptionTy = std::pair<StringRef, std::string>;
  OptionAndDescriptionTy PrintableOptions[] = {
#define ANALYZER_OPTION(TYPE, NAME, CMDFLAG, DESC, DEFAULT_VAL)                \
    {                                                                          \
      CMDFLAG,                                                                 \
      toolchain::Twine(toolchain::Twine() + "(" +                                        \
                  (StringRef(#TYPE) == "StringRef" ? "string" : #TYPE ) +      \
                  ") " DESC                                                    \
                  " (default: " #DEFAULT_VAL ")").str()                        \
    },

#define ANALYZER_OPTION_DEPENDS_ON_USER_MODE(TYPE, NAME, CMDFLAG, DESC,        \
                                             SHALLOW_VAL, DEEP_VAL)            \
    {                                                                          \
      CMDFLAG,                                                                 \
      toolchain::Twine(toolchain::Twine() + "(" +                                        \
                  (StringRef(#TYPE) == "StringRef" ? "string" : #TYPE ) +      \
                  ") " DESC                                                    \
                  " (default: " #SHALLOW_VAL " in shallow mode, " #DEEP_VAL    \
                  " in deep mode)").str()                                      \
    },
#include "language/Core/StaticAnalyzer/Core/AnalyzerOptions.def"
#undef ANALYZER_OPTION
#undef ANALYZER_OPTION_DEPENDS_ON_USER_MODE
  };

  toolchain::sort(PrintableOptions, toolchain::less_first());

  for (const auto &Pair : PrintableOptions) {
    AnalyzerOptions::printFormattedEntry(out, Pair, /*InitialPad*/ 2,
                                         /*EntryWidth*/ 30,
                                         /*MinLineWidth*/ 70);
    out << "\n\n";
  }
}
