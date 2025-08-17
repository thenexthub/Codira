//===--- NoSanitizeList.cpp - Ignored list for sanitizers ----------------===//
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
// User-provided ignore-list used to disable/alter instrumentation done in
// sanitizers.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/NoSanitizeList.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/Basic/SanitizerSpecialCaseList.h"
#include "language/Core/Basic/Sanitizers.h"
#include "language/Core/Basic/SourceManager.h"

using namespace language::Core;

NoSanitizeList::NoSanitizeList(const std::vector<std::string> &NoSanitizePaths,
                               SourceManager &SM)
    : SSCL(SanitizerSpecialCaseList::createOrDie(
          NoSanitizePaths, SM.getFileManager().getVirtualFileSystem())),
      SM(SM) {}

NoSanitizeList::~NoSanitizeList() = default;

bool NoSanitizeList::containsPrefix(SanitizerMask Mask, StringRef Prefix,
                                    StringRef Name, StringRef Category) const {
  std::pair<unsigned, unsigned> NoSan =
      SSCL->inSectionBlame(Mask, Prefix, Name, Category);
  if (NoSan == toolchain::SpecialCaseList::NotFound)
    return false;
  std::pair<unsigned, unsigned> San =
      SSCL->inSectionBlame(Mask, Prefix, Name, "sanitize");
  // The statement evaluates to true under the following conditions:
  // 1. The string "prefix:*=sanitize" is absent.
  // 2. If "prefix:*=sanitize" is present, its (File Index, Line Number) is less
  // than that of "prefix:*".
  return San == toolchain::SpecialCaseList::NotFound || NoSan > San;
}

bool NoSanitizeList::containsGlobal(SanitizerMask Mask, StringRef GlobalName,
                                    StringRef Category) const {
  return containsPrefix(Mask, "global", GlobalName, Category);
}

bool NoSanitizeList::containsType(SanitizerMask Mask, StringRef MangledTypeName,
                                  StringRef Category) const {
  return containsPrefix(Mask, "type", MangledTypeName, Category);
}

bool NoSanitizeList::containsFunction(SanitizerMask Mask,
                                      StringRef FunctionName) const {
  return containsPrefix(Mask, "fun", FunctionName, {});
}

bool NoSanitizeList::containsFile(SanitizerMask Mask, StringRef FileName,
                                  StringRef Category) const {
  return containsPrefix(Mask, "src", FileName, Category);
}

bool NoSanitizeList::containsMainFile(SanitizerMask Mask, StringRef FileName,
                                      StringRef Category) const {
  return containsPrefix(Mask, "mainfile", FileName, Category);
}

bool NoSanitizeList::containsLocation(SanitizerMask Mask, SourceLocation Loc,
                                      StringRef Category) const {
  return Loc.isValid() &&
         containsFile(Mask, SM.getFilename(SM.getFileLoc(Loc)), Category);
}
