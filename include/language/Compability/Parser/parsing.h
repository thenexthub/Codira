//===-- language/Compability/Parser/parsing.h --------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_PARSER_PARSING_H_
#define LANGUAGE_COMPABILITY_PARSER_PARSING_H_

#include "instrumented-parser.h"
#include "message.h"
#include "options.h"
#include "parse-tree.h"
#include "provenance.h"
#include "language/Compability/Parser/preprocessor.h"
#include "toolchain/Support/raw_ostream.h"
#include <optional>
#include <string>

namespace language::Compability::parser {

class Parsing {
public:
  explicit Parsing(AllCookedSources &);
  ~Parsing();

  bool consumedWholeFile() const { return consumedWholeFile_; }
  const char *finalRestingPlace() const { return finalRestingPlace_; }
  AllCookedSources &allCooked() { return allCooked_; }
  const AllCookedSources &allCooked() const { return allCooked_; }
  Messages &messages() { return messages_; }
  std::optional<Program> &parseTree() { return parseTree_; }

  const CookedSource &cooked() const { return DEREF(currentCooked_); }

  const SourceFile *Prescan(const std::string &path, Options);
  void EmitPreprocessedSource(
      toolchain::raw_ostream &, bool lineDirectives = true) const;
  void EmitPreprocessorMacros(toolchain::raw_ostream &) const;
  void DumpCookedChars(toolchain::raw_ostream &) const;
  void DumpProvenance(toolchain::raw_ostream &) const;
  void DumpParsingLog(toolchain::raw_ostream &) const;
  void Parse(toolchain::raw_ostream &debugOutput);
  void ClearLog();

  void EmitMessage(toolchain::raw_ostream &o, const char *at,
      const std::string &message, const std::string &prefix,
      toolchain::raw_ostream::Colors color = toolchain::raw_ostream::SAVEDCOLOR,
      bool echoSourceLine = false) const {
    allCooked_.allSources().EmitMessage(o,
        allCooked_.GetProvenanceRange(CharBlock(at)), message, prefix, color,
        echoSourceLine);
  }

private:
  Options options_;
  AllCookedSources &allCooked_;
  CookedSource *currentCooked_{nullptr};
  Messages messages_;
  bool consumedWholeFile_{false};
  const char *finalRestingPlace_{nullptr};
  std::optional<Program> parseTree_;
  ParsingLog log_;
  Preprocessor preprocessor_{allCooked_.allSources()};
};
} // namespace language::Compability::parser
#endif // FORTRAN_PARSER_PARSING_H_
