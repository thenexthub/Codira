//===-- StopPointSiteList.cpp ---------------------------------------------===//
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

#include "lldb/Breakpoint/StopPointSiteList.h"
#include "lldb/Breakpoint/BreakpointSite.h"
#include "lldb/Breakpoint/WatchpointResource.h"

#include "lldb/Utility/Stream.h"
#include <algorithm>

using namespace lldb;
using namespace lldb_private;

// This method is only defined when we're specializing for
// BreakpointSite / BreakpointLocation / Breakpoint.
// Watchpoints don't have a similar structure, they are
// WatchpointResource / Watchpoint

template <>
bool StopPointSiteList<BreakpointSite>::StopPointSiteContainsBreakpoint(
    typename BreakpointSite::SiteID site_id, lldb::break_id_t bp_id) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  typename collection::const_iterator pos = GetIDConstIterator(site_id);
  if (pos != m_site_list.end())
    return pos->second->IsBreakpointAtThisSite(bp_id);

  return false;
}

namespace lldb_private {
template class StopPointSiteList<BreakpointSite>;
} // namespace lldb_private
