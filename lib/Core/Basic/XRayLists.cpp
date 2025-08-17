//===-- XRayLists.cpp - XRay automatic-attribution ------------------------===//
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

#include "language/Core/Basic/XRayLists.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/Basic/SourceManager.h"
#include "toolchain/Support/SpecialCaseList.h"

using namespace language::Core;

XRayFunctionFilter::XRayFunctionFilter(
    ArrayRef<std::string> AlwaysInstrumentPaths,
    ArrayRef<std::string> NeverInstrumentPaths,
    ArrayRef<std::string> AttrListPaths, SourceManager &SM)
    : AlwaysInstrument(toolchain::SpecialCaseList::createOrDie(
          AlwaysInstrumentPaths, SM.getFileManager().getVirtualFileSystem())),
      NeverInstrument(toolchain::SpecialCaseList::createOrDie(
          NeverInstrumentPaths, SM.getFileManager().getVirtualFileSystem())),
      AttrList(toolchain::SpecialCaseList::createOrDie(
          AttrListPaths, SM.getFileManager().getVirtualFileSystem())),
      SM(SM) {}

XRayFunctionFilter::~XRayFunctionFilter() = default;

XRayFunctionFilter::ImbueAttribute
XRayFunctionFilter::shouldImbueFunction(StringRef FunctionName) const {
  // First apply the always instrument list, than if it isn't an "always" see
  // whether it's treated as a "never" instrument function.
  // TODO: Remove these as they're deprecated; use the AttrList exclusively.
  if (AlwaysInstrument->inSection("xray_always_instrument", "fun", FunctionName,
                                  "arg1") ||
      AttrList->inSection("always", "fun", FunctionName, "arg1"))
    return ImbueAttribute::ALWAYS_ARG1;
  if (AlwaysInstrument->inSection("xray_always_instrument", "fun",
                                  FunctionName) ||
      AttrList->inSection("always", "fun", FunctionName))
    return ImbueAttribute::ALWAYS;

  if (NeverInstrument->inSection("xray_never_instrument", "fun",
                                 FunctionName) ||
      AttrList->inSection("never", "fun", FunctionName))
    return ImbueAttribute::NEVER;

  return ImbueAttribute::NONE;
}

XRayFunctionFilter::ImbueAttribute
XRayFunctionFilter::shouldImbueFunctionsInFile(StringRef Filename,
                                               StringRef Category) const {
  if (AlwaysInstrument->inSection("xray_always_instrument", "src", Filename,
                                  Category) ||
      AttrList->inSection("always", "src", Filename, Category))
    return ImbueAttribute::ALWAYS;
  if (NeverInstrument->inSection("xray_never_instrument", "src", Filename,
                                 Category) ||
      AttrList->inSection("never", "src", Filename, Category))
    return ImbueAttribute::NEVER;
  return ImbueAttribute::NONE;
}

XRayFunctionFilter::ImbueAttribute
XRayFunctionFilter::shouldImbueLocation(SourceLocation Loc,
                                        StringRef Category) const {
  if (!Loc.isValid())
    return ImbueAttribute::NONE;
  return this->shouldImbueFunctionsInFile(SM.getFilename(SM.getFileLoc(Loc)),
                                          Category);
}
