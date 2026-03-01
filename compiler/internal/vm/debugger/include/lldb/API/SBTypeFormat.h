//===-- SBTypeFormat.h --------------------------------------------*- C++
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

#ifndef LLDB_API_SBTYPEFORMAT_H
#define LLDB_API_SBTYPEFORMAT_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBTypeFormat {
public:
  SBTypeFormat();

  SBTypeFormat(lldb::Format format,
               uint32_t options = 0); // see lldb::eTypeOption values

  SBTypeFormat(const char *type,
               uint32_t options = 0); // see lldb::eTypeOption values

  SBTypeFormat(const lldb::SBTypeFormat &rhs);

  ~SBTypeFormat();

  explicit operator bool() const;

  bool IsValid() const;

  lldb::Format GetFormat();

  const char *GetTypeName();

  uint32_t GetOptions();

  void SetFormat(lldb::Format);

  void SetTypeName(const char *);

  void SetOptions(uint32_t);

  bool GetDescription(lldb::SBStream &description,
                      lldb::DescriptionLevel description_level);

  lldb::SBTypeFormat &operator=(const lldb::SBTypeFormat &rhs);

  bool IsEqualTo(lldb::SBTypeFormat &rhs);

  bool operator==(lldb::SBTypeFormat &rhs);

  bool operator!=(lldb::SBTypeFormat &rhs);

protected:
  friend class SBDebugger;
  friend class SBTypeCategory;
  friend class SBValue;

  lldb::TypeFormatImplSP GetSP();

  void SetSP(const lldb::TypeFormatImplSP &typeformat_impl_sp);

  lldb::TypeFormatImplSP m_opaque_sp;

  SBTypeFormat(const lldb::TypeFormatImplSP &);

  enum class Type { eTypeKeepSame, eTypeFormat, eTypeEnum };

  bool CopyOnWrite_Impl(Type);
};

} // namespace lldb

#endif // LLDB_API_SBTYPEFORMAT_H
