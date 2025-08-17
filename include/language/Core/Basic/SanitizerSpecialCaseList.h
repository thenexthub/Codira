//===--- SanitizerSpecialCaseList.h - SCL for sanitizers --------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_BASIC_SANITIZERSPECIALCASELIST_H
#define LANGUAGE_CORE_BASIC_SANITIZERSPECIALCASELIST_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/Sanitizers.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/SpecialCaseList.h"
#include <memory>
#include <utility>
#include <vector>

namespace toolchain {
namespace vfs {
class FileSystem;
}
} // namespace toolchain

namespace language::Core {

class SanitizerSpecialCaseList : public toolchain::SpecialCaseList {
public:
  static std::unique_ptr<SanitizerSpecialCaseList>
  create(const std::vector<std::string> &Paths, toolchain::vfs::FileSystem &VFS,
         std::string &Error);

  static std::unique_ptr<SanitizerSpecialCaseList>
  createOrDie(const std::vector<std::string> &Paths,
              toolchain::vfs::FileSystem &VFS);

  // Query ignorelisted entries if any bit in Mask matches the entry's section.
  bool inSection(SanitizerMask Mask, StringRef Prefix, StringRef Query,
                 StringRef Category = StringRef()) const;

  // Query ignorelisted entries if any bit in Mask matches the entry's section.
  // Return NotFound (0,0) if not found. If found, return the file index number
  // and the line number (FileIdx, LineNo) (FileIdx starts with 1 and LineNo
  // starts with 0).
  std::pair<unsigned, unsigned>
  inSectionBlame(SanitizerMask Mask, StringRef Prefix, StringRef Query,
                 StringRef Category = StringRef()) const;

protected:
  // Initialize SanitizerSections.
  void createSanitizerSections();

  struct SanitizerSection {
    SanitizerSection(SanitizerMask SM, SectionEntries &E, unsigned idx)
        : Mask(SM), Entries(E), FileIdx(idx) {};

    SanitizerMask Mask;
    SectionEntries &Entries;
    unsigned FileIdx;
  };

  std::vector<SanitizerSection> SanitizerSections;
};

} // end namespace language::Core

#endif
