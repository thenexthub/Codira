//===-- language/Compability/Parser/options.h --------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_PARSER_OPTIONS_H_
#define LANGUAGE_COMPABILITY_PARSER_OPTIONS_H_

#include "characters.h"
#include "language/Compability/Support/Fortran-features.h"

#include <optional>
#include <string>
#include <vector>

namespace language::Compability::parser {

struct Options {
  Options() {}

  using Predefinition = std::pair<std::string, std::optional<std::string>>;

  bool isFixedForm{false};
  int fixedFormColumns{72};
  common::LanguageFeatureControl features;
  std::vector<std::string> searchDirectories;
  std::vector<std::string> intrinsicModuleDirectories;
  std::vector<Predefinition> predefinitions;
  bool instrumentedParse{false};
  bool isModuleFile{false};
  bool needProvenanceRangeToCharBlockMappings{false};
  language::Compability::parser::Encoding encoding{language::Compability::parser::Encoding::UTF_8};
  bool prescanAndReformat{false}; // -E
  bool expandIncludeLinesInPreprocessedOutput{true};
  bool showColors{false};
};

} // namespace language::Compability::parser

#endif // FORTRAN_PARSER_OPTIONS_H_
