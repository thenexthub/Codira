//===-- JITLoaderList.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_JITLOADERLIST_H
#define LLDB_TARGET_JITLOADERLIST_H

#include <mutex>
#include <vector>

#include "lldb/lldb-forward.h"

namespace lldb_private {

/// \class JITLoaderList JITLoaderList.h "lldb/Target/JITLoaderList.h"
///
/// Class used by the Process to hold a list of its JITLoaders.
class JITLoaderList {
public:
  JITLoaderList();
  ~JITLoaderList();

  void Append(const lldb::JITLoaderSP &jit_loader_sp);

  void Remove(const lldb::JITLoaderSP &jit_loader_sp);

  size_t GetSize() const;

  lldb::JITLoaderSP GetLoaderAtIndex(size_t idx);

  void DidLaunch();

  void DidAttach();

  void ModulesDidLoad(ModuleList &module_list);

private:
  std::vector<lldb::JITLoaderSP> m_jit_loaders_vec;
  std::recursive_mutex m_jit_loaders_mutex;
};

} // namespace lldb_private

#endif // LLDB_TARGET_JITLOADERLIST_H
