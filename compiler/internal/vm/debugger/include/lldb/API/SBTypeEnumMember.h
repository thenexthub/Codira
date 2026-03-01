
//===-- SBTypeEnumMember.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBTYPEENUMMEMBER_H
#define LLDB_API_SBTYPEENUMMEMBER_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBTypeEnumMember {
public:
  SBTypeEnumMember();

  SBTypeEnumMember(const SBTypeEnumMember &rhs);

  ~SBTypeEnumMember();

  SBTypeEnumMember &operator=(const SBTypeEnumMember &rhs);

  explicit operator bool() const;

  bool IsValid() const;

  int64_t GetValueAsSigned();

  uint64_t GetValueAsUnsigned();

  const char *GetName();

  lldb::SBType GetType();

  bool GetDescription(lldb::SBStream &description,
                      lldb::DescriptionLevel description_level);

protected:
  friend class SBType;
  friend class SBTypeEnumMemberList;

  void reset(lldb_private::TypeEnumMemberImpl *);

  lldb_private::TypeEnumMemberImpl &ref();

  const lldb_private::TypeEnumMemberImpl &ref() const;

  lldb::TypeEnumMemberImplSP m_opaque_sp;

  SBTypeEnumMember(const lldb::TypeEnumMemberImplSP &);
};

class SBTypeEnumMemberList {
public:
  SBTypeEnumMemberList();

  SBTypeEnumMemberList(const SBTypeEnumMemberList &rhs);

  ~SBTypeEnumMemberList();

  SBTypeEnumMemberList &operator=(const SBTypeEnumMemberList &rhs);

  explicit operator bool() const;

  bool IsValid();

  void Append(SBTypeEnumMember entry);

  SBTypeEnumMember GetTypeEnumMemberAtIndex(uint32_t index);

  uint32_t GetSize();

private:
  std::unique_ptr<lldb_private::TypeEnumMemberListImpl> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBTYPEENUMMEMBER_H
