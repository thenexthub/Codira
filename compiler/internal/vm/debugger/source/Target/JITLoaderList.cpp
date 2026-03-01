//===-- JITLoaderList.cpp -------------------------------------------------===//
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

#include "lldb/Target/JITLoader.h"
#include "lldb/Target/JITLoaderList.h"
#include "lldb/lldb-private.h"

using namespace lldb;
using namespace lldb_private;

JITLoaderList::JITLoaderList() : m_jit_loaders_vec(), m_jit_loaders_mutex() {}

JITLoaderList::~JITLoaderList() = default;

void JITLoaderList::Append(const JITLoaderSP &jit_loader_sp) {
  std::lock_guard<std::recursive_mutex> guard(m_jit_loaders_mutex);
  m_jit_loaders_vec.push_back(jit_loader_sp);
}

void JITLoaderList::Remove(const JITLoaderSP &jit_loader_sp) {
  std::lock_guard<std::recursive_mutex> guard(m_jit_loaders_mutex);
  llvm::erase(m_jit_loaders_vec, jit_loader_sp);
}

size_t JITLoaderList::GetSize() const { return m_jit_loaders_vec.size(); }

JITLoaderSP JITLoaderList::GetLoaderAtIndex(size_t idx) {
  std::lock_guard<std::recursive_mutex> guard(m_jit_loaders_mutex);
  return m_jit_loaders_vec[idx];
}

void JITLoaderList::DidLaunch() {
  std::lock_guard<std::recursive_mutex> guard(m_jit_loaders_mutex);
  for (auto const &jit_loader : m_jit_loaders_vec)
    jit_loader->DidLaunch();
}

void JITLoaderList::DidAttach() {
  std::lock_guard<std::recursive_mutex> guard(m_jit_loaders_mutex);
  for (auto const &jit_loader : m_jit_loaders_vec)
    jit_loader->DidAttach();
}

void JITLoaderList::ModulesDidLoad(ModuleList &module_list) {
  std::lock_guard<std::recursive_mutex> guard(m_jit_loaders_mutex);
  for (auto const &jit_loader : m_jit_loaders_vec)
    jit_loader->ModulesDidLoad(module_list);
}
