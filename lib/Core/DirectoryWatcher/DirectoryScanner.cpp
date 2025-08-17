//===- DirectoryScanner.cpp - Utility functions for DirectoryWatcher ------===//
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

#include "DirectoryScanner.h"

#include "toolchain/Support/Path.h"
#include <optional>

namespace language::Core {

using namespace toolchain;

std::optional<sys::fs::file_status> getFileStatus(StringRef Path) {
  sys::fs::file_status Status;
  std::error_code EC = status(Path, Status);
  if (EC)
    return std::nullopt;
  return Status;
}

std::vector<std::string> scanDirectory(StringRef Path) {
  using namespace toolchain::sys;
  std::vector<std::string> Result;

  std::error_code EC;
  for (auto It = fs::directory_iterator(Path, EC),
            End = fs::directory_iterator();
       !EC && It != End; It.increment(EC)) {
    auto status = getFileStatus(It->path());
    if (!status)
      continue;
    Result.emplace_back(sys::path::filename(It->path()));
  }

  return Result;
}

std::vector<DirectoryWatcher::Event>
getAsFileEvents(const std::vector<std::string> &Scan) {
  std::vector<DirectoryWatcher::Event> Events;
  Events.reserve(Scan.size());

  for (const auto &File : Scan) {
    Events.emplace_back(DirectoryWatcher::Event{
        DirectoryWatcher::Event::EventKind::Modified, File});
  }
  return Events;
}

} // namespace language::Core
