//===-- StreamFile.cpp ----------------------------------------------------===//
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

#include "lldb/Host/StreamFile.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"

#include <cstdio>

using namespace lldb;
using namespace lldb_private;

StreamFile::StreamFile(uint32_t flags, uint32_t addr_size, ByteOrder byte_order)
    : Stream(flags, addr_size, byte_order) {
  m_file_sp = std::make_shared<File>();
}

StreamFile::StreamFile(int fd, bool transfer_ownership) : Stream() {
  m_file_sp = std::make_shared<NativeFile>(fd, File::eOpenOptionWriteOnly,
                                           transfer_ownership);
}

StreamFile::StreamFile(FILE *fh, bool transfer_ownership) : Stream() {
  m_file_sp = std::make_shared<NativeFile>(fh, File::eOpenOptionWriteOnly,
                                           transfer_ownership);
}

StreamFile::StreamFile(const char *path, File::OpenOptions options,
                       uint32_t permissions)
    : Stream() {
  auto file = FileSystem::Instance().Open(FileSpec(path), options, permissions);
  if (file)
    m_file_sp = std::move(file.get());
  else {
    // TODO refactor this so the error gets popagated up instead of logged here.
    LLDB_LOG_ERROR(GetLog(LLDBLog::Host), file.takeError(),
                   "Cannot open {1}: {0}", path);
    m_file_sp = std::make_shared<File>();
  }
}

StreamFile::~StreamFile() = default;

void StreamFile::Flush() { m_file_sp->Flush(); }

size_t StreamFile::WriteImpl(const void *s, size_t length) {
  m_file_sp->Write(s, length);
  return length;
}
