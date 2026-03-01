//===-- SBTypeFilter.h --------------------------------------------*- C++
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

#ifndef LLDB_API_SBTYPEFILTER_H
#define LLDB_API_SBTYPEFILTER_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBTypeFilter {
public:
  SBTypeFilter();

  SBTypeFilter(uint32_t options); // see lldb::eTypeOption values

  SBTypeFilter(const lldb::SBTypeFilter &rhs);

  ~SBTypeFilter();

  explicit operator bool() const;

  bool IsValid() const;

  uint32_t GetNumberOfExpressionPaths();

  const char *GetExpressionPathAtIndex(uint32_t i);

  bool ReplaceExpressionPathAtIndex(uint32_t i, const char *item);

  void AppendExpressionPath(const char *item);

  void Clear();

  uint32_t GetOptions();

  void SetOptions(uint32_t);

  bool GetDescription(lldb::SBStream &description,
                      lldb::DescriptionLevel description_level);

  lldb::SBTypeFilter &operator=(const lldb::SBTypeFilter &rhs);

  bool IsEqualTo(lldb::SBTypeFilter &rhs);

  bool operator==(lldb::SBTypeFilter &rhs);

  bool operator!=(lldb::SBTypeFilter &rhs);

protected:
  friend class SBDebugger;
  friend class SBTypeCategory;
  friend class SBValue;

  lldb::TypeFilterImplSP GetSP();

  void SetSP(const lldb::TypeFilterImplSP &typefilter_impl_sp);

  lldb::TypeFilterImplSP m_opaque_sp;

  SBTypeFilter(const lldb::TypeFilterImplSP &);

  bool CopyOnWrite_Impl();
};

} // namespace lldb

#endif // LLDB_API_SBTYPEFILTER_H
