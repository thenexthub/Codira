//===-- FileCache.cpp -----------------------------------------------------===//
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

#include "lldb/Host/FileCache.h"

#include "lldb/Host/File.h"
#include "lldb/Host/FileSystem.h"

using namespace lldb;
using namespace lldb_private;

FileCache *FileCache::m_instance = nullptr;

FileCache &FileCache::GetInstance() {
  if (m_instance == nullptr)
    m_instance = new FileCache();

  return *m_instance;
}

lldb::user_id_t FileCache::OpenFile(const FileSpec &file_spec,
                                    File::OpenOptions flags, uint32_t mode,
                                    Status &error) {
  if (!file_spec) {
    error = Status::FromErrorString("empty path");
    return UINT64_MAX;
  }
  auto file = FileSystem::Instance().Open(file_spec, flags, mode);
  if (!file) {
    error = Status::FromError(file.takeError());
    return UINT64_MAX;
  }
  lldb::user_id_t fd = file.get()->GetDescriptor();
  m_cache[fd] = std::move(file.get());
  return fd;
}

bool FileCache::CloseFile(lldb::user_id_t fd, Status &error) {
  if (fd == UINT64_MAX) {
    error = Status::FromErrorString("invalid file descriptor");
    return false;
  }
  FDToFileMap::iterator pos = m_cache.find(fd);
  if (pos == m_cache.end()) {
    error = Status::FromErrorStringWithFormat(
        "invalid host file descriptor %" PRIu64, fd);
    return false;
  }
  FileUP &file_up = pos->second;
  if (!file_up) {
    error = Status::FromErrorString("invalid host backing file");
    return false;
  }
  error = file_up->Close();
  m_cache.erase(pos);
  return error.Success();
}

uint64_t FileCache::WriteFile(lldb::user_id_t fd, uint64_t offset,
                              const void *src, uint64_t src_len,
                              Status &error) {
  if (fd == UINT64_MAX) {
    error = Status::FromErrorString("invalid file descriptor");
    return UINT64_MAX;
  }
  FDToFileMap::iterator pos = m_cache.find(fd);
  if (pos == m_cache.end()) {
    error = Status::FromErrorStringWithFormat(
        "invalid host file descriptor %" PRIu64, fd);
    return false;
  }
  FileUP &file_up = pos->second;
  if (!file_up) {
    error = Status::FromErrorString("invalid host backing file");
    return UINT64_MAX;
  }
  if (static_cast<uint64_t>(file_up->SeekFromStart(offset, &error)) != offset ||
      error.Fail())
    return UINT64_MAX;
  size_t bytes_written = src_len;
  error = file_up->Write(src, bytes_written);
  if (error.Fail())
    return UINT64_MAX;
  return bytes_written;
}

uint64_t FileCache::ReadFile(lldb::user_id_t fd, uint64_t offset, void *dst,
                             uint64_t dst_len, Status &error) {
  if (fd == UINT64_MAX) {
    error = Status::FromErrorString("invalid file descriptor");
    return UINT64_MAX;
  }
  FDToFileMap::iterator pos = m_cache.find(fd);
  if (pos == m_cache.end()) {
    error = Status::FromErrorStringWithFormat(
        "invalid host file descriptor %" PRIu64, fd);
    return false;
  }
  FileUP &file_up = pos->second;
  if (!file_up) {
    error = Status::FromErrorString("invalid host backing file");
    return UINT64_MAX;
  }
  if (static_cast<uint64_t>(file_up->SeekFromStart(offset, &error)) != offset ||
      error.Fail())
    return UINT64_MAX;
  size_t bytes_read = dst_len;
  error = file_up->Read(dst, bytes_read);
  if (error.Fail())
    return UINT64_MAX;
  return bytes_read;
}
