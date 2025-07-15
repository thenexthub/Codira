//===--- ModuleDiagsConsumer.cpp - Print module differ diagnostics --*- C++ -*-===//
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
//  This file implements the ModuleDifferDiagsConsumer class, which displays
//  diagnostics from the module differ as text to an output.
//
//===----------------------------------------------------------------------===//

#include "language/APIDigester/ModuleDiagsConsumer.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticList.h"
#include "language/Basic/Assertions.h"

using namespace language;

static StringRef getCategoryName(DiagID ID) {
  switch(ID) {
  case DiagID::removed_decl:
    return "/* Removed Decls */";
  case DiagID::moved_decl:
  case DiagID::decl_kind_changed:
    return "/* Moved Decls */";
  case DiagID::renamed_decl:
  case DiagID::objc_name_change:
    return "/* Renamed Decls */";
  case DiagID::decl_attr_change:
  case DiagID::decl_new_attr:
  case DiagID::func_self_access_change:
  case DiagID::new_decl_without_intro:
    return "/* Decl Attribute changes */";
  case DiagID::default_arg_removed:
  case DiagID::decl_type_change:
  case DiagID::func_type_escaping_changed:
  case DiagID::param_ownership_change:
  case DiagID::type_witness_change:
    return "/* Type Changes */";
  case DiagID::raw_type_change:
    return "/* RawRepresentable Changes */";
  case DiagID::generic_sig_change:
  case DiagID::demangled_name_changed:
    return "/* Generic Signature Changes */";
  case DiagID::enum_case_added:
  case DiagID::decl_added:
  case DiagID::decl_reorder:
  case DiagID::var_has_fixed_order_change:
  case DiagID::func_has_fixed_order_change:
    return "/* Fixed-layout Type Changes */";
  case DiagID::conformance_added:
  case DiagID::conformance_removed:
  case DiagID::optional_req_changed:
  case DiagID::existing_conformance_added:
    return "/* Protocol Conformance Change */";
  case DiagID::default_associated_type_removed:
  case DiagID::protocol_req_added:
  case DiagID::decl_new_witness_table_entry:
    return "/* Protocol Requirement Change */";
  case DiagID::super_class_removed:
  case DiagID::super_class_changed:
  case DiagID::no_longer_open:
  case DiagID::desig_init_added:
  case DiagID::added_invisible_designated_init:
  case DiagID::not_inheriting_convenience_inits:
    return "/* Class Inheritance Change */";
  default:
    return "/* Others */";
  }
}

language::ide::api::
ModuleDifferDiagsConsumer::ModuleDifferDiagsConsumer(bool DiagnoseModuleDiff,
                                                     toolchain::raw_ostream &OS):
    PrintingDiagnosticConsumer(OS), OS(OS),
    DiagnoseModuleDiff(DiagnoseModuleDiff) {
#define DIAG(KIND, ID, Group, Options, Text, Signature)                        \
  auto ID = getCategoryName(DiagID::ID);                                       \
  assert(!ID.empty());                                                         \
  AllDiags[ID] = std::set<std::string>();
#include "language/AST/DiagnosticsModuleDiffer.def"
}

void language::ide::api::ModuleDifferDiagsConsumer::handleDiagnostic(
    SourceManager &SM, const DiagnosticInfo &Info) {
  auto Category = getCategoryName(Info.ID);
  if (Category.empty()) {
    PrintingDiagnosticConsumer::handleDiagnostic(SM, Info);
    return;
  }
  if (!DiagnoseModuleDiff)
    return;
  toolchain::SmallString<256> Text;
  {
    toolchain::raw_svector_ostream Out(Text);
    DiagnosticEngine::formatDiagnosticText(Out, Info.FormatString,
                                           Info.FormatArgs);
  }
  AllDiags[Category].insert(Text.str().str());
}

language::ide::api::ModuleDifferDiagsConsumer::~ModuleDifferDiagsConsumer() {
  for (auto &Pair: AllDiags) {
    OS << "\n";
    OS << Pair.first << "\n";
    for (auto &Item: Pair.second) {
      OS << Item << "\n";
    }
  }
}

void language::ide::api::FilteringDiagnosticConsumer::flush() {
  for (auto &consumer: subConsumers) {
    consumer->flush();
  }
}

void language::ide::api::
FilteringDiagnosticConsumer::informDriverOfIncompleteBatchModeCompilation() {
  for (auto &consumer: subConsumers) {
    consumer->informDriverOfIncompleteBatchModeCompilation();
  }
}

bool language::ide::api::FilteringDiagnosticConsumer::finishProcessing() {
  for (auto &consumer: subConsumers) {
    consumer->finishProcessing();
  }
  return false;
}

bool language::ide::api::FilteringDiagnosticConsumer::shouldProceed(const DiagnosticInfo &Info) {
  if (allowedBreakages->empty()) {
    return true;
  }
  toolchain::SmallString<256> Text;
  {
    toolchain::raw_svector_ostream Out(Text);
    DiagnosticEngine::formatDiagnosticText(Out, Info.FormatString,
                                           Info.FormatArgs);
  }
  return allowedBreakages->count(Text.str()) == 0;
}

void language::ide::api::
FilteringDiagnosticConsumer::handleDiagnostic(SourceManager &SM,
                                              const DiagnosticInfo &RawInfo) {
  if (shouldProceed(RawInfo)) {
    DiagnosticInfo Info = RawInfo;
    if (DowngradeToWarning && Info.Kind == DiagnosticKind::Error) {
      Info.Kind = DiagnosticKind::Warning;
    }
    if (Info.Kind == DiagnosticKind::Error) {
      HasError = true;
    }
    for (auto &consumer: subConsumers) {
      consumer->handleDiagnostic(SM, Info);
    }
  }
}
