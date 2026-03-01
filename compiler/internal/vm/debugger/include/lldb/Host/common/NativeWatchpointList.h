//===-- NativeWatchpointList.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_COMMON_NATIVEWATCHPOINTLIST_H
#define LLDB_HOST_COMMON_NATIVEWATCHPOINTLIST_H

#include "lldb/Utility/Status.h"
#include "lldb/lldb-private-forward.h"

#include <map>

namespace lldb_private {
struct NativeWatchpoint {
  lldb::addr_t m_addr;
  size_t m_size;
  uint32_t m_watch_flags;
  bool m_hardware;
};

class NativeWatchpointList {
public:
  Status Add(lldb::addr_t addr, size_t size, uint32_t watch_flags,
             bool hardware);

  Status Remove(lldb::addr_t addr);

  using WatchpointMap = std::map<lldb::addr_t, NativeWatchpoint>;

  const WatchpointMap &GetWatchpointMap() const;

private:
  WatchpointMap m_watchpoints;
};
}

#endif // LLDB_HOST_COMMON_NATIVEWATCHPOINTLIST_H
