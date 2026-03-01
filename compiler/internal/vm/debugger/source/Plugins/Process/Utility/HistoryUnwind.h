//===-- HistoryUnwind.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_HISTORYUNWIND_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_HISTORYUNWIND_H

#include <vector>

#include "lldb/Target/Unwind.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class HistoryUnwind : public lldb_private::Unwind {
public:
  HistoryUnwind(Thread &thread, std::vector<lldb::addr_t> pcs,
                HistoryPCType pc_type = HistoryPCType::Returns);

  ~HistoryUnwind() override;

protected:
  void DoClear() override;

  lldb::RegisterContextSP
  DoCreateRegisterContextForFrame(StackFrame *frame) override;

  bool DoGetFrameInfoAtIndex(uint32_t frame_idx, lldb::addr_t &cfa,
                             lldb::addr_t &pc,
                             bool &behaves_like_zeroth_frame) override;
  uint32_t DoGetFrameCount() override;

private:
  std::vector<lldb::addr_t> m_pcs;
  HistoryPCType m_pc_type;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_HISTORYUNWIND_H
