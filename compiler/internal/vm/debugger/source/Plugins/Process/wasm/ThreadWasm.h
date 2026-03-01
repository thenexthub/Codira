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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_WASM_THREADWASM_H
#define LLDB_SOURCE_PLUGINS_PROCESS_WASM_THREADWASM_H

#include "Plugins/Process/gdb-remote/ThreadGDBRemote.h"

namespace lldb_private {
namespace wasm {

/// ProcessWasm provides the access to the Wasm program state
/// retrieved from the Wasm engine.
class ThreadWasm : public process_gdb_remote::ThreadGDBRemote {
public:
  ThreadWasm(Process &process, lldb::tid_t tid)
      : process_gdb_remote::ThreadGDBRemote(process, tid) {}
  ~ThreadWasm() override = default;

  /// Retrieve the current call stack from the WebAssembly remote process.
  llvm::Expected<std::vector<lldb::addr_t>> GetWasmCallStack();

  lldb::RegisterContextSP
  CreateRegisterContextForFrame(StackFrame *frame) override;

protected:
  Unwind &GetUnwinder() override;

  ThreadWasm(const ThreadWasm &);
  const ThreadWasm &operator=(const ThreadWasm &) = delete;
};

} // namespace wasm
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PROCESS_WASM_THREADWASM_H
