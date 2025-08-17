//===- Library.cpp --------------------------------------------------------===//
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

#include "language/Core/InstallAPI/Library.h"

using namespace toolchain;
namespace language::Core::installapi {

const Regex Rule("(.+)/(.+)\\.framework/");
StringRef Library::getFrameworkNameFromInstallName(StringRef InstallName) {
  assert(InstallName.contains(".framework") && "expected a framework");
  SmallVector<StringRef, 3> Match;
  Rule.match(InstallName, &Match);
  if (Match.empty())
    return "";
  return Match.back();
}

StringRef Library::getName() const {
  assert(!IsUnwrappedDylib && "expected a framework");
  StringRef Path = BaseDirectory;

  // Return the framework name extracted from path.
  while (!Path.empty()) {
    if (Path.ends_with(".framework"))
      return sys::path::filename(Path);
    Path = sys::path::parent_path(Path);
  }

  // Otherwise, return the name of the BaseDirectory.
  Path = BaseDirectory;
  return sys::path::filename(Path.rtrim("/"));
}

} // namespace language::Core::installapi
