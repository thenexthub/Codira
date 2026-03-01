//===-- FileAction.cpp ----------------------------------------------------===//
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

#include <fcntl.h>

#include "lldb/Host/FileAction.h"
#include "lldb/Host/PosixApi.h"
#include "lldb/Utility/Stream.h"

using namespace lldb_private;

// FileAction member functions

FileAction::FileAction() : m_file_spec() {}

void FileAction::Clear() {
  m_action = eFileActionNone;
  m_fd = -1;
  m_arg = -1;
  m_file_spec.Clear();
}

const FileSpec &FileAction::GetFileSpec() const { return m_file_spec; }

bool FileAction::Open(int fd, const FileSpec &file_spec, bool read,
                      bool write) {
  if ((read || write) && fd >= 0 && file_spec) {
    m_action = eFileActionOpen;
    m_fd = fd;
    if (read && write)
      m_arg = O_NOCTTY | O_CREAT | O_RDWR;
    else if (read)
      m_arg = O_NOCTTY | O_RDONLY;
    else
      m_arg = O_NOCTTY | O_CREAT | O_WRONLY | O_TRUNC;
    m_file_spec = file_spec;
    return true;
  } else {
    Clear();
  }
  return false;
}

bool FileAction::Close(int fd) {
  Clear();
  if (fd >= 0) {
    m_action = eFileActionClose;
    m_fd = fd;
  }
  return m_fd >= 0;
}

bool FileAction::Duplicate(int fd, int dup_fd) {
  Clear();
  if (fd >= 0 && dup_fd >= 0) {
    m_action = eFileActionDuplicate;
    m_fd = fd;
    m_arg = dup_fd;
  }
  return m_fd >= 0;
}

void FileAction::Dump(Stream &stream) const {
  stream.PutCString("file action: ");
  switch (m_action) {
  case eFileActionClose:
    stream.Printf("close fd %d", m_fd);
    break;
  case eFileActionDuplicate:
    stream.Printf("duplicate fd %d to %d", m_fd, m_arg);
    break;
  case eFileActionNone:
    stream.PutCString("no action");
    break;
  case eFileActionOpen:
    stream.Printf("open fd %d with '%s', OFLAGS = 0x%x", m_fd,
                  m_file_spec.GetPath().c_str(), m_arg);
    break;
  }
}
