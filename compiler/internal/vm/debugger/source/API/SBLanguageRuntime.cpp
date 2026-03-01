//===-- SBLanguageRuntime.cpp ---------------------------------------------===//
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

#include "lldb/API/SBLanguageRuntime.h"
#include "lldb/Target/Language.h"
#include "lldb/Utility/Instrumentation.h"

using namespace lldb;
using namespace lldb_private;

lldb::LanguageType
SBLanguageRuntime::GetLanguageTypeFromString(const char *string) {
  LLDB_INSTRUMENT_VA(string);

  return Language::GetLanguageTypeFromString(llvm::StringRef(string));
}

const char *
SBLanguageRuntime::GetNameForLanguageType(lldb::LanguageType language) {
  LLDB_INSTRUMENT_VA(language);

  return Language::GetNameForLanguageType(language);
}

bool SBLanguageRuntime::LanguageIsCPlusPlus(lldb::LanguageType language) {
  return Language::LanguageIsCPlusPlus(language);
}

bool SBLanguageRuntime::LanguageIsObjC(lldb::LanguageType language) {
  return Language::LanguageIsObjC(language);
}

bool SBLanguageRuntime::LanguageIsCFamily(lldb::LanguageType language) {
  return Language::LanguageIsCFamily(language);
}

bool SBLanguageRuntime::SupportsExceptionBreakpointsOnThrow(
    lldb::LanguageType language) {
  if (Language *lang_plugin = Language::FindPlugin(language))
    return lang_plugin->SupportsExceptionBreakpointsOnThrow();
  return false;
}

bool SBLanguageRuntime::SupportsExceptionBreakpointsOnCatch(
    lldb::LanguageType language) {
  if (Language *lang_plugin = Language::FindPlugin(language))
    return lang_plugin->SupportsExceptionBreakpointsOnCatch();
  return false;
}

const char *
SBLanguageRuntime::GetThrowKeywordForLanguage(lldb::LanguageType language) {
  if (Language *lang_plugin = Language::FindPlugin(language))
    return ConstString(lang_plugin->GetThrowKeyword()).AsCString();
  return nullptr;
}

const char *
SBLanguageRuntime::GetCatchKeywordForLanguage(lldb::LanguageType language) {
  if (Language *lang_plugin = Language::FindPlugin(language))
    return ConstString(lang_plugin->GetCatchKeyword()).AsCString();
  return nullptr;
}
