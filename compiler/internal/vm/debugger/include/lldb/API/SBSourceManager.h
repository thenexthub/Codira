//===-- SBSourceManager.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBSOURCEMANAGER_H
#define LLDB_API_SBSOURCEMANAGER_H

#include "lldb/API/SBDefines.h"

#include <cstdio>

namespace lldb {

class LLDB_API SBSourceManager {
public:
  SBSourceManager(const SBDebugger &debugger);
  SBSourceManager(const SBTarget &target);
  SBSourceManager(const SBSourceManager &rhs);

  ~SBSourceManager();

  const lldb::SBSourceManager &operator=(const lldb::SBSourceManager &rhs);

  size_t DisplaySourceLinesWithLineNumbers(
      const lldb::SBFileSpec &file, uint32_t line, uint32_t context_before,
      uint32_t context_after, const char *current_line_cstr, lldb::SBStream &s);

  size_t DisplaySourceLinesWithLineNumbersAndColumn(
      const lldb::SBFileSpec &file, uint32_t line, uint32_t column,
      uint32_t context_before, uint32_t context_after,
      const char *current_line_cstr, lldb::SBStream &s);

protected:
  friend class SBCommandInterpreter;
  friend class SBDebugger;

private:
  std::unique_ptr<lldb_private::SourceManagerImpl> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBSOURCEMANAGER_H
