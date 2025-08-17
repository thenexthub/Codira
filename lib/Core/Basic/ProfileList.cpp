//===--- ProfileList.h - ProfileList filter ---------------------*- C++ -*-===//
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
// User-provided filters include/exclude profile instrumentation in certain
// functions or files.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/ProfileList.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/Basic/SourceManager.h"
#include "toolchain/Support/SpecialCaseList.h"

#include <optional>

using namespace language::Core;

namespace language::Core {

class ProfileSpecialCaseList : public toolchain::SpecialCaseList {
public:
  static std::unique_ptr<ProfileSpecialCaseList>
  create(const std::vector<std::string> &Paths, toolchain::vfs::FileSystem &VFS,
         std::string &Error);

  static std::unique_ptr<ProfileSpecialCaseList>
  createOrDie(const std::vector<std::string> &Paths,
              toolchain::vfs::FileSystem &VFS);

  bool isEmpty() const { return Sections.empty(); }

  bool hasPrefix(StringRef Prefix) const {
    for (const auto &It : Sections)
      if (It.Entries.count(Prefix) > 0)
        return true;
    return false;
  }
};

std::unique_ptr<ProfileSpecialCaseList>
ProfileSpecialCaseList::create(const std::vector<std::string> &Paths,
                               toolchain::vfs::FileSystem &VFS, std::string &Error) {
  auto PSCL = std::make_unique<ProfileSpecialCaseList>();
  if (PSCL->createInternal(Paths, VFS, Error))
    return PSCL;
  return nullptr;
}

std::unique_ptr<ProfileSpecialCaseList>
ProfileSpecialCaseList::createOrDie(const std::vector<std::string> &Paths,
                                    toolchain::vfs::FileSystem &VFS) {
  std::string Error;
  if (auto PSCL = create(Paths, VFS, Error))
    return PSCL;
  toolchain::report_fatal_error(toolchain::Twine(Error));
}

} // namespace language::Core

ProfileList::ProfileList(ArrayRef<std::string> Paths, SourceManager &SM)
    : SCL(ProfileSpecialCaseList::createOrDie(
          Paths, SM.getFileManager().getVirtualFileSystem())),
      Empty(SCL->isEmpty()), SM(SM) {}

ProfileList::~ProfileList() = default;

static StringRef getSectionName(toolchain::driver::ProfileInstrKind Kind) {
  switch (Kind) {
  case toolchain::driver::ProfileInstrKind::ProfileNone:
    return "";
  case toolchain::driver::ProfileInstrKind::ProfileClangInstr:
    return "clang";
  case toolchain::driver::ProfileInstrKind::ProfileIRInstr:
    return "toolchain";
  case toolchain::driver::ProfileInstrKind::ProfileCSIRInstr:
    return "cstoolchain";
  case toolchain::driver::ProfileInstrKind::ProfileIRSampleColdCov:
    return "sample-coldcov";
  }
  toolchain_unreachable("Unhandled toolchain::driver::ProfileInstrKind enum");
}

ProfileList::ExclusionType
ProfileList::getDefault(toolchain::driver::ProfileInstrKind Kind) const {
  StringRef Section = getSectionName(Kind);
  // Check for "default:<type>"
  if (SCL->inSection(Section, "default", "allow"))
    return Allow;
  if (SCL->inSection(Section, "default", "skip"))
    return Skip;
  if (SCL->inSection(Section, "default", "forbid"))
    return Forbid;
  // If any cases use "fun" or "src", set the default to FORBID.
  if (SCL->hasPrefix("fun") || SCL->hasPrefix("src"))
    return Forbid;
  return Allow;
}

std::optional<ProfileList::ExclusionType>
ProfileList::inSection(StringRef Section, StringRef Prefix,
                       StringRef Query) const {
  if (SCL->inSection(Section, Prefix, Query, "allow"))
    return Allow;
  if (SCL->inSection(Section, Prefix, Query, "skip"))
    return Skip;
  if (SCL->inSection(Section, Prefix, Query, "forbid"))
    return Forbid;
  if (SCL->inSection(Section, Prefix, Query))
    return Allow;
  return std::nullopt;
}

std::optional<ProfileList::ExclusionType>
ProfileList::isFunctionExcluded(StringRef FunctionName,
                                toolchain::driver::ProfileInstrKind Kind) const {
  StringRef Section = getSectionName(Kind);
  // Check for "function:<regex>=<case>"
  if (auto V = inSection(Section, "function", FunctionName))
    return V;
  if (SCL->inSection(Section, "!fun", FunctionName))
    return Forbid;
  if (SCL->inSection(Section, "fun", FunctionName))
    return Allow;
  return std::nullopt;
}

std::optional<ProfileList::ExclusionType>
ProfileList::isLocationExcluded(SourceLocation Loc,
                                toolchain::driver::ProfileInstrKind Kind) const {
  return isFileExcluded(SM.getFilename(SM.getFileLoc(Loc)), Kind);
}

std::optional<ProfileList::ExclusionType>
ProfileList::isFileExcluded(StringRef FileName,
                            toolchain::driver::ProfileInstrKind Kind) const {
  StringRef Section = getSectionName(Kind);
  // Check for "source:<regex>=<case>"
  if (auto V = inSection(Section, "source", FileName))
    return V;
  if (SCL->inSection(Section, "!src", FileName))
    return Forbid;
  if (SCL->inSection(Section, "src", FileName))
    return Allow;
  return std::nullopt;
}
