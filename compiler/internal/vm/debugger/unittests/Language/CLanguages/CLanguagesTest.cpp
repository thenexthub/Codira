//===-- CLanguagesTest.cpp ------------------------------------------------===//
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

#include "Plugins/Language/CPlusPlus/CPlusPlusLanguage.h"
#include "Plugins/Language/ObjC/ObjCLanguage.h"
#include "Plugins/Language/ObjCPlusPlus/ObjCPlusPlusLanguage.h"
#include "TestingSupport/SubsystemRAII.h"
#include "lldb/lldb-enumerations.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lldb_private;

/// Returns the name of the LLDB plugin for the given language or an empty
/// string if there is no fitting plugin.
static llvm::StringRef GetPluginName(lldb::LanguageType language) {
  Language *language_plugin = Language::FindPlugin(language);
  if (language_plugin)
    return language_plugin->GetPluginName();
  return "";
}

TEST(CLanguages, LookupCLanguagesByLanguageType) {
  SubsystemRAII<CPlusPlusLanguage, ObjCPlusPlusLanguage, ObjCLanguage> langs;

  // There is no plugin to find for C.
  EXPECT_EQ(Language::FindPlugin(lldb::eLanguageTypeC), nullptr);
  EXPECT_EQ(Language::FindPlugin(lldb::eLanguageTypeC89), nullptr);
  EXPECT_EQ(Language::FindPlugin(lldb::eLanguageTypeC99), nullptr);
  EXPECT_EQ(Language::FindPlugin(lldb::eLanguageTypeC11), nullptr);

  EXPECT_EQ(GetPluginName(lldb::eLanguageTypeC_plus_plus), "cplusplus");
  EXPECT_EQ(GetPluginName(lldb::eLanguageTypeC_plus_plus_03), "cplusplus");
  EXPECT_EQ(GetPluginName(lldb::eLanguageTypeC_plus_plus_11), "cplusplus");
  EXPECT_EQ(GetPluginName(lldb::eLanguageTypeC_plus_plus_14), "cplusplus");

  EXPECT_EQ(GetPluginName(lldb::eLanguageTypeObjC), "objc");

  EXPECT_EQ(GetPluginName(lldb::eLanguageTypeObjC_plus_plus), "objcplusplus");
}
