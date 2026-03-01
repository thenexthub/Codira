//===-- SBBroadcaster.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBBROADCASTER_H
#define LLDB_API_SBBROADCASTER_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBBroadcaster {
public:
  SBBroadcaster();

  SBBroadcaster(const char *name);

  SBBroadcaster(const SBBroadcaster &rhs);

  const SBBroadcaster &operator=(const SBBroadcaster &rhs);

  ~SBBroadcaster();

  explicit operator bool() const;

  bool IsValid() const;

  void Clear();

  void BroadcastEventByType(uint32_t event_type, bool unique = false);

  void BroadcastEvent(const lldb::SBEvent &event, bool unique = false);

  void AddInitialEventsToListener(const lldb::SBListener &listener,
                                  uint32_t requested_events);

  uint32_t AddListener(const lldb::SBListener &listener, uint32_t event_mask);

  const char *GetName() const;

  bool EventTypeHasListeners(uint32_t event_type);

  bool RemoveListener(const lldb::SBListener &listener,
                      uint32_t event_mask = UINT32_MAX);

  // This comparison is checking if the internal opaque pointer value is equal
  // to that in "rhs".
  bool operator==(const lldb::SBBroadcaster &rhs) const;

  // This comparison is checking if the internal opaque pointer value is not
  // equal to that in "rhs".
  bool operator!=(const lldb::SBBroadcaster &rhs) const;

  // This comparison is checking if the internal opaque pointer value is less
  // than that in "rhs" so SBBroadcaster objects can be contained in ordered
  // containers.
  bool operator<(const lldb::SBBroadcaster &rhs) const;

protected:
  friend class SBCommandInterpreter;
  friend class SBCommunication;
  friend class SBDebugger;
  friend class SBEvent;
  friend class SBListener;
  friend class SBProcess;
  friend class SBTarget;

  SBBroadcaster(lldb_private::Broadcaster *broadcaster, bool owns);

  lldb_private::Broadcaster *get() const;

  void reset(lldb_private::Broadcaster *broadcaster, bool owns);

private:
  lldb::BroadcasterSP m_opaque_sp;
  lldb_private::Broadcaster *m_opaque_ptr = nullptr;
};

} // namespace lldb

#endif // LLDB_API_SBBROADCASTER_H
