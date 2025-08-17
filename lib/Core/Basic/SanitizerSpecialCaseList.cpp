//===--- SanitizerSpecialCaseList.cpp - SCL for sanitizers ----------------===//
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
// An extension of SpecialCaseList to allowing querying sections by
// SanitizerMask.
//
//===----------------------------------------------------------------------===//
#include "language/Core/Basic/SanitizerSpecialCaseList.h"
#include "toolchain/ADT/STLExtras.h"

using namespace language::Core;

std::unique_ptr<SanitizerSpecialCaseList>
SanitizerSpecialCaseList::create(const std::vector<std::string> &Paths,
                                 toolchain::vfs::FileSystem &VFS,
                                 std::string &Error) {
  std::unique_ptr<language::Core::SanitizerSpecialCaseList> SSCL(
      new SanitizerSpecialCaseList());
  if (SSCL->createInternal(Paths, VFS, Error)) {
    SSCL->createSanitizerSections();
    return SSCL;
  }
  return nullptr;
}

std::unique_ptr<SanitizerSpecialCaseList>
SanitizerSpecialCaseList::createOrDie(const std::vector<std::string> &Paths,
                                      toolchain::vfs::FileSystem &VFS) {
  std::string Error;
  if (auto SSCL = create(Paths, VFS, Error))
    return SSCL;
  toolchain::report_fatal_error(StringRef(Error));
}

void SanitizerSpecialCaseList::createSanitizerSections() {
  for (auto &S : Sections) {
    SanitizerMask Mask;

#define SANITIZER(NAME, ID)                                                    \
  if (S.SectionMatcher->match(NAME))                                           \
    Mask |= SanitizerKind::ID;
#define SANITIZER_GROUP(NAME, ID, ALIAS) SANITIZER(NAME, ID)

#include "language/Core/Basic/Sanitizers.def"
#undef SANITIZER
#undef SANITIZER_GROUP

    SanitizerSections.emplace_back(Mask, S.Entries, S.FileIdx);
  }
}

bool SanitizerSpecialCaseList::inSection(SanitizerMask Mask, StringRef Prefix,
                                         StringRef Query,
                                         StringRef Category) const {
  return inSectionBlame(Mask, Prefix, Query, Category) != NotFound;
}

std::pair<unsigned, unsigned>
SanitizerSpecialCaseList::inSectionBlame(SanitizerMask Mask, StringRef Prefix,
                                         StringRef Query,
                                         StringRef Category) const {
  for (const auto &S : toolchain::reverse(SanitizerSections)) {
    if (S.Mask & Mask) {
      unsigned LineNum =
          SpecialCaseList::inSectionBlame(S.Entries, Prefix, Query, Category);
      if (LineNum > 0)
        return {S.FileIdx, LineNum};
    }
  }
  return NotFound;
}
