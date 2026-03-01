//===-- BitWriter.cpp -----------------------------------------------------===//
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

#include "vm/core-c/BitWriter.h"
#include "vm/core/Bitcode/BitcodeWriter.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;


/*===-- Operations on modules ---------------------------------------------===*/

int LLVMWriteBitcodeToFile(LLVMModuleRef M, const char *Path) {
  std::error_code EC;
  raw_fd_ostream OS(Path, EC, sys::fs::OF_None);

  if (EC)
    return -1;

  WriteBitcodeToFile(*unwrap(M), OS);
  return 0;
}

int LLVMWriteBitcodeToFD(LLVMModuleRef M, int FD, int ShouldClose,
                         int Unbuffered) {
  raw_fd_ostream OS(FD, ShouldClose, Unbuffered);

  WriteBitcodeToFile(*unwrap(M), OS);
  return 0;
}

int LLVMWriteBitcodeToFileHandle(LLVMModuleRef M, int FileHandle) {
  return LLVMWriteBitcodeToFD(M, FileHandle, true, false);
}

LLVMMemoryBufferRef LLVMWriteBitcodeToMemoryBuffer(LLVMModuleRef M) {
  std::string Data;
  raw_string_ostream OS(Data);

  WriteBitcodeToFile(*unwrap(M), OS);
  return wrap(MemoryBuffer::getMemBufferCopy(OS.str()).release());
}
