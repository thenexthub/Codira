//===-- DebuggerEvents.cpp ------------------------------------------------===//
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

#include "lldb/Core/DebuggerEvents.h"
#include "lldb/Core/Debugger.h"
#include "lldb/Core/Module.h"
#include "lldb/Core/Progress.h"
#include "llvm/Support/WithColor.h"

using namespace lldb_private;
using namespace lldb;

template <typename T>
static const T *GetEventDataFromEventImpl(const Event *event_ptr) {
  if (event_ptr)
    if (const EventData *event_data = event_ptr->GetData())
      if (event_data->GetFlavor() == T::GetFlavorString())
        return static_cast<const T *>(event_ptr->GetData());
  return nullptr;
}

llvm::StringRef ProgressEventData::GetFlavorString() {
  return "ProgressEventData";
}

llvm::StringRef ProgressEventData::GetFlavor() const {
  return ProgressEventData::GetFlavorString();
}

void ProgressEventData::Dump(Stream *s) const {
  s->Printf(" id = %" PRIu64 ", title = \"%s\"", m_id, m_title.c_str());
  if (!m_details.empty())
    s->Printf(", details = \"%s\"", m_details.c_str());
  if (m_completed == 0 || m_completed == m_total)
    s->Printf(", type = %s", m_completed == 0 ? "start" : "end");
  else
    s->PutCString(", type = update");
  // If m_total is UINT64_MAX, there is no progress to report, just "start"
  // and "end". If it isn't we will show the completed and total amounts.
  if (m_total != Progress::kNonDeterministicTotal)
    s->Printf(", progress = %" PRIu64 " of %" PRIu64, m_completed, m_total);
}

const ProgressEventData *
ProgressEventData::GetEventDataFromEvent(const Event *event_ptr) {
  return GetEventDataFromEventImpl<ProgressEventData>(event_ptr);
}

StructuredData::DictionarySP
ProgressEventData::GetAsStructuredData(const Event *event_ptr) {
  const ProgressEventData *progress_data =
      ProgressEventData::GetEventDataFromEvent(event_ptr);

  if (!progress_data)
    return {};

  auto dictionary_sp = std::make_shared<StructuredData::Dictionary>();
  dictionary_sp->AddStringItem("title", progress_data->GetTitle());
  dictionary_sp->AddStringItem("details", progress_data->GetDetails());
  dictionary_sp->AddStringItem("message", progress_data->GetMessage());
  dictionary_sp->AddIntegerItem("progress_id", progress_data->GetID());
  dictionary_sp->AddIntegerItem("completed", progress_data->GetCompleted());
  dictionary_sp->AddIntegerItem("total", progress_data->GetTotal());
  dictionary_sp->AddBooleanItem("debugger_specific",
                                progress_data->IsDebuggerSpecific());

  return dictionary_sp;
}

llvm::StringRef DiagnosticEventData::GetPrefix() const {
  switch (m_severity) {
  case Severity::eSeverityInfo:
    return "info";
  case Severity::eSeverityWarning:
    return "warning";
  case Severity::eSeverityError:
    return "error";
  }
  llvm_unreachable("Fully covered switch above!");
}

void DiagnosticEventData::Dump(Stream *s) const {
  llvm::HighlightColor color = m_severity == lldb::eSeverityWarning
                                   ? llvm::HighlightColor::Warning
                                   : llvm::HighlightColor::Error;
  llvm::WithColor(s->AsRawOstream(), color, llvm::ColorMode::Enable)
      << GetPrefix();
  *s << ": " << GetMessage() << '\n';
  s->Flush();
}

llvm::StringRef DiagnosticEventData::GetFlavorString() {
  return "DiagnosticEventData";
}

llvm::StringRef DiagnosticEventData::GetFlavor() const {
  return DiagnosticEventData::GetFlavorString();
}

const DiagnosticEventData *
DiagnosticEventData::GetEventDataFromEvent(const Event *event_ptr) {
  return GetEventDataFromEventImpl<DiagnosticEventData>(event_ptr);
}

StructuredData::DictionarySP
DiagnosticEventData::GetAsStructuredData(const Event *event_ptr) {
  const DiagnosticEventData *diagnostic_data =
      DiagnosticEventData::GetEventDataFromEvent(event_ptr);

  if (!diagnostic_data)
    return {};

  auto dictionary_sp = std::make_shared<StructuredData::Dictionary>();
  dictionary_sp->AddStringItem("message", diagnostic_data->GetMessage());
  dictionary_sp->AddStringItem("type", diagnostic_data->GetPrefix());
  dictionary_sp->AddBooleanItem("debugger_specific",
                                diagnostic_data->IsDebuggerSpecific());
  return dictionary_sp;
}

llvm::StringRef SymbolChangeEventData::GetFlavorString() {
  return "SymbolChangeEventData";
}

llvm::StringRef SymbolChangeEventData::GetFlavor() const {
  return SymbolChangeEventData::GetFlavorString();
}

const SymbolChangeEventData *
SymbolChangeEventData::GetEventDataFromEvent(const Event *event_ptr) {
  return GetEventDataFromEventImpl<SymbolChangeEventData>(event_ptr);
}

void SymbolChangeEventData::DoOnRemoval(Event *event_ptr) {
  DebuggerSP debugger_sp(m_debugger_wp.lock());
  if (!debugger_sp)
    return;

  for (TargetSP target_sp : debugger_sp->GetTargetList().Targets()) {
    if (ModuleSP module_sp =
            target_sp->GetImages().FindModule(m_module_spec.GetUUID())) {
      {
        std::lock_guard<std::recursive_mutex> guard(module_sp->GetMutex());
        if (!module_sp->GetSymbolFileFileSpec())
          module_sp->SetSymbolFileFileSpec(m_module_spec.GetSymbolFileSpec());
      }
      ModuleList module_list;
      module_list.Append(module_sp);
      target_sp->SymbolsDidLoad(module_list);
    }
  }
}
