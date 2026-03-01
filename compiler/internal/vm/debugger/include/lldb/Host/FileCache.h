//===-- FileCache.h ---------------------------------------------*- C++ -*-===//
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
#ifndef LLDB_HOST_FILECACHE_H
#define LLDB_HOST_FILECACHE_H

#include <cstdint>
#include <map>

#include "lldb/lldb-forward.h"
#include "lldb/lldb-types.h"

#include "lldb/Host/File.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/Status.h"

namespace lldb_private {
class FileCache {
private:
  FileCache() = default;

  typedef std::map<lldb::user_id_t, lldb::FileUP> FDToFileMap;

public:
  static FileCache &GetInstance();

  lldb::user_id_t OpenFile(const FileSpec &file_spec, File::OpenOptions flags,
                           uint32_t mode, Status &error);
  bool CloseFile(lldb::user_id_t fd, Status &error);

  uint64_t WriteFile(lldb::user_id_t fd, uint64_t offset, const void *src,
                     uint64_t src_len, Status &error);
  uint64_t ReadFile(lldb::user_id_t fd, uint64_t offset, void *dst,
                    uint64_t dst_len, Status &error);

private:
  static FileCache *m_instance;

  FDToFileMap m_cache;
};
}

#endif
