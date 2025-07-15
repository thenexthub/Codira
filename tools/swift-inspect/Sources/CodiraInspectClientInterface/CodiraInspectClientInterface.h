//===----------------------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#if defined(_WIN32)

#include <stdint.h>

#define BUF_SIZE 512
#define SHARED_MEM_NAME_PREFIX "Local\\CodiraInspectFileMapping"
#define READ_EVENT_NAME_PREFIX "Local\\CodiraInspectReadEvent"
#define WRITE_EVENT_NAME_PREFIX "Local\\CodiraInspectWriteEvent"
#define WAIT_TIMEOUT_MS 30000

struct HeapEntry {
  uintptr_t Address;
  uintptr_t Size;
};

#endif
