//===-- DataBufferLLVM.cpp ------------------------------------------------===//
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

#include "lldb/Utility/DataBufferLLVM.h"

#include "llvm/Support/MemoryBuffer.h"

#include <cassert>

using namespace lldb_private;

DataBufferLLVM::DataBufferLLVM(std::unique_ptr<llvm::MemoryBuffer> MemBuffer)
    : Buffer(std::move(MemBuffer)) {
  assert(Buffer != nullptr &&
         "Cannot construct a DataBufferLLVM with a null buffer");
}

DataBufferLLVM::~DataBufferLLVM() = default;

const uint8_t *DataBufferLLVM::GetBytesImpl() const {
  return reinterpret_cast<const uint8_t *>(Buffer->getBufferStart());
}

lldb::offset_t DataBufferLLVM::GetByteSize() const {
  return Buffer->getBufferSize();
}

WritableDataBufferLLVM::WritableDataBufferLLVM(
    std::unique_ptr<llvm::WritableMemoryBuffer> MemBuffer)
    : Buffer(std::move(MemBuffer)) {
  assert(Buffer != nullptr &&
         "Cannot construct a WritableDataBufferLLVM with a null buffer");
}

WritableDataBufferLLVM::~WritableDataBufferLLVM() = default;

const uint8_t *WritableDataBufferLLVM::GetBytesImpl() const {
  return reinterpret_cast<const uint8_t *>(Buffer->getBufferStart());
}

lldb::offset_t WritableDataBufferLLVM::GetByteSize() const {
  return Buffer->getBufferSize();
}

char DataBufferLLVM::ID;
char WritableDataBufferLLVM::ID;
