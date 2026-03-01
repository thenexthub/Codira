//===- DiagnosticHandler.h - DiagnosticHandler class for LLVM -------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
//
//===----------------------------------------------------------------------===//
#include "vm/core/IR/DiagnosticHandler.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Regex.h"

using namespace vm::core;

namespace {

/// Regular expression corresponding to the value given in one of the
/// -pass-remarks* command line flags. Passes whose name matches this regexp
/// will emit a diagnostic when calling the associated diagnostic function
/// (emitOptimizationRemark, emitOptimizationRemarkMissed or
/// emitOptimizationRemarkAnalysis).
struct PassRemarksOpt {
  std::shared_ptr<Regex> Pattern;

  void operator=(const std::string &Val) {
    // Create a regexp object to match pass names for emitOptimizationRemark.
    if (!Val.empty()) {
      Pattern = std::make_shared<Regex>(Val);
      std::string RegexError;
      if (!Pattern->isValid(RegexError))
        report_fatal_error(Twine("Invalid regular expression '") + Val +
                               "' in -pass-remarks: " + RegexError,
                           false);
    }
  }
};
} // namespace

static PassRemarksOpt PassRemarksPassedOptLoc;
static PassRemarksOpt PassRemarksMissedOptLoc;
static PassRemarksOpt PassRemarksAnalysisOptLoc;

// -pass-remarks
//    Command line flag to enable emitOptimizationRemark()
static cl::opt<PassRemarksOpt, true, cl::parser<std::string>> PassRemarks(
    "pass-remarks", cl::value_desc("pattern"),
    cl::desc("Enable optimization remarks from passes whose name match "
             "the given regular expression"),
    cl::Hidden, cl::location(PassRemarksPassedOptLoc), cl::ValueRequired);

// -pass-remarks-missed
//    Command line flag to enable emitOptimizationRemarkMissed()
static cl::opt<PassRemarksOpt, true, cl::parser<std::string>> PassRemarksMissed(
    "pass-remarks-missed", cl::value_desc("pattern"),
    cl::desc("Enable missed optimization remarks from passes whose name match "
             "the given regular expression"),
    cl::Hidden, cl::location(PassRemarksMissedOptLoc), cl::ValueRequired);

// -pass-remarks-analysis
//    Command line flag to enable emitOptimizationRemarkAnalysis()
static cl::opt<PassRemarksOpt, true, cl::parser<std::string>>
    PassRemarksAnalysis(
        "pass-remarks-analysis", cl::value_desc("pattern"),
        cl::desc(
            "Enable optimization analysis remarks from passes whose name match "
            "the given regular expression"),
        cl::Hidden, cl::location(PassRemarksAnalysisOptLoc), cl::ValueRequired);

bool DiagnosticHandler::isAnalysisRemarkEnabled(StringRef PassName) const {
  return (PassRemarksAnalysisOptLoc.Pattern &&
          PassRemarksAnalysisOptLoc.Pattern->match(PassName));
}
bool DiagnosticHandler::isMissedOptRemarkEnabled(StringRef PassName) const {
  return (PassRemarksMissedOptLoc.Pattern &&
          PassRemarksMissedOptLoc.Pattern->match(PassName));
}
bool DiagnosticHandler::isPassedOptRemarkEnabled(StringRef PassName) const {
  return (PassRemarksPassedOptLoc.Pattern &&
          PassRemarksPassedOptLoc.Pattern->match(PassName));
}

bool DiagnosticHandler::isAnyRemarkEnabled() const {
  return (PassRemarksPassedOptLoc.Pattern || PassRemarksMissedOptLoc.Pattern ||
          PassRemarksAnalysisOptLoc.Pattern);
}
