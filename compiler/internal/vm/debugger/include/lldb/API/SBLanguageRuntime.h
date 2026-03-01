//===-- SBLanguageRuntime.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBLANGUAGERUNTIME_H
#define LLDB_API_SBLANGUAGERUNTIME_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class SBLanguageRuntime {
public:
  static lldb::LanguageType GetLanguageTypeFromString(const char *string);

  static const char *GetNameForLanguageType(lldb::LanguageType language);

  /// Returns whether the given language is any version of C++.
  static bool LanguageIsCPlusPlus(lldb::LanguageType language);

  /// Returns whether the given language is Obj-C or Obj-C++.
  static bool LanguageIsObjC(lldb::LanguageType language);

  /// Returns whether the given language is any version of C, C++ or Obj-C.
  static bool LanguageIsCFamily(lldb::LanguageType language);

  /// Returns whether the given language supports exception breakpoints on
  /// throw statements.
  static bool SupportsExceptionBreakpointsOnThrow(lldb::LanguageType language);

  /// Returns whether the given language supports exception breakpoints on
  /// catch statements.
  static bool SupportsExceptionBreakpointsOnCatch(lldb::LanguageType language);

  /// Returns the keyword used for throw statements in the given language, e.g.
  /// Python uses \b raise. Returns \b nullptr if the language is not supported.
  static const char *GetThrowKeywordForLanguage(lldb::LanguageType language);

  /// Returns the keyword used for catch statements in the given language, e.g.
  /// Python uses \b except. Returns \b nullptr if the language is not
  /// supported.
  static const char *GetCatchKeywordForLanguage(lldb::LanguageType language);
};

} // namespace lldb

#endif // LLDB_API_SBLANGUAGERUNTIME_H
