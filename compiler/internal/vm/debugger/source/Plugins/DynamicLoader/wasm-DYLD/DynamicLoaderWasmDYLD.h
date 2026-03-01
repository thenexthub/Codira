//===-- DynamicLoaderWasmDYLD.h ---------------------------------*- C++ -*-===//
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

#ifndef liblldb_Plugins_DynamicLoaderWasmDYLD_h_
#define liblldb_Plugins_DynamicLoaderWasmDYLD_h_

#include "lldb/Target/DynamicLoader.h"

namespace lldb_private {
namespace wasm {

class DynamicLoaderWasmDYLD : public DynamicLoader {
public:
  DynamicLoaderWasmDYLD(Process *process);

  static void Initialize();
  static void Terminate() {}

  static llvm::StringRef GetPluginNameStatic() { return "wasm-dyld"; }
  static llvm::StringRef GetPluginDescriptionStatic();

  static DynamicLoader *CreateInstance(Process *process, bool force);

  /// DynamicLoader
  /// \{
  void DidAttach() override;
  void DidLaunch() override {}
  Status CanLoadImage() override { return Status(); }
  lldb::ThreadPlanSP GetStepThroughTrampolinePlan(Thread &thread,
                                                  bool stop) override;
  lldb::ModuleSP LoadModuleAtAddress(const lldb_private::FileSpec &file,
                                     lldb::addr_t link_map_addr,
                                     lldb::addr_t base_addr,
                                     bool base_addr_is_offset) override;

  /// \}

  /// PluginInterface protocol.
  /// \{
  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }
  /// \}
};

} // namespace wasm
} // namespace lldb_private

#endif // liblldb_Plugins_DynamicLoaderWasmDYLD_h_
