//===-- language/Compability/Parser/user-state.h -----------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_PARSER_USER_STATE_H_
#define LANGUAGE_COMPABILITY_PARSER_USER_STATE_H_

// Instances of ParseState (parse-state.h) incorporate instances of this
// UserState class, which encapsulates any semantic information necessary for
// parse tree construction so as to avoid any need for representing
// state in static data.

#include "language/Compability/Common/idioms.h"
#include "language/Compability/Parser/char-block.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Support/Fortran-features.h"
#include "toolchain/Support/raw_ostream.h"
#include <cinttypes>
#include <optional>
#include <set>
#include <unordered_map>

namespace language::Compability::parser {

class AllCookedSources;
class ParsingLog;
class ParseState;

class Success {}; // for when one must return something that's present

class UserState {
public:
  UserState(const AllCookedSources &allCooked,
      common::LanguageFeatureControl features)
      : allCooked_{allCooked}, features_{features} {}

  const AllCookedSources &allCooked() const { return allCooked_; }
  const common::LanguageFeatureControl &features() const { return features_; }

  toolchain::raw_ostream *debugOutput() const { return debugOutput_; }
  UserState &set_debugOutput(toolchain::raw_ostream &out) {
    debugOutput_ = &out;
    return *this;
  }

  ParsingLog *log() const { return log_; }
  UserState &set_log(ParsingLog *log) {
    log_ = log;
    return *this;
  }

  bool instrumentedParse() const { return instrumentedParse_; }
  UserState &set_instrumentedParse(bool yes) {
    instrumentedParse_ = yes;
    return *this;
  }

  void NewSubprogram() {
    doLabels_.clear();
    nonlabelDoConstructNestingDepth_ = 0;
    oldStructureComponents_.clear();
  }

  using Label = std::uint64_t;
  bool IsDoLabel(Label label) const {
    auto iter{doLabels_.find(label)};
    return iter != doLabels_.end() &&
        iter->second >= nonlabelDoConstructNestingDepth_;
  }
  void NewDoLabel(Label label) {
    doLabels_[label] = nonlabelDoConstructNestingDepth_;
  }

  void EnterNonlabelDoConstruct() { ++nonlabelDoConstructNestingDepth_; }
  void LeaveDoConstruct() {
    if (nonlabelDoConstructNestingDepth_ > 0) {
      --nonlabelDoConstructNestingDepth_;
    }
  }

  void NoteOldStructureComponent(const CharBlock &name) {
    oldStructureComponents_.insert(name);
  }
  bool IsOldStructureComponent(const CharBlock &name) const {
    return oldStructureComponents_.find(name) != oldStructureComponents_.end();
  }

private:
  const AllCookedSources &allCooked_;

  toolchain::raw_ostream *debugOutput_{nullptr};

  ParsingLog *log_{nullptr};
  bool instrumentedParse_{false};

  std::unordered_map<Label, int> doLabels_;
  int nonlabelDoConstructNestingDepth_{0};

  std::set<CharBlock> oldStructureComponents_;

  common::LanguageFeatureControl features_;
};

// Definitions of parser classes that manipulate the UserState.
struct StartNewSubprogram {
  using resultType = Success;
  static std::optional<Success> Parse(ParseState &);
};

struct CapturedLabelDoStmt {
  using resultType = Statement<common::Indirection<LabelDoStmt>>;
  static std::optional<resultType> Parse(ParseState &);
};

struct EndDoStmtForCapturedLabelDoStmt {
  using resultType = Statement<common::Indirection<EndDoStmt>>;
  static std::optional<resultType> Parse(ParseState &);
};

struct EnterNonlabelDoConstruct {
  using resultType = Success;
  static std::optional<Success> Parse(ParseState &);
};

struct LeaveDoConstruct {
  using resultType = Success;
  static std::optional<Success> Parse(ParseState &);
};

struct OldStructureComponentName {
  using resultType = Name;
  static std::optional<Name> Parse(ParseState &);
};

struct StructureComponents {
  using resultType = DataComponentDefStmt;
  static std::optional<DataComponentDefStmt> Parse(ParseState &);
};

struct NestedStructureStmt {
  using resultType = StructureStmt;
  static std::optional<StructureStmt> Parse(ParseState &);
};
} // namespace language::Compability::parser
#endif // FORTRAN_PARSER_USER_STATE_H_
