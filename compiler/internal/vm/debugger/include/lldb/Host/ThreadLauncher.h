//===-- ThreadLauncher.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_THREADLAUNCHER_H
#define LLDB_HOST_THREADLAUNCHER_H

#include "lldb/Host/HostThread.h"
#include "lldb/lldb-types.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"

namespace lldb_private {

class ThreadLauncher {
public:
  static llvm::Expected<HostThread>
  LaunchThread(llvm::StringRef name,
               std::function<lldb::thread_result_t()> thread_function,
               size_t min_stack_byte_size = 0); // Minimum stack size in bytes,
                                                // set stack size to zero for
                                                // default platform thread stack
                                                // size

  struct HostThreadCreateInfo {
    std::string thread_name;
    std::function<lldb::thread_result_t()> impl;

    HostThreadCreateInfo(std::string thread_name,
                         std::function<lldb::thread_result_t()> impl)
        : thread_name(std::move(thread_name)), impl(std::move(impl)) {}
  };
};
}

#endif
