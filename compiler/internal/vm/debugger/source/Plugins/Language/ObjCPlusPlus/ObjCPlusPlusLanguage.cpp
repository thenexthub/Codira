//===-- ObjCPlusPlusLanguage.cpp ------------------------------------------===//
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

#include "ObjCPlusPlusLanguage.h"

#include "lldb/Core/PluginManager.h"
#include "lldb/Utility/ConstString.h"

using namespace lldb;
using namespace lldb_private;

LLDB_PLUGIN_DEFINE(ObjCPlusPlusLanguage)

bool ObjCPlusPlusLanguage::IsSourceFile(llvm::StringRef file_path) const {
  const auto suffixes = {".h", ".mm"};
  for (auto suffix : suffixes) {
    if (file_path.ends_with_insensitive(suffix))
      return true;
  }
  return false;
}

void ObjCPlusPlusLanguage::Initialize() {
  PluginManager::RegisterPlugin(GetPluginNameStatic(), "Objective-C++ Language",
                                CreateInstance);
}

void ObjCPlusPlusLanguage::Terminate() {
  PluginManager::UnregisterPlugin(CreateInstance);
}

// Static Functions
Language *ObjCPlusPlusLanguage::CreateInstance(lldb::LanguageType language) {
  switch (language) {
  case lldb::eLanguageTypeObjC_plus_plus:
    return new ObjCPlusPlusLanguage();
  default:
    return nullptr;
  }
}

std::optional<bool>
ObjCPlusPlusLanguage::GetBooleanFromString(llvm::StringRef str) const {
  return llvm::StringSwitch<std::optional<bool>>(str)
      .Cases({"true", "YES"}, {true})
      .Cases({"false", "NO"}, {false})
      .Default({});
}
