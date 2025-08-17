//===--- NoSanitizeList.h - List of ignored entities for sanitizers --*- C++
//-*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//
// User-provided list of ignored entities used to disable/alter
// instrumentation done in sanitizers.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_BASIC_NOSANITIZELIST_H
#define LANGUAGE_CORE_BASIC_NOSANITIZELIST_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/StringRef.h"
#include <memory>
#include <vector>

namespace language::Core {

class SanitizerMask;
class SourceManager;
class SanitizerSpecialCaseList;

class NoSanitizeList {
  std::unique_ptr<SanitizerSpecialCaseList> SSCL;
  SourceManager &SM;
  bool containsPrefix(SanitizerMask Mask, StringRef Prefix, StringRef Name,
                      StringRef Category) const;

public:
  NoSanitizeList(const std::vector<std::string> &NoSanitizeListPaths,
                 SourceManager &SM);
  ~NoSanitizeList();
  bool containsGlobal(SanitizerMask Mask, StringRef GlobalName,
                      StringRef Category = StringRef()) const;
  bool containsType(SanitizerMask Mask, StringRef MangledTypeName,
                    StringRef Category = StringRef()) const;
  bool containsFunction(SanitizerMask Mask, StringRef FunctionName) const;
  bool containsFile(SanitizerMask Mask, StringRef FileName,
                    StringRef Category = StringRef()) const;
  bool containsMainFile(SanitizerMask Mask, StringRef FileName,
                        StringRef Category = StringRef()) const;
  bool containsLocation(SanitizerMask Mask, SourceLocation Loc,
                        StringRef Category = StringRef()) const;
};

} // end namespace language::Core

#endif
