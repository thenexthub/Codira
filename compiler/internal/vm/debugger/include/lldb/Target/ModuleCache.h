//===-- ModuleCache.h -------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_MODULECACHE_H
#define LLDB_TARGET_MODULECACHE_H

#include "lldb/lldb-forward.h"
#include "lldb/lldb-types.h"

#include "lldb/Host/File.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/Status.h"

#include <functional>
#include <string>
#include <unordered_map>

namespace lldb_private {

class Module;
class UUID;

/// \class ModuleCache ModuleCache.h "lldb/Target/ModuleCache.h"
/// A module cache class.
///
/// Caches locally modules that are downloaded from remote targets. Each
/// cached module maintains 2 views:
///  - UUID view:
///  /${CACHE_ROOT}/${PLATFORM_NAME}/.cache/${UUID}/${MODULE_FILENAME}
///  - Sysroot view:
///  /${CACHE_ROOT}/${PLATFORM_NAME}/${HOSTNAME}/${MODULE_FULL_FILEPATH}
///
/// UUID views stores a real module file, whereas Sysroot view holds a symbolic
/// link to UUID-view file.
///
/// Example:
/// UUID view   :
/// /tmp/lldb/remote-
/// linux/.cache/30C94DC6-6A1F-E951-80C3-D68D2B89E576-D5AE213C/libc.so.6
/// Sysroot view: /tmp/lldb/remote-linux/ubuntu/lib/x86_64-linux-gnu/libc.so.6

class ModuleCache {
public:
  using ModuleDownloader =
      std::function<Status(const ModuleSpec &, const FileSpec &)>;
  using SymfileDownloader =
      std::function<Status(const lldb::ModuleSP &, const FileSpec &)>;

  Status GetAndPut(const FileSpec &root_dir_spec, const char *hostname,
                   const ModuleSpec &module_spec,
                   const ModuleDownloader &module_downloader,
                   const SymfileDownloader &symfile_downloader,
                   lldb::ModuleSP &cached_module_sp, bool *did_create_ptr);

private:
  Status Put(const FileSpec &root_dir_spec, const char *hostname,
             const ModuleSpec &module_spec, const FileSpec &tmp_file,
             const FileSpec &target_file);

  Status Get(const FileSpec &root_dir_spec, const char *hostname,
             const ModuleSpec &module_spec, lldb::ModuleSP &cached_module_sp,
             bool *did_create_ptr);

  std::unordered_map<std::string, lldb::ModuleWP> m_loaded_modules;
};

} // namespace lldb_private

#endif // LLDB_TARGET_MODULECACHE_H
