//===-- FileAction.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_FILEACTION_H
#define LLDB_HOST_FILEACTION_H

#include "lldb/Utility/FileSpec.h"
#include <string>

namespace lldb_private {

class FileAction {
public:
  enum Action {
    eFileActionNone,
    eFileActionClose,
    eFileActionDuplicate,
    eFileActionOpen
  };

  FileAction();

  void Clear();

  bool Close(int fd);

  bool Duplicate(int fd, int dup_fd);

  bool Open(int fd, const FileSpec &file_spec, bool read, bool write);

  int GetFD() const { return m_fd; }

  Action GetAction() const { return m_action; }

  int GetActionArgument() const { return m_arg; }

  const FileSpec &GetFileSpec() const;

  void Dump(Stream &stream) const;

protected:
  Action m_action = eFileActionNone; // The action for this file
  int m_fd = -1;                     // An existing file descriptor
  int m_arg = -1; // oflag for eFileActionOpen*, dup_fd for eFileActionDuplicate
  FileSpec
      m_file_spec; // A file spec to use for opening after fork or posix_spawn
};

} // namespace lldb_private

#endif
