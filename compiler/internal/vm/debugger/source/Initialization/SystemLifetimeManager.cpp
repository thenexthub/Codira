//===-- SystemLifetimeManager.cpp -----------------------------------------===//
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

#include "lldb/Initialization/SystemLifetimeManager.h"

#include "lldb/Initialization/SystemInitializer.h"

#include <utility>

using namespace lldb_private;

SystemLifetimeManager::SystemLifetimeManager() : m_mutex() {}

SystemLifetimeManager::~SystemLifetimeManager() {
  assert(!m_initialized &&
         "SystemLifetimeManager destroyed without calling Terminate!");
}

llvm::Error SystemLifetimeManager::Initialize(
    std::unique_ptr<SystemInitializer> initializer) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  if (!m_initialized) {
    assert(!m_initializer && "Attempting to call "
                             "SystemLifetimeManager::Initialize() when it is "
                             "already initialized");
    m_initialized = true;
    m_initializer = std::move(initializer);

    if (auto e = m_initializer->Initialize())
      return e;
  }

  return llvm::Error::success();
}

void SystemLifetimeManager::Terminate() {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);

  if (m_initialized) {
    m_initializer->Terminate();

    m_initializer.reset();
    m_initialized = false;
  }
}
