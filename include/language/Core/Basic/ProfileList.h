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
// functions.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_BASIC_PROFILELIST_H
#define LANGUAGE_CORE_BASIC_PROFILELIST_H

#include "language/Core/Basic/CodeGenOptions.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringRef.h"
#include <memory>
#include <optional>

namespace language::Core {

class ProfileSpecialCaseList;

class ProfileList {
public:
  /// Represents if an how something should be excluded from profiling.
  enum ExclusionType {
    /// Profiling is allowed.
    Allow,
    /// Profiling is skipped using the \p skipprofile attribute.
    Skip,
    /// Profiling is forbidden using the \p noprofile attribute.
    Forbid,
  };

private:
  std::unique_ptr<ProfileSpecialCaseList> SCL;
  const bool Empty;
  SourceManager &SM;
  std::optional<ExclusionType> inSection(StringRef Section, StringRef Prefix,
                                         StringRef Query) const;

public:
  ProfileList(ArrayRef<std::string> Paths, SourceManager &SM);
  ~ProfileList();

  bool isEmpty() const { return Empty; }
  ExclusionType getDefault(toolchain::driver::ProfileInstrKind Kind) const;

  std::optional<ExclusionType>
  isFunctionExcluded(StringRef FunctionName,
                     toolchain::driver::ProfileInstrKind Kind) const;
  std::optional<ExclusionType>
  isLocationExcluded(SourceLocation Loc,
                     toolchain::driver::ProfileInstrKind Kind) const;
  std::optional<ExclusionType>
  isFileExcluded(StringRef FileName, toolchain::driver::ProfileInstrKind Kind) const;
};

} // namespace language::Core

#endif
