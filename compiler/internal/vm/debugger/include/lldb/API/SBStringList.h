//===-- SBStringList.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBSTRINGLIST_H
#define LLDB_API_SBSTRINGLIST_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBStringList {
public:
  SBStringList();

  SBStringList(const lldb::SBStringList &rhs);

  const SBStringList &operator=(const SBStringList &rhs);

  ~SBStringList();

  explicit operator bool() const;

  bool IsValid() const;

  void AppendString(const char *str);

  void AppendList(const char **strv, int strc);

  void AppendList(const lldb::SBStringList &strings);

  uint32_t GetSize() const;

  const char *GetStringAtIndex(size_t idx);

  const char *GetStringAtIndex(size_t idx) const;

  void Clear();

protected:
  friend class SBCommandInterpreter;
  friend class SBDebugger;
  friend class SBBreakpoint;
  friend class SBBreakpointLocation;
  friend class SBBreakpointName;
  friend class SBStructuredData;

  SBStringList(const lldb_private::StringList *lldb_strings);

  void AppendList(const lldb_private::StringList &strings);

  lldb_private::StringList *operator->();

  const lldb_private::StringList *operator->() const;

  const lldb_private::StringList &operator*() const;

private:
  std::unique_ptr<lldb_private::StringList> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBSTRINGLIST_H
