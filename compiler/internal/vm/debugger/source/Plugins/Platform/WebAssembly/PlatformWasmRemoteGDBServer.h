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

#ifndef LLDB_SOURCE_PLUGINS_PLATFORM_WASM_PLATFORMWASMREMOTEGDBSERVER_H
#define LLDB_SOURCE_PLUGINS_PLATFORM_WASM_PLATFORMWASMREMOTEGDBSERVER_H

#include "Plugins/Platform/gdb-server/PlatformRemoteGDBServer.h"

namespace lldb_private {

class PlatformWasmRemoteGDBServer
    : public platform_gdb_server::PlatformRemoteGDBServer {
public:
  PlatformWasmRemoteGDBServer() = default;

  ~PlatformWasmRemoteGDBServer() override;

  virtual llvm::StringRef GetDefaultProcessPluginName() const override;

private:
  PlatformWasmRemoteGDBServer(const PlatformWasmRemoteGDBServer &) = delete;
  const PlatformWasmRemoteGDBServer &
  operator=(const PlatformWasmRemoteGDBServer &) = delete;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PLATFORM_WASM_PLATFORMWASMREMOTEGDBSERVER_H
