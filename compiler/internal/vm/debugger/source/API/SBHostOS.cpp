//===-- SBHostOS.cpp ------------------------------------------------------===//
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

#include "lldb/API/SBHostOS.h"
#include "lldb/API/SBError.h"
#include "lldb/Host/Config.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/Host.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Host/HostNativeThread.h"
#include "lldb/Host/HostThread.h"
#include "lldb/Host/ThreadLauncher.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/Instrumentation.h"

#include "Plugins/ExpressionParser/Clang/ClangHost.h"
#if LLDB_ENABLE_PYTHON
#include "Plugins/ScriptInterpreter/Python/ScriptInterpreterPython.h"
#endif

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Path.h"

using namespace lldb;
using namespace lldb_private;

SBFileSpec SBHostOS::GetProgramFileSpec() {
  LLDB_INSTRUMENT();

  SBFileSpec sb_filespec;
  sb_filespec.SetFileSpec(HostInfo::GetProgramFileSpec());
  return sb_filespec;
}

SBFileSpec SBHostOS::GetLLDBPythonPath() {
  LLDB_INSTRUMENT();

  return GetLLDBPath(ePathTypePythonDir);
}

SBFileSpec SBHostOS::GetLLDBPath(lldb::PathType path_type) {
  LLDB_INSTRUMENT_VA(path_type);

  FileSpec fspec;
  switch (path_type) {
  case ePathTypeLLDBShlibDir:
    fspec = HostInfo::GetShlibDir();
    break;
  case ePathTypeSupportExecutableDir:
    fspec = HostInfo::GetSupportExeDir();
    break;
  case ePathTypeHeaderDir:
    fspec = HostInfo::GetHeaderDir();
    break;
  case ePathTypePythonDir:
#if LLDB_ENABLE_PYTHON
    fspec = ScriptInterpreterPython::GetPythonDir();
#endif
    break;
  case ePathTypeLLDBSystemPlugins:
    fspec = HostInfo::GetSystemPluginDir();
    break;
  case ePathTypeLLDBUserPlugins:
    fspec = HostInfo::GetUserPluginDir();
    break;
  case ePathTypeLLDBTempSystemDir:
    fspec = HostInfo::GetProcessTempDir();
    break;
  case ePathTypeGlobalLLDBTempSystemDir:
    fspec = HostInfo::GetGlobalTempDir();
    break;
  case ePathTypeClangDir:
    fspec = GetClangResourceDir();
    break;
  }

  SBFileSpec sb_fspec;
  sb_fspec.SetFileSpec(fspec);
  return sb_fspec;
}

SBFileSpec SBHostOS::GetUserHomeDirectory() {
  LLDB_INSTRUMENT();
  return HostInfo::GetUserHomeDir();
}

lldb::thread_t SBHostOS::ThreadCreate(const char *name,
                                      lldb::thread_func_t thread_function,
                                      void *thread_arg, SBError *error_ptr) {
  LLDB_INSTRUMENT_VA(name, thread_function, thread_arg, error_ptr);
  return LLDB_INVALID_HOST_THREAD;
}

void SBHostOS::ThreadCreated(const char *name) { LLDB_INSTRUMENT_VA(name); }

bool SBHostOS::ThreadCancel(lldb::thread_t thread, SBError *error_ptr) {
  LLDB_INSTRUMENT_VA(thread, error_ptr);
  return false;
}

bool SBHostOS::ThreadDetach(lldb::thread_t thread, SBError *error_ptr) {
  LLDB_INSTRUMENT_VA(thread, error_ptr);
  return false;
}

bool SBHostOS::ThreadJoin(lldb::thread_t thread, lldb::thread_result_t *result,
                          SBError *error_ptr) {
  LLDB_INSTRUMENT_VA(thread, result, error_ptr);
  return false;
}
