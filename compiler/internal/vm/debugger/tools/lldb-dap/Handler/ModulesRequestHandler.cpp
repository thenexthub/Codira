//===-- ModulesRequestHandler.cpp -----------------------------------------===//
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

#include "DAP.h"
#include "ProtocolUtils.h"
#include "RequestHandler.h"

using namespace lldb_dap::protocol;
namespace lldb_dap {

/// Modules can be retrieved from the debug adapter with this request which can
/// either return all modules or a range of modules to support paging.
///
/// Clients should only call this request if the corresponding capability
/// `supportsModulesRequest` is true.
llvm::Expected<ModulesResponseBody>
ModulesRequestHandler::Run(const std::optional<ModulesArguments> &args) const {
  ModulesResponseBody response;

  std::vector<Module> &modules = response.modules;
  std::lock_guard<std::mutex> guard(dap.modules_mutex);
  const uint32_t total_modules = dap.target.GetNumModules();
  response.totalModules = total_modules;

  modules.reserve(total_modules);
  for (uint32_t i = 0; i < total_modules; i++) {
    lldb::SBModule module = dap.target.GetModuleAtIndex(i);

    std::optional<Module> result = CreateModule(dap.target, module);
    if (result && !result->id.empty()) {
      dap.modules.insert(result->id);
      modules.emplace_back(std::move(result).value());
    }
  }

  return response;
}

} // namespace lldb_dap
