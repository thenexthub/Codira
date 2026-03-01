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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_WASM_PROCESSWASM_H
#define LLDB_SOURCE_PLUGINS_PROCESS_WASM_PROCESSWASM_H

#include "Plugins/Process/gdb-remote/ProcessGDBRemote.h"
#include "Utility/WasmVirtualRegisters.h"

namespace lldb_private {
namespace wasm {

/// Each WebAssembly module has separated address spaces for Code and Memory.
/// A WebAssembly module also has a Data section which, when the module is
/// loaded, gets mapped into a region in the module Memory.
enum WasmAddressType : uint8_t { Memory = 0x00, Object = 0x01, Invalid = 0xff };

/// For the purpose of debugging, we can represent all these separated 32-bit
/// address spaces with a single virtual 64-bit address space. The
/// wasm_addr_t provides this encoding using bitfields.
struct wasm_addr_t {
  uint64_t offset : 32;
  uint64_t module_id : 30;
  uint64_t type : 2;

  wasm_addr_t(lldb::addr_t addr)
      : offset(addr & 0x00000000ffffffff),
        module_id((addr & 0x00ffffff00000000) >> 32), type(addr >> 62) {}

  wasm_addr_t(WasmAddressType type, uint32_t module_id, uint32_t offset)
      : offset(offset), module_id(module_id), type(type) {}

  WasmAddressType GetType() { return static_cast<WasmAddressType>(type); }

  operator lldb::addr_t() { return *(uint64_t *)this; }
};

static_assert(sizeof(wasm_addr_t) == 8, "");

/// ProcessWasm provides the access to the Wasm program state
/// retrieved from the Wasm engine.
class ProcessWasm : public process_gdb_remote::ProcessGDBRemote {
public:
  ProcessWasm(lldb::TargetSP target_sp, lldb::ListenerSP listener_sp);
  ~ProcessWasm() override = default;

  static lldb::ProcessSP CreateInstance(lldb::TargetSP target_sp,
                                        lldb::ListenerSP listener_sp,
                                        const FileSpec *crash_file_path,
                                        bool can_connect);

  static void Initialize();
  static void DebuggerInitialize(Debugger &debugger);
  static void Terminate();

  static llvm::StringRef GetPluginNameStatic();
  static llvm::StringRef GetPluginDescriptionStatic();

  llvm::StringRef GetPluginName() override;

  size_t ReadMemory(lldb::addr_t vm_addr, void *buf, size_t size,
                    Status &error) override;

  bool CanDebug(lldb::TargetSP target_sp,
                bool plugin_specified_by_name) override;

  /// Retrieve the current call stack from the WebAssembly remote process.
  llvm::Expected<std::vector<lldb::addr_t>> GetWasmCallStack(lldb::tid_t tid);

  /// Query the value of a WebAssembly variable from the WebAssembly
  /// remote process.
  llvm::Expected<lldb::DataBufferSP>
  GetWasmVariable(WasmVirtualRegisterKinds kind, int frame_index, int index);

protected:
  std::shared_ptr<process_gdb_remote::ThreadGDBRemote>
  CreateThread(lldb::tid_t tid) override;

private:
  friend class UnwindWasm;
  friend class ThreadWasm;

  process_gdb_remote::GDBRemoteDynamicRegisterInfoSP &GetRegisterInfo() {
    return m_register_info_sp;
  }

  ProcessWasm(const ProcessWasm &);
  const ProcessWasm &operator=(const ProcessWasm &) = delete;
};

} // namespace wasm
} // namespace lldb_private

#endif
