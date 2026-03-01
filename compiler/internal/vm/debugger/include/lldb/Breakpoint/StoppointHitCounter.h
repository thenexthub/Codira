//===-- StoppointHitCounter.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_BREAKPOINT_STOPPOINT_HIT_COUNTER_H
#define LLDB_BREAKPOINT_STOPPOINT_HIT_COUNTER_H

#include <cassert>
#include <cstdint>
#include <limits>

#include "lldb/Utility/LLDBAssert.h"

namespace lldb_private {

class StoppointHitCounter {
public:
  uint32_t GetValue() const { return m_hit_count; }

  void Increment(uint32_t difference = 1) {
    lldbassert(std::numeric_limits<uint32_t>::max() - m_hit_count >= difference);
    m_hit_count += difference;
  }

  void Decrement(uint32_t difference = 1) {
    lldbassert(m_hit_count >= difference);
    m_hit_count -= difference;
  }

  void Reset() { m_hit_count = 0; }

private:
  /// Number of times this breakpoint/watchpoint has been hit.
  uint32_t m_hit_count = 0;
};

} // namespace lldb_private

#endif // LLDB_BREAKPOINT_STOPPOINT_HIT_COUNTER_H
