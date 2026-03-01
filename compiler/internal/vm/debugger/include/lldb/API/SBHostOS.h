//===-- SBHostOS.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBHOSTOS_H
#define LLDB_API_SBHOSTOS_H

#include "lldb/API/SBDefines.h"
#include "lldb/API/SBFileSpec.h"

namespace lldb {

class LLDB_API SBHostOS {
public:
  static lldb::SBFileSpec GetProgramFileSpec();

  static lldb::SBFileSpec GetLLDBPythonPath();

  static lldb::SBFileSpec GetLLDBPath(lldb::PathType path_type);

  static lldb::SBFileSpec GetUserHomeDirectory();

  LLDB_DEPRECATED("Threading functionality in SBHostOS is not well supported, "
                  "not portable, and is difficult to use from Python.")
  static void ThreadCreated(const char *name);

  LLDB_DEPRECATED("Threading functionality in SBHostOS is not well supported, "
                  "not portable, and is difficult to use from Python.")
  static lldb::thread_t ThreadCreate(const char *name,
                                     lldb::thread_func_t thread_function,
                                     void *thread_arg, lldb::SBError *err);

  LLDB_DEPRECATED("Threading functionality in SBHostOS is not well supported, "
                  "not portable, and is difficult to use from Python.")
  static bool ThreadCancel(lldb::thread_t thread, lldb::SBError *err);

  LLDB_DEPRECATED("Threading functionality in SBHostOS is not well supported, "
                  "not portable, and is difficult to use from Python.")
  static bool ThreadDetach(lldb::thread_t thread, lldb::SBError *err);

  LLDB_DEPRECATED("Threading functionality in SBHostOS is not well supported, "
                  "not portable, and is difficult to use from Python.")
  static bool ThreadJoin(lldb::thread_t thread, lldb::thread_result_t *result,
                         lldb::SBError *err);

private:
};

} // namespace lldb

#endif // LLDB_API_SBHOSTOS_H
