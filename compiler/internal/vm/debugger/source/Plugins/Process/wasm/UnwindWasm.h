//===----------------------------------------------------------------------===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_WASM_UNWINDWASM_H
#define LLDB_SOURCE_PLUGINS_PROCESS_WASM_UNWINDWASM_H

#include "lldb/Target/RegisterContext.h"
#include "lldb/Target/Unwind.h"
#include <vector>

namespace lldb_private {
namespace wasm {

/// UnwindWasm manages stack unwinding for a WebAssembly process.
class UnwindWasm : public lldb_private::Unwind {
public:
  UnwindWasm(lldb_private::Thread &thread) : Unwind(thread) {}
  ~UnwindWasm() override = default;

protected:
  void DoClear() override {
    m_frames.clear();
    m_unwind_complete = false;
  }

  uint32_t DoGetFrameCount() override;

  bool DoGetFrameInfoAtIndex(uint32_t frame_idx, lldb::addr_t &cfa,
                             lldb::addr_t &pc,
                             bool &behaves_like_zeroth_frame) override;

  lldb::RegisterContextSP
  DoCreateRegisterContextForFrame(lldb_private::StackFrame *frame) override;

private:
  std::vector<lldb::addr_t> m_frames;
  bool m_unwind_complete = false;

  UnwindWasm(const UnwindWasm &);
  const UnwindWasm &operator=(const UnwindWasm &) = delete;
};

} // namespace wasm
} // namespace lldb_private

#endif
