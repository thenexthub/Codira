//===-- PThreadEvent.h ------------------------------------------*- C++ -*-===//
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
//
//  Created by Greg Clayton on 6/16/07.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_TOOLS_DEBUGSERVER_SOURCE_PTHREADEVENT_H
#define LLDB_TOOLS_DEBUGSERVER_SOURCE_PTHREADEVENT_H

#include <cstdint>
#include <ctime>
#include <functional>
#include <mutex>

class PThreadEvent {
public:
  PThreadEvent(uint32_t bits = 0, uint32_t validBits = 0);
  ~PThreadEvent();

  uint32_t NewEventBit();
  void FreeEventBits(const uint32_t mask);

  void ReplaceEventBits(const uint32_t bits);
  uint32_t GetEventBits() const;
  void SetEvents(const uint32_t mask);
  void ResetEvents(const uint32_t mask);
  // Wait for events to be set or reset. These functions take an optional
  // timeout value. If timeout is NULL an infinite timeout will be used.
  uint32_t
  WaitForSetEvents(const uint32_t mask,
                   const struct timespec *timeout_abstime = NULL) const;
  uint32_t
  WaitForEventsToReset(const uint32_t mask,
                       const struct timespec *timeout_abstime = NULL) const;

  uint32_t GetResetAckMask() const { return m_reset_ack_mask; }
  uint32_t SetResetAckMask(uint32_t mask) { return m_reset_ack_mask = mask; }
  uint32_t WaitForResetAck(const uint32_t mask,
                           const struct timespec *timeout_abstime = NULL) const;

protected:
  mutable std::mutex m_mutex;
  mutable std::condition_variable m_set_condition;
  uint32_t m_bits;
  uint32_t m_validBits;
  uint32_t m_reset_ack_mask;

  uint32_t GetBitsMasked(uint32_t mask) const { return mask & m_bits; }

  uint32_t WaitForEventsImpl(const uint32_t mask,
                             const struct timespec *timeout_abstime,
                             std::function<bool()> predicate) const;

private:
  PThreadEvent(const PThreadEvent &) = delete;
  PThreadEvent &operator=(const PThreadEvent &rhs) = delete;
};

#endif // LLDB_TOOLS_DEBUGSERVER_SOURCE_PTHREADEVENT_H
