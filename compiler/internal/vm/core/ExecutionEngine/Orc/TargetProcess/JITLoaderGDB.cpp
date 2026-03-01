//===- JITLoaderGDB.h - Register objects via GDB JIT interface -*- C++ -*-===//
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

#include "vm/core/ExecutionEngine/Orc/TargetProcess/JITLoaderGDB.h"

#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/FormatVariadic.h"

#include <cstdint>
#include <mutex>

#define DEBUG_TYPE "orc"

// First version as landed in August 2009
static constexpr uint32_t JitDescriptorVersion = 1;

extern "C" {

// We put information about the JITed function in this global, which the
// debugger reads.  Make sure to specify the version statically, because the
// debugger checks the version before we can set it during runtime.
LLVM_ABI LLVM_ALWAYS_EXPORT struct jit_descriptor __jit_debug_descriptor = {
    JitDescriptorVersion, 0, nullptr, nullptr};

// Debuggers that implement the GDB JIT interface put a special breakpoint in
// this function.
LLVM_ABI LLVM_ALWAYS_EXPORT LLVM_ATTRIBUTE_NOINLINE void
__jit_debug_register_code() {
  // The noinline and the asm prevent calls to this function from being
  // optimized out.
#if !defined(_MSC_VER)
  asm volatile("" ::: "memory");
#endif
}
}

using namespace vm::core;
using namespace vm::core::orc;

// Register debug object, return error message or null for success.
static void appendJITDebugDescriptor(const char *ObjAddr, size_t Size) {
  LLVM_DEBUG({
    dbgs() << "Adding debug object to GDB JIT interface "
           << formatv("([{0:x16} -- {1:x16}])",
                      reinterpret_cast<uintptr_t>(ObjAddr),
                      reinterpret_cast<uintptr_t>(ObjAddr + Size))
           << "\n";
  });

  jit_code_entry *E = new jit_code_entry;
  E->symfile_addr = ObjAddr;
  E->symfile_size = Size;
  E->prev_entry = nullptr;

  // Serialize rendezvous with the debugger as well as access to shared data.
  static std::mutex JITDebugLock;
  std::lock_guard<std::mutex> Lock(JITDebugLock);

  // Insert this entry at the head of the list.
  jit_code_entry *NextEntry = __jit_debug_descriptor.first_entry;
  E->next_entry = NextEntry;
  if (NextEntry) {
    NextEntry->prev_entry = E;
  }

  __jit_debug_descriptor.first_entry = E;
  __jit_debug_descriptor.relevant_entry = E;
  __jit_debug_descriptor.action_flag = JIT_REGISTER_FN;
}

extern "C" orc::shared::CWrapperFunctionBuffer
llvm_orc_registerJITLoaderGDBAllocAction(const char *ArgData, size_t ArgSize) {
  using namespace orc::shared;
  return WrapperFunction<SPSError(SPSExecutorAddrRange, bool)>::handle(
             ArgData, ArgSize,
             [](ExecutorAddrRange R, bool AutoRegisterCode) {
               appendJITDebugDescriptor(R.Start.toPtr<const char *>(),
                                        R.size());
               // Run into the rendezvous breakpoint.
               if (AutoRegisterCode)
                 __jit_debug_register_code();
               return Error::success();
             })
      .release();
}
