//===----------------------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
///
/// \file
/// This file implements \c vfs::OutputBackend class methods.
///
//===----------------------------------------------------------------------===//

#include "vm/core/Support/VirtualOutputBackend.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/Support/VirtualOutputError.h"

using namespace vm::core;
using namespace vm::core::vfs;

void OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(const Twine &Path,
                          std::optional<OutputConfig> Config) {
  SmallString<128> PathStorage;
  Path.toVector(PathStorage);

  if (Config) {
    // Check for invalid configs.
    if (!Config->getText() && Config->getCRLF())
      return make_error<OutputConfigError>(*Config, PathStorage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  if (Error E = createFileImpl(PathStorage, Config).moveInto(Impl))
    return std::move(E);
  assert(Impl && "Expected valid Impl or Error");
  return OutputFile(PathStorage, std::move(Impl));
}
