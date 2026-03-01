//===-- IOObject.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_IOOBJECT_H
#define LLDB_UTILITY_IOOBJECT_H

#include <cstdarg>
#include <cstdio>
#include <sys/types.h>

#include "lldb/lldb-private.h"
#include "lldb/lldb-types.h"

namespace lldb_private {

class IOObject {
public:
  enum FDType {
    eFDTypeFile,   // Other FD requiring read/write
    eFDTypeSocket, // Socket requiring send/recv
  };

  // A handle for integrating with the host event loop model.
  using WaitableHandle = lldb::file_t;

  static const WaitableHandle kInvalidHandleValue;

  IOObject(FDType type) : m_fd_type(type) {}
  virtual ~IOObject();

  virtual Status Read(void *buf, size_t &num_bytes) = 0;
  virtual Status Write(const void *buf, size_t &num_bytes) = 0;
  virtual bool IsValid() const = 0;
  virtual Status Close() = 0;

  FDType GetFdType() const { return m_fd_type; }

  virtual WaitableHandle GetWaitableHandle() = 0;

protected:
  FDType m_fd_type;

private:
  IOObject(const IOObject &) = delete;
  const IOObject &operator=(const IOObject &) = delete;
};
} // namespace lldb_private

#endif
