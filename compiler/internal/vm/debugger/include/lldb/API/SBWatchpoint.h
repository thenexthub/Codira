//===-- SBWatchpoint.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBWATCHPOINT_H
#define LLDB_API_SBWATCHPOINT_H

#include "lldb/API/SBDefines.h"
#include "lldb/API/SBType.h"

namespace lldb_private {
namespace python {
class SWIGBridge;
}
namespace lua {
class SWIGBridge;
}
} // namespace lldb_private

namespace lldb {

class LLDB_API SBWatchpoint {
public:
  SBWatchpoint();

  SBWatchpoint(const lldb::SBWatchpoint &rhs);

  ~SBWatchpoint();

  const lldb::SBWatchpoint &operator=(const lldb::SBWatchpoint &rhs);

  explicit operator bool() const;

  bool operator==(const SBWatchpoint &rhs) const;

  bool operator!=(const SBWatchpoint &rhs) const;

  bool IsValid() const;

  SBError GetError();

  watch_id_t GetID();

  LLDB_DEPRECATED("Hardware index is not available, always returns -1")
  int32_t GetHardwareIndex();

  lldb::addr_t GetWatchAddress();

  size_t GetWatchSize();

  void SetEnabled(bool enabled);

  bool IsEnabled();

  uint32_t GetHitCount();

  uint32_t GetIgnoreCount();

  void SetIgnoreCount(uint32_t n);

  const char *GetCondition();

  void SetCondition(const char *condition);

  bool GetDescription(lldb::SBStream &description, DescriptionLevel level);

  void Clear();

  static bool EventIsWatchpointEvent(const lldb::SBEvent &event);

  static lldb::WatchpointEventType
  GetWatchpointEventTypeFromEvent(const lldb::SBEvent &event);

  static lldb::SBWatchpoint GetWatchpointFromEvent(const lldb::SBEvent &event);

  lldb::SBType GetType();

  WatchpointValueKind GetWatchValueKind();

  const char *GetWatchSpec();

  bool IsWatchingReads();

  bool IsWatchingWrites();

protected:
  friend class lldb_private::python::SWIGBridge;
  friend class lldb_private::lua::SWIGBridge;

  SBWatchpoint(const lldb::WatchpointSP &wp_sp);

  lldb::WatchpointSP GetSP() const;

  void SetSP(const lldb::WatchpointSP &sp);

private:
  friend class SBTarget;
  friend class SBValue;

  std::weak_ptr<lldb_private::Watchpoint> m_opaque_wp;
};

} // namespace lldb

#endif // LLDB_API_SBWATCHPOINT_H
