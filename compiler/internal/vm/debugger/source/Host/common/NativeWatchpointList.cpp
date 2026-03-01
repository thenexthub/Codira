//===-- NativeWatchpointList.cpp ------------------------------------------===//
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

#include "lldb/Host/common/NativeWatchpointList.h"

#include "lldb/Utility/Log.h"

using namespace lldb;
using namespace lldb_private;

Status NativeWatchpointList::Add(addr_t addr, size_t size, uint32_t watch_flags,
                                 bool hardware) {
  m_watchpoints[addr] = {addr, size, watch_flags, hardware};
  return Status();
}

Status NativeWatchpointList::Remove(addr_t addr) {
  m_watchpoints.erase(addr);
  return Status();
}

const NativeWatchpointList::WatchpointMap &
NativeWatchpointList::GetWatchpointMap() const {
  return m_watchpoints;
}
