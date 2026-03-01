//===-- FifoFilesTest.cpp -------------------------------------------------===//
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

#include "FifoFiles.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Testing/Support/Error.h"
#include "gtest/gtest.h"
#include <chrono>
#include <thread>

using namespace lldb_dap;
using namespace llvm;

namespace {

std::string MakeTempFifoPath() {
  llvm::SmallString<128> temp_path;
  llvm::sys::fs::createUniquePath("lldb-dap-fifo-%%%%%%", temp_path,
                                  /*MakeAbsolute=*/true);
  return temp_path.str().str();
}

} // namespace

TEST(FifoFilesTest, CreateAndDestroyFifoFile) {
  std::string fifo_path = MakeTempFifoPath();
  auto fifo = CreateFifoFile(fifo_path);
  EXPECT_THAT_EXPECTED(fifo, llvm::Succeeded());

  // File should exist.
  EXPECT_TRUE(llvm::sys::fs::exists(fifo_path));

  // Destructor should remove the file.
  fifo->reset();
  EXPECT_FALSE(llvm::sys::fs::exists(fifo_path));
}

TEST(FifoFilesTest, SendAndReceiveJSON) {
  std::string fifo_path = MakeTempFifoPath();
  auto fifo = CreateFifoFile(fifo_path);
  EXPECT_THAT_EXPECTED(fifo, llvm::Succeeded());

  FifoFileIO writer(fifo_path, "writer");
  FifoFileIO reader(fifo_path, "reader");

  llvm::json::Object obj;
  obj["foo"] = "bar";
  obj["num"] = 42;

  // Writer thread.
  std::thread writer_thread([&]() {
    EXPECT_THAT_ERROR(writer.SendJSON(llvm::json::Value(std::move(obj)),
                                      std::chrono::milliseconds(500)),
                      llvm::Succeeded());
  });

  // Reader thread.
  std::thread reader_thread([&]() {
    auto result = reader.ReadJSON(std::chrono::milliseconds(500));
    EXPECT_THAT_EXPECTED(result, llvm::Succeeded());
    auto *read_obj = result->getAsObject();

    ASSERT_NE(read_obj, nullptr);
    EXPECT_EQ((*read_obj)["foo"].getAsString(), "bar");
    EXPECT_EQ((*read_obj)["num"].getAsInteger(), 42);
  });

  writer_thread.join();
  reader_thread.join();
}

TEST(FifoFilesTest, ReadTimeout) {
  std::string fifo_path = MakeTempFifoPath();
  auto fifo = CreateFifoFile(fifo_path);
  EXPECT_THAT_EXPECTED(fifo, llvm::Succeeded());

  FifoFileIO reader(fifo_path, "reader");

  // No writer, should timeout.
  auto result = reader.ReadJSON(std::chrono::milliseconds(100));
  EXPECT_THAT_EXPECTED(result, llvm::Failed());
}

TEST(FifoFilesTest, WriteTimeout) {
  std::string fifo_path = MakeTempFifoPath();
  auto fifo = CreateFifoFile(fifo_path);
  EXPECT_THAT_EXPECTED(fifo, llvm::Succeeded());

  FifoFileIO writer(fifo_path, "writer");

  // No reader, should timeout.
  llvm::json::Object obj;
  obj["foo"] = "bar";
  EXPECT_THAT_ERROR(writer.SendJSON(llvm::json::Value(std::move(obj)),
                                    std::chrono::milliseconds(100)),
                    llvm::Failed());
}
