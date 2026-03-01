//===-- SBQueueItem.h -------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBQUEUEITEM_H
#define LLDB_API_SBQUEUEITEM_H

#include "lldb/API/SBAddress.h"
#include "lldb/API/SBDefines.h"

namespace lldb_private {
class QueueImpl;
}

namespace lldb {

class LLDB_API SBQueueItem {
public:
  SBQueueItem();

  ~SBQueueItem();

  explicit operator bool() const;

  bool IsValid() const;

  void Clear();

  lldb::QueueItemKind GetKind() const;

  void SetKind(lldb::QueueItemKind kind);

  lldb::SBAddress GetAddress() const;

  void SetAddress(lldb::SBAddress addr);

  SBThread GetExtendedBacktraceThread(const char *type);

protected:
  friend class lldb_private::QueueImpl;

  SBQueueItem(const lldb::QueueItemSP &queue_item_sp);

  void SetQueueItem(const lldb::QueueItemSP &queue_item_sp);

private:
  lldb::QueueItemSP m_queue_item_sp;
};

} // namespace lldb

#endif // LLDB_API_SBQUEUEITEM_H
