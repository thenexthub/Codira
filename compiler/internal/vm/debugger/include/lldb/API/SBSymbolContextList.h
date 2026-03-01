//===-- SBSymbolContextList.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBSYMBOLCONTEXTLIST_H
#define LLDB_API_SBSYMBOLCONTEXTLIST_H

#include "lldb/API/SBDefines.h"
#include "lldb/API/SBSymbolContext.h"

namespace lldb {

class LLDB_API SBSymbolContextList {
public:
  SBSymbolContextList();

  SBSymbolContextList(const lldb::SBSymbolContextList &rhs);

  ~SBSymbolContextList();

  const lldb::SBSymbolContextList &
  operator=(const lldb::SBSymbolContextList &rhs);

  explicit operator bool() const;

  bool IsValid() const;

  uint32_t GetSize() const;

  lldb::SBSymbolContext GetContextAtIndex(uint32_t idx);

  bool GetDescription(lldb::SBStream &description);

  void Append(lldb::SBSymbolContext &sc);

  void Append(lldb::SBSymbolContextList &sc_list);

  void Clear();

protected:
  friend class SBModule;
  friend class SBTarget;
  friend class SBCompileUnit;

  lldb_private::SymbolContextList *operator->() const;

  lldb_private::SymbolContextList &operator*() const;

private:
  std::unique_ptr<lldb_private::SymbolContextList> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBSYMBOLCONTEXTLIST_H
