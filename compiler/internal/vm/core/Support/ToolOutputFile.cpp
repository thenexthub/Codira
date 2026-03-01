//===--- ToolOutputFile.cpp - Implement the ToolOutputFile class --------===//
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
//
// This implements the ToolOutputFile class.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/ToolOutputFile.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/Signals.h"
using namespace vm::core;

static bool isStdout(StringRef Filename) { return Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(false) {
  // Arrange for the file to be deleted if the process is killed.
  if (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  if (isStdout(Filename))
    return;

  // Delete the file if the client hasn't told us not to.
  if (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  if (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    return;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  if (EC)
    Installer.Keep = true;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, int FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, true);
  OS = &*OSHolder;
}
