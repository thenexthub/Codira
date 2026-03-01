//===-- PipeBase.h -----------------------------------------------*- C++
//-*-===//
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

#ifndef LLDB_HOST_PIPEBASE_H
#define LLDB_HOST_PIPEBASE_H

#include "lldb/Utility/Status.h"
#include "lldb/Utility/Timeout.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"

namespace lldb_private {
class PipeBase {
public:
  virtual ~PipeBase();

  virtual Status CreateNew() = 0;
  virtual Status CreateNew(llvm::StringRef name) = 0;
  virtual Status CreateWithUniqueName(llvm::StringRef prefix,
                                      llvm::SmallVectorImpl<char> &name) = 0;

  virtual Status OpenAsReader(llvm::StringRef name) = 0;

  virtual llvm::Error OpenAsWriter(llvm::StringRef name,
                                   const Timeout<std::micro> &timeout) = 0;

  virtual bool CanRead() const = 0;
  virtual bool CanWrite() const = 0;

  virtual lldb::pipe_t GetReadPipe() const = 0;
  virtual lldb::pipe_t GetWritePipe() const = 0;

  virtual int GetReadFileDescriptor() const = 0;
  virtual int GetWriteFileDescriptor() const = 0;
  virtual int ReleaseReadFileDescriptor() = 0;
  virtual int ReleaseWriteFileDescriptor() = 0;
  virtual void CloseReadFileDescriptor() = 0;
  virtual void CloseWriteFileDescriptor() = 0;

  // Close both descriptors
  virtual void Close() = 0;

  // Delete named pipe.
  virtual Status Delete(llvm::StringRef name) = 0;

  virtual llvm::Expected<size_t>
  Write(const void *buf, size_t size,
        const Timeout<std::micro> &timeout = std::nullopt) = 0;

  virtual llvm::Expected<size_t>
  Read(void *buf, size_t size,
       const Timeout<std::micro> &timeout = std::nullopt) = 0;
};
}

#endif
