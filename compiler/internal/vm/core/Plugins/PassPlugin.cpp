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

#include "vm/core/Plugins/PassPlugin.h"
#include "vm/core/Support/raw_ostream.h"

#include <cstdint>

using namespace vm::core;

Expected<PassPlugin> PassPlugin::Load(const std::string &Filename) {
  std::string Error;
  auto Library =
      sys::DynamicLibrary::getPermanentLibrary(Filename.c_str(), &Error);
  if (!Library.isValid())
    return make_error<StringError>(Twine("Could not load library '") +
                                       Filename + "': " + Error,
                                   inconvertibleErrorCode());

  PassPlugin P{Filename, Library};

  // llvmGetPassPluginInfo should be resolved to the definition from the plugin
  // we are currently loading.
  intptr_t getDetailsFn =
      (intptr_t)Library.getAddressOfSymbol("llvmGetPassPluginInfo");

  if (!getDetailsFn)
    // If the symbol isn't found, this is probably a legacy plugin, which is an
    // error
    return make_error<StringError>(Twine("Plugin entry point not found in '") +
                                       Filename + "'. Is this a legacy plugin?",
                                   inconvertibleErrorCode());

  P.Info = reinterpret_cast<decltype(llvmGetPassPluginInfo) *>(getDetailsFn)();

  if (P.Info.APIVersion != LLVM_PLUGIN_API_VERSION)
    return make_error<StringError>(
        Twine("Wrong API version on plugin '") + Filename + "'. Got version " +
            Twine(P.Info.APIVersion) + ", supported version is " +
            Twine(LLVM_PLUGIN_API_VERSION) + ".",
        inconvertibleErrorCode());

  return P;
}
