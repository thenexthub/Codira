//===-- WindowsMiniDump.cpp -----------------------------------------------===//
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

// This function is separated out from ObjectFilePECOFF.cpp to name avoid name
// collisions with WinAPI preprocessor macros.

#include "WindowsMiniDump.h"
#include "lldb/Utility/FileSpec.h"
#include "llvm/Support/ConvertUTF.h"

#ifdef _WIN32
#include "lldb/Host/windows/windows.h"
#include <dbghelp.h>
#endif

namespace lldb_private {

bool SaveMiniDump(const lldb::ProcessSP &process_sp,
                  SaveCoreOptions &core_options, lldb_private::Status &error) {
  if (!process_sp)
    return false;
#ifdef _WIN32
  std::optional<FileSpec> outfileSpec = core_options.GetOutputFile();
  const auto &outfile = outfileSpec.value();
  HANDLE process_handle = ::OpenProcess(
      PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_sp->GetID());
  const std::string file_name = outfile.GetPath();
  std::wstring wide_name;
  wide_name.resize(file_name.size() + 1);
  char *result_ptr = reinterpret_cast<char *>(&wide_name[0]);
  const llvm::UTF8 *error_ptr = nullptr;
  if (!llvm::ConvertUTF8toWide(sizeof(wchar_t), file_name, result_ptr,
                               error_ptr)) {
    error = Status::FromErrorString("cannot convert file name");
    return false;
  }
  HANDLE file_handle =
      ::CreateFileW(wide_name.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  const auto result =
      ::MiniDumpWriteDump(process_handle, process_sp->GetID(), file_handle,
                          MiniDumpWithFullMemoryInfo, NULL, NULL, NULL);
  ::CloseHandle(file_handle);
  ::CloseHandle(process_handle);
  if (!result) {
    error = Status(::GetLastError(), lldb::eErrorTypeWin32);
    return false;
  }
  return true;
#endif
  return false;
}

} // namesapce lldb_private
