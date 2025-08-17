//===--- ParserActions.cpp ------------------------------------------------===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Frontend/ParserActions.h"
#include "language/Compability/Frontend/CompilerInstance.h"
#include "language/Compability/Lower/Bridge.h"
#include "language/Compability/Lower/PFTBuilder.h"
#include "language/Compability/Parser/dump-parse-tree.h"
#include "language/Compability/Parser/parsing.h"
#include "language/Compability/Parser/provenance.h"
#include "language/Compability/Parser/source.h"
#include "language/Compability/Parser/unparse.h"
#include "language/Compability/Semantics/unparse-with-symbols.h"
#include "toolchain/Support/raw_ostream.h"

namespace language::Compability::frontend {

parser::AllCookedSources &getAllCooked(CompilerInstance &ci) {
  return ci.getParsing().allCooked();
}

void parseAndLowerTree(CompilerInstance &ci, lower::LoweringBridge &lb) {
  parser::Program &parseTree{*ci.getParsing().parseTree()};
  lb.lower(parseTree, ci.getSemanticsContext());
}

void dumpTree(CompilerInstance &ci) {
  auto &parseTree{ci.getParsing().parseTree()};
  toolchain::outs() << "========================";
  toolchain::outs() << " Flang: parse tree dump ";
  toolchain::outs() << "========================\n";
  parser::DumpTree(toolchain::outs(), parseTree, &ci.getInvocation().getAsFortran());
}

void dumpProvenance(CompilerInstance &ci) {
  ci.getParsing().DumpProvenance(toolchain::outs());
}

void dumpPreFIRTree(CompilerInstance &ci) {
  auto &parseTree{*ci.getParsing().parseTree()};

  if (auto ast{lower::createPFT(parseTree, ci.getSemanticsContext())}) {
    lower::dumpPFT(toolchain::outs(), *ast);
  } else {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error, "Pre FIR Tree is NULL.");
    ci.getDiagnostics().Report(diagID);
  }
}

void formatOrDumpPrescanner(std::string &buf,
                            toolchain::raw_string_ostream &outForPP,
                            CompilerInstance &ci) {
  if (ci.getInvocation().getPreprocessorOpts().showMacros) {
    ci.getParsing().EmitPreprocessorMacros(outForPP);
  } else if (ci.getInvocation().getPreprocessorOpts().noReformat) {
    ci.getParsing().DumpCookedChars(outForPP);
  } else {
    ci.getParsing().EmitPreprocessedSource(
        outForPP, !ci.getInvocation().getPreprocessorOpts().noLineDirectives);
  }

  // Print getDiagnostics from the prescanner
  ci.getParsing().messages().Emit(toolchain::errs(), ci.getAllCookedSources());
}

struct MeasurementVisitor {
  template <typename A>
  bool Pre(const A &) {
    return true;
  }
  template <typename A>
  void Post(const A &) {
    ++objects;
    bytes += sizeof(A);
  }
  size_t objects{0}, bytes{0};
};

void debugMeasureParseTree(CompilerInstance &ci, toolchain::StringRef filename) {
  // Parse. In case of failure, report and return.
  ci.getParsing().Parse(toolchain::outs());

  if ((ci.getParsing().parseTree().has_value() &&
       !ci.getParsing().consumedWholeFile()) ||
      (!ci.getParsing().messages().empty() &&
       (ci.getInvocation().getWarnAsErr() ||
        ci.getParsing().messages().AnyFatalError()))) {
    unsigned diagID = ci.getDiagnostics().getCustomDiagID(
        language::Core::DiagnosticsEngine::Error, "Could not parse %0");
    ci.getDiagnostics().Report(diagID) << filename;

    ci.getParsing().messages().Emit(toolchain::errs(), ci.getAllCookedSources());
    return;
  }

  // Report the getDiagnostics from parsing
  ci.getParsing().messages().Emit(toolchain::errs(), ci.getAllCookedSources());

  auto &parseTree{ci.getParsing().parseTree()};
  MeasurementVisitor visitor;
  parser::Walk(parseTree, visitor);
  toolchain::outs() << "Parse tree comprises " << visitor.objects
               << " objects and occupies " << visitor.bytes
               << " total bytes.\n";
}

void debugUnparseNoSema(CompilerInstance &ci, toolchain::raw_ostream &out) {
  auto &invoc = ci.getInvocation();
  auto &parseTree{ci.getParsing().parseTree()};

  // TODO: Options should come from CompilerInvocation
  Unparse(out, *parseTree, ci.getInvocation().getLangOpts(),
          /*encoding=*/parser::Encoding::UTF_8,
          /*capitalizeKeywords=*/true, /*backslashEscapes=*/false,
          /*preStatement=*/nullptr,
          invoc.getUseAnalyzedObjectsForUnparse() ? &invoc.getAsFortran()
                                                  : nullptr);
}

void debugUnparseWithSymbols(CompilerInstance &ci) {
  auto &parseTree{*ci.getParsing().parseTree()};

  semantics::UnparseWithSymbols(toolchain::outs(), parseTree,
                                ci.getInvocation().getLangOpts(),
                                /*encoding=*/parser::Encoding::UTF_8);
}

void debugUnparseWithModules(CompilerInstance &ci) {
  auto &parseTree{*ci.getParsing().parseTree()};
  semantics::UnparseWithModules(toolchain::outs(), ci.getSemantics().context(),
                                parseTree,
                                /*encoding=*/parser::Encoding::UTF_8);
}

void debugDumpParsingLog(CompilerInstance &ci) {
  ci.getParsing().Parse(toolchain::errs());
  ci.getParsing().DumpParsingLog(toolchain::outs());
}
} // namespace language::Compability::frontend
