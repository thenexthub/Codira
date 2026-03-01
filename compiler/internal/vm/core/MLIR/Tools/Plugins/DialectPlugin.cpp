//===- lib/Tools/Plugins/DialectPlugin.cpp - Load Dialect Plugins ---------===//
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

#include "mlir/Tools/Plugins/DialectPlugin.h"
#include "vm/core/Support/raw_ostream.h"

#include <cstdint>

using namespace mlir;

toolchain::Expected<DialectPlugin> DialectPlugin::load(const std::string &filename) {
  std::string error;
  auto library =
      toolchain::sys::DynamicLibrary::getPermanentLibrary(filename.c_str(), &error);
  if (!library.isValid())
    return toolchain::make_error<toolchain::StringError>(
        Twine("Could not load library '") + filename + "': " + error,
        toolchain::inconvertibleErrorCode());

  DialectPlugin plugin{filename, library};

  // mlirGetDialectPluginInfo should be resolved to the definition from the
  // plugin we are currently loading.
  intptr_t getDetailsFn =
      (intptr_t)library.getAddressOfSymbol("mlirGetDialectPluginInfo");

  if (!getDetailsFn)
    return toolchain::make_error<toolchain::StringError>(
        Twine("Plugin entry point not found in '") + filename,
        toolchain::inconvertibleErrorCode());

  plugin.info =
      reinterpret_cast<decltype(mlirGetDialectPluginInfo) *>(getDetailsFn)();

  if (plugin.info.apiVersion != MLIR_PLUGIN_API_VERSION)
    return toolchain::make_error<toolchain::StringError>(
        Twine("Wrong API version on plugin '") + filename + "'. Got version " +
            Twine(plugin.info.apiVersion) + ", supported version is " +
            Twine(MLIR_PLUGIN_API_VERSION) + ".",
        toolchain::inconvertibleErrorCode());

  if (!plugin.info.registerDialectRegistryCallbacks)
    return toolchain::make_error<toolchain::StringError>(
        Twine("Empty entry callback in plugin '") + filename + "'.'",
        toolchain::inconvertibleErrorCode());

  return plugin;
}
