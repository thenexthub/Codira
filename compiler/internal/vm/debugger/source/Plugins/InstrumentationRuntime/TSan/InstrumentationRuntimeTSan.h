//===-- InstrumentationRuntimeTSan.h ----------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_INSTRUMENTATIONRUNTIME_TSAN_INSTRUMENTATIONRUNTIMETSAN_H
#define LLDB_SOURCE_PLUGINS_INSTRUMENTATIONRUNTIME_TSAN_INSTRUMENTATIONRUNTIMETSAN_H

#include "lldb/Target/ABI.h"
#include "lldb/Target/InstrumentationRuntime.h"
#include "lldb/Utility/StructuredData.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class InstrumentationRuntimeTSan : public lldb_private::InstrumentationRuntime {
public:
  ~InstrumentationRuntimeTSan() override;

  static lldb::InstrumentationRuntimeSP
  CreateInstance(const lldb::ProcessSP &process_sp);

  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "ThreadSanitizer"; }

  static lldb::InstrumentationRuntimeType GetTypeStatic();

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  virtual lldb::InstrumentationRuntimeType GetType() { return GetTypeStatic(); }

  lldb::ThreadCollectionSP
  GetBacktracesFromExtendedStopInfo(StructuredData::ObjectSP info) override;

private:
  InstrumentationRuntimeTSan(const lldb::ProcessSP &process_sp)
      : lldb_private::InstrumentationRuntime(process_sp) {}

  const RegularExpression &GetPatternForRuntimeLibrary() override;

  bool CheckIfRuntimeIsValid(const lldb::ModuleSP module_sp) override;

  void Activate() override;

  void Deactivate();

  static bool NotifyBreakpointHit(void *baton,
                                  StoppointCallbackContext *context,
                                  lldb::user_id_t break_id,
                                  lldb::user_id_t break_loc_id);

  StructuredData::ObjectSP RetrieveReportData(ExecutionContextRef exe_ctx_ref);

  std::string FormatDescription(StructuredData::ObjectSP report);

  std::string GenerateSummary(StructuredData::ObjectSP report);

  lldb::addr_t GetMainRacyAddress(StructuredData::ObjectSP report);

  std::string GetLocationDescription(StructuredData::ObjectSP report,
                                     lldb::addr_t &global_addr,
                                     std::string &global_name,
                                     std::string &filename, uint32_t &line);

  lldb::addr_t GetFirstNonInternalFramePc(StructuredData::ObjectSP trace,
                                          bool skip_one_frame = false);
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_INSTRUMENTATIONRUNTIME_TSAN_INSTRUMENTATIONRUNTIMETSAN_H
