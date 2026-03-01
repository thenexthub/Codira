//===- Memory.cpp - Memory Handling Support ---------------------*- C++ -*-===//
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
//
// This file defines some helpful functions for allocating memory and dealing
// with memory mapped files
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/Memory.h"
#include "vm/core/Config/toolchain-config.h"

#ifndef NDEBUG
#include "vm/core/Support/raw_ostream.h"
#endif // ifndef NDEBUG

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Memory.inc"
#endif
#ifdef _WIN32
#include "Windows/Memory.inc"
#endif

#ifndef NDEBUG

namespace vm::core {
namespace sys {

raw_ostream &operator<<(raw_ostream &OS, const Memory::ProtectionFlags &PF) {
  assert((PF & ~(Memory::MF_READ | Memory::MF_WRITE | Memory::MF_EXEC)) == 0 &&
         "Unrecognized flags");

  return OS << (PF & Memory::MF_READ ? 'R' : '-')
            << (PF & Memory::MF_WRITE ? 'W' : '-')
            << (PF & Memory::MF_EXEC ? 'X' : '-');
}

raw_ostream &operator<<(raw_ostream &OS, const MemoryBlock &MB) {
  return OS << "[ " << MB.base() << " .. "
            << (void *)((char *)MB.base() + MB.allocatedSize()) << " ] ("
            << MB.allocatedSize() << " bytes)";
}

} // end namespace sys
} // end namespace vm::core

#endif // ifndef NDEBUG
