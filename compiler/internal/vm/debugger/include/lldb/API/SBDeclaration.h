//===-- SBDeclaration.h -------------------------------------------*- C++
//-*-===//
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

#ifndef LLDB_API_SBDECLARATION_H
#define LLDB_API_SBDECLARATION_H

#include "lldb/API/SBDefines.h"
#include "lldb/API/SBFileSpec.h"

namespace lldb {

class LLDB_API SBDeclaration {
public:
  SBDeclaration();

  SBDeclaration(const lldb::SBDeclaration &rhs);

  ~SBDeclaration();

  const lldb::SBDeclaration &operator=(const lldb::SBDeclaration &rhs);

  explicit operator bool() const;

  bool IsValid() const;

  lldb::SBFileSpec GetFileSpec() const;

  uint32_t GetLine() const;

  uint32_t GetColumn() const;

  void SetFileSpec(lldb::SBFileSpec filespec);

  void SetLine(uint32_t line);

  void SetColumn(uint32_t column);

  bool operator==(const lldb::SBDeclaration &rhs) const;

  bool operator!=(const lldb::SBDeclaration &rhs) const;

  bool GetDescription(lldb::SBStream &description);

protected:
  lldb_private::Declaration *get();

private:
  friend class SBValue;

  const lldb_private::Declaration *operator->() const;

  lldb_private::Declaration &ref();

  const lldb_private::Declaration &ref() const;

  SBDeclaration(const lldb_private::Declaration *lldb_object_ptr);

  void SetDeclaration(const lldb_private::Declaration &lldb_object_ref);

  std::unique_ptr<lldb_private::Declaration> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBDECLARATION_H
