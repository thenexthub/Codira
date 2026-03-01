//===-- CompileUnitsRequestHandler.cpp ------------------------------------===//
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
#include "EventHelper.h"
#include "Protocol/ProtocolRequests.h"
#include "RequestHandler.h"
#include "lldb/Host/PosixApi.h" // IWYU pragma: keep

using namespace lldb_dap;
using namespace lldb_dap::protocol;

static CompileUnit CreateCompileUnit(lldb::SBCompileUnit &unit) {
  char unit_path_arr[PATH_MAX];
  unit.GetFileSpec().GetPath(unit_path_arr, sizeof(unit_path_arr));
  std::string unit_path(unit_path_arr);
  return {std::move(unit_path)};
}

/// The `compileUnits` request returns an array of path of compile units for
/// given module specified by `moduleId`.
llvm::Expected<CompileUnitsResponseBody> CompileUnitsRequestHandler::Run(
    const std::optional<CompileUnitsArguments> &args) const {
  std::vector<CompileUnit> units;
  int num_modules = dap.target.GetNumModules();
  for (int i = 0; i < num_modules; i++) {
    auto curr_module = dap.target.GetModuleAtIndex(i);
    if (args->moduleId == llvm::StringRef(curr_module.GetUUIDString())) {
      int num_units = curr_module.GetNumCompileUnits();
      for (int j = 0; j < num_units; j++) {
        auto curr_unit = curr_module.GetCompileUnitAtIndex(j);
        units.emplace_back(CreateCompileUnit(curr_unit));
      }
      break;
    }
  }
  return CompileUnitsResponseBody{std::move(units)};
}
