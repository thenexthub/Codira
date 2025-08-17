//===--- XRayLists.h - XRay automatic attribution ---------------*- C++ -*-===//
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
// User-provided filters for always/never XRay instrumenting certain functions.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_BASIC_XRAYLISTS_H
#define LANGUAGE_CORE_BASIC_XRAYLISTS_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringRef.h"
#include <memory>

namespace toolchain {
class SpecialCaseList;
}

namespace language::Core {

class SourceManager;

class XRayFunctionFilter {
  std::unique_ptr<toolchain::SpecialCaseList> AlwaysInstrument;
  std::unique_ptr<toolchain::SpecialCaseList> NeverInstrument;
  std::unique_ptr<toolchain::SpecialCaseList> AttrList;
  SourceManager &SM;

public:
  XRayFunctionFilter(ArrayRef<std::string> AlwaysInstrumentPaths,
                     ArrayRef<std::string> NeverInstrumentPaths,
                     ArrayRef<std::string> AttrListPaths, SourceManager &SM);
  ~XRayFunctionFilter();

  enum class ImbueAttribute {
    NONE,
    ALWAYS,
    NEVER,
    ALWAYS_ARG1,
  };

  ImbueAttribute shouldImbueFunction(StringRef FunctionName) const;

  ImbueAttribute
  shouldImbueFunctionsInFile(StringRef Filename,
                             StringRef Category = StringRef()) const;

  ImbueAttribute shouldImbueLocation(SourceLocation Loc,
                                     StringRef Category = StringRef()) const;
};

} // namespace language::Core

#endif
