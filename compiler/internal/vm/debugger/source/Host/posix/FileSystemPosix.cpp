//===-- FileSystemPosix.cpp -----------------------------------------------===//
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

#include "lldb/Host/FileSystem.h"

// C includes
#include <fcntl.h>
#include <unistd.h>

// lldb Includes
#include "lldb/Host/Host.h"
#include "lldb/Utility/Status.h"
#include "lldb/Utility/StreamString.h"

#include "llvm/Support/Errno.h"
#include "llvm/Support/FileSystem.h"

using namespace lldb;
using namespace lldb_private;

const char *FileSystem::DEV_NULL = "/dev/null";

Status FileSystem::Symlink(const FileSpec &src, const FileSpec &dst) {
  Status error;
  if (::symlink(dst.GetPath().c_str(), src.GetPath().c_str()) == -1)
    return Status::FromErrno();
  return error;
}

Status FileSystem::Readlink(const FileSpec &src, FileSpec &dst) {
  Status error;
  char buf[PATH_MAX];
  ssize_t count = ::readlink(src.GetPath().c_str(), buf, sizeof(buf) - 1);
  if (count < 0)
    return Status::FromErrno();

  buf[count] = '\0'; // Success
  dst.SetFile(buf, FileSpec::Style::native);

  return error;
}

Status FileSystem::ResolveSymbolicLink(const FileSpec &src, FileSpec &dst) {
  char resolved_path[PATH_MAX];
  if (!src.GetPath(resolved_path, sizeof(resolved_path))) {
    return Status::FromErrorStringWithFormat(
        "Couldn't get the canonical path for %s", src.GetPath().c_str());
  }

  char real_path[PATH_MAX + 1];
  if (realpath(resolved_path, real_path) == nullptr) {
    return Status::FromErrno();
  }

  dst = FileSpec(real_path);

  return Status();
}

FILE *FileSystem::Fopen(const char *path, const char *mode) {
  return llvm::sys::RetryAfterSignal(nullptr, ::fopen, path, mode);
}

int FileSystem::Open(const char *path, int flags, int mode) {
  // Call ::open in a lambda to avoid overload resolution in RetryAfterSignal
  // when open is overloaded, such as in Bionic.
  auto lambda = [&]() { return ::open(path, flags, mode); };
  return llvm::sys::RetryAfterSignal(-1, lambda);
}
