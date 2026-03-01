//===-- SBQueue.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBQUEUE_H
#define LLDB_API_SBQUEUE_H

#include <vector>

#include "lldb/API/SBDefines.h"
#include "lldb/lldb-forward.h"

namespace lldb {

class LLDB_API SBQueue {
public:
  SBQueue();

  SBQueue(const SBQueue &rhs);

  const SBQueue &operator=(const lldb::SBQueue &rhs);

  ~SBQueue();

  explicit operator bool() const;

  bool IsValid() const;

  void Clear();

  lldb::SBProcess GetProcess();

  lldb::queue_id_t GetQueueID() const;

  const char *GetName() const;

  uint32_t GetIndexID() const;

  uint32_t GetNumThreads();

  lldb::SBThread GetThreadAtIndex(uint32_t);

  uint32_t GetNumPendingItems();

  lldb::SBQueueItem GetPendingItemAtIndex(uint32_t);

  uint32_t GetNumRunningItems();

  lldb::QueueKind GetKind();

protected:
  friend class SBProcess;
  friend class SBThread;

  SBQueue(const QueueSP &queue_sp);

  void SetQueue(const lldb::QueueSP &queue_sp);

private:
  std::shared_ptr<lldb_private::QueueImpl> m_opaque_sp;
};

} // namespace lldb

#endif // LLDB_API_SBQUEUE_H
