//===-- DataBufferTest.cpp ------------------------------------------------===//
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

#include "gtest/gtest.h"

#include "lldb/Utility/DataBuffer.h"
#include "lldb/Utility/DataBufferHeap.h"
#include "lldb/Utility/DataBufferLLVM.h"

#include "llvm/Support/Casting.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace lldb_private;
using namespace lldb;

TEST(DataBufferTest, RTTI) {
  {
    DataBufferSP data_buffer_sp = std::make_shared<DataBufferHeap>();
    DataBuffer *data_buffer = data_buffer_sp.get();

    EXPECT_TRUE(llvm::isa<DataBuffer>(data_buffer));
    EXPECT_TRUE(llvm::isa<WritableDataBuffer>(data_buffer));
    EXPECT_TRUE(llvm::isa<DataBufferHeap>(data_buffer));
    EXPECT_FALSE(llvm::isa<DataBufferLLVM>(data_buffer));
  }

  {
    llvm::StringRef data;
    DataBufferSP data_buffer_sp = std::make_shared<DataBufferLLVM>(
        llvm::MemoryBuffer::getMemBufferCopy(data));
    DataBuffer *data_buffer = data_buffer_sp.get();

    EXPECT_TRUE(llvm::isa<DataBuffer>(data_buffer));
    EXPECT_TRUE(llvm::isa<DataBufferLLVM>(data_buffer));
    EXPECT_FALSE(llvm::isa<WritableDataBuffer>(data_buffer));
    EXPECT_FALSE(llvm::isa<WritableDataBufferLLVM>(data_buffer));
    EXPECT_FALSE(llvm::isa<DataBufferHeap>(data_buffer));
  }

  {
    DataBufferSP data_buffer_sp = std::make_shared<WritableDataBufferLLVM>(
        llvm::WritableMemoryBuffer::getNewMemBuffer(1));
    DataBuffer *data_buffer = data_buffer_sp.get();

    EXPECT_TRUE(llvm::isa<DataBuffer>(data_buffer));
    EXPECT_TRUE(llvm::isa<WritableDataBuffer>(data_buffer));
    EXPECT_TRUE(llvm::isa<WritableDataBufferLLVM>(data_buffer));
    EXPECT_FALSE(llvm::isa<DataBufferLLVM>(data_buffer));
    EXPECT_FALSE(llvm::isa<DataBufferHeap>(data_buffer));
  }
}
