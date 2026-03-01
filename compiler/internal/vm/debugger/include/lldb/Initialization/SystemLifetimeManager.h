//===-- SystemLifetimeManager.h -------------------------------*- C++ -*-===//
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

#ifndef LLDB_INITIALIZATION_SYSTEMLIFETIMEMANAGER_H
#define LLDB_INITIALIZATION_SYSTEMLIFETIMEMANAGER_H

#include "lldb/Initialization/SystemInitializer.h"
#include "lldb/lldb-private-types.h"
#include "llvm/Support/Error.h"

#include <memory>
#include <mutex>

namespace lldb_private {

class SystemLifetimeManager {
public:
  SystemLifetimeManager();
  ~SystemLifetimeManager();

  llvm::Error Initialize(std::unique_ptr<SystemInitializer> initializer);
  void Terminate();

private:
  std::recursive_mutex m_mutex;
  std::unique_ptr<SystemInitializer> m_initializer;
  bool m_initialized = false;

  // Noncopyable.
  SystemLifetimeManager(const SystemLifetimeManager &other) = delete;
  SystemLifetimeManager &operator=(const SystemLifetimeManager &other) = delete;
};
}

#endif
