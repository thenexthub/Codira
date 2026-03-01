//===-- Stoppoint.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_BREAKPOINT_STOPPOINT_H
#define LLDB_BREAKPOINT_STOPPOINT_H

#include "lldb/Utility/UserID.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class Stoppoint {
public:
  // Constructors and Destructors
  Stoppoint();

  virtual ~Stoppoint();

  // Methods
  virtual void Dump(Stream *) = 0;

  virtual bool IsEnabled() = 0;

  virtual void SetEnabled(bool enable) = 0;

  lldb::break_id_t GetID() const;

  void SetID(lldb::break_id_t bid);

protected:
  lldb::break_id_t m_bid = LLDB_INVALID_BREAK_ID;

private:
  // For Stoppoint only
  Stoppoint(const Stoppoint &) = delete;
  const Stoppoint &operator=(const Stoppoint &) = delete;
};

} // namespace lldb_private

#endif // LLDB_BREAKPOINT_STOPPOINT_H
