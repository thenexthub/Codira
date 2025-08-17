//===----------------------------------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef LANGUAGE_CORE_TOOLING_DEPENDENCYSCANNING_INPROCESSMODULECACHE_H
#define LANGUAGE_CORE_TOOLING_DEPENDENCYSCANNING_INPROCESSMODULECACHE_H

#include "language/Core/Serialization/ModuleCache.h"
#include "toolchain/ADT/StringMap.h"

#include <mutex>
#include <shared_mutex>

namespace language::Core {
namespace tooling {
namespace dependencies {
struct ModuleCacheEntry {
  std::shared_mutex CompilationMutex;
  std::atomic<std::time_t> Timestamp = 0;
};

struct ModuleCacheEntries {
  std::mutex Mutex;
  toolchain::StringMap<std::unique_ptr<ModuleCacheEntry>> Map;
};

IntrusiveRefCntPtr<ModuleCache>
makeInProcessModuleCache(ModuleCacheEntries &Entries);
} // namespace dependencies
} // namespace tooling
} // namespace language::Core

#endif
