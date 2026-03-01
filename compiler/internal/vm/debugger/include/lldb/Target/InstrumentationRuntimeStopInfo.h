//===-- InstrumentationRuntimeStopInfo.h ------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_INSTRUMENTATIONRUNTIMESTOPINFO_H
#define LLDB_TARGET_INSTRUMENTATIONRUNTIMESTOPINFO_H

#include <string>

#include "lldb/Target/StopInfo.h"
#include "lldb/Utility/StructuredData.h"

namespace lldb_private {

class InstrumentationRuntimeStopInfo : public StopInfo {
public:
  ~InstrumentationRuntimeStopInfo() override = default;

  lldb::StopReason GetStopReason() const override {
    return lldb::eStopReasonInstrumentation;
  }

  std::optional<uint32_t>
  GetSuggestedStackFrameIndex(bool inlined_stack) override;

  const char *GetDescription() override;

  bool DoShouldNotify(Event *event_ptr) override { return true; }

  static lldb::StopInfoSP CreateStopReasonWithInstrumentationData(
      Thread &thread, std::string description,
      StructuredData::ObjectSP additional_data);

private:
  InstrumentationRuntimeStopInfo(Thread &thread, std::string description,
                                 StructuredData::ObjectSP additional_data);
};

} // namespace lldb_private

#endif // LLDB_TARGET_INSTRUMENTATIONRUNTIMESTOPINFO_H
