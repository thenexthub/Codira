//===--- ModuleDiagsConsumer.h - Print module differ diagnostics --*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  This file defines the ModuleDifferDiagsConsumer class, which displays
//  diagnostics from the module differ as text to an output.
//
//===----------------------------------------------------------------------===//

#ifndef __LANGUAGE_MODULE_DIFFER_DIAGS_CONSUMER_H__
#define __LANGUAGE_MODULE_DIFFER_DIAGS_CONSUMER_H__

#include "language/AST/DiagnosticConsumer.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "toolchain/ADT/MapVector.h"
#include "toolchain/ADT/StringSet.h"
#include <set>

namespace language {
namespace ide {
namespace api {

/// Diagnostic consumer that displays diagnostics to standard output.
class ModuleDifferDiagsConsumer: public PrintingDiagnosticConsumer {
  toolchain::raw_ostream &OS;
  bool DiagnoseModuleDiff;
  toolchain::MapVector<StringRef, std::set<std::string>> AllDiags;
public:
  ModuleDifferDiagsConsumer(bool DiagnoseModuleDiff,
                            toolchain::raw_ostream &OS = toolchain::errs());
  ~ModuleDifferDiagsConsumer();
  void handleDiagnostic(SourceManager &SM, const DiagnosticInfo &Info) override;
};

class FilteringDiagnosticConsumer: public DiagnosticConsumer {
  bool HasError = false;
  std::vector<std::unique_ptr<DiagnosticConsumer>> subConsumers;
  std::unique_ptr<toolchain::StringSet<>> allowedBreakages;
  bool DowngradeToWarning;
  bool shouldProceed(const DiagnosticInfo &Info);
public:
  FilteringDiagnosticConsumer(std::vector<std::unique_ptr<DiagnosticConsumer>> subConsumers,
                              std::unique_ptr<toolchain::StringSet<>> allowedBreakages,
                              bool DowngradeToWarning):
    subConsumers(std::move(subConsumers)),
    allowedBreakages(std::move(allowedBreakages)),
    DowngradeToWarning(DowngradeToWarning) {}
  ~FilteringDiagnosticConsumer() = default;

  void flush() override;
  void informDriverOfIncompleteBatchModeCompilation() override;
  bool finishProcessing() override;
  bool hasError() const { return HasError; }

  void handleDiagnostic(SourceManager &SM,
                        const DiagnosticInfo &Info) override;
};
}
}
}

#endif
