//===-- ReportRetriever.h ---------------------------------------*- C++ -*-===//
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

#include "lldb/Target/Process.h"

#ifndef LLDB_SOURCE_PLUGINS_INSTRUMENTATIONRUNTIME_UTILITY_REPORTRETRIEVER_H
#define LLDB_SOURCE_PLUGINS_INSTRUMENTATIONRUNTIME_UTILITY_REPORTRETRIEVER_H

namespace lldb_private {

class ReportRetriever {
private:
  static StructuredData::ObjectSP
  RetrieveReportData(const lldb::ProcessSP process_sp);

  static std::string FormatDescription(StructuredData::ObjectSP report);

public:
  static bool NotifyBreakpointHit(lldb::ProcessSP process_sp,
                                  StoppointCallbackContext *context,
                                  lldb::user_id_t break_id,
                                  lldb::user_id_t break_loc_id);

  static Breakpoint *SetupBreakpoint(lldb::ModuleSP, lldb::ProcessSP,
                                     ConstString);
};
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_INSTRUMENTATIONRUNTIME_UTILITY_REPORTRETRIEVER_H
