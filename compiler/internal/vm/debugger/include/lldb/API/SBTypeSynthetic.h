//===-- SBTypeSynthetic.h -----------------------------------------*- C++
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

#ifndef LLDB_API_SBTYPESYNTHETIC_H
#define LLDB_API_SBTYPESYNTHETIC_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBTypeSynthetic {
public:
  SBTypeSynthetic();

  static SBTypeSynthetic
  CreateWithClassName(const char *data,
                      uint32_t options = 0); // see lldb::eTypeOption values

  static SBTypeSynthetic
  CreateWithScriptCode(const char *data,
                       uint32_t options = 0); // see lldb::eTypeOption values

  SBTypeSynthetic(const lldb::SBTypeSynthetic &rhs);

  ~SBTypeSynthetic();

  explicit operator bool() const;

  bool IsValid() const;

  bool IsClassCode();

  bool IsClassName();

  const char *GetData();

  void SetClassName(const char *data);

  void SetClassCode(const char *data);

  uint32_t GetOptions();

  void SetOptions(uint32_t);

  bool GetDescription(lldb::SBStream &description,
                      lldb::DescriptionLevel description_level);

  lldb::SBTypeSynthetic &operator=(const lldb::SBTypeSynthetic &rhs);

  bool IsEqualTo(lldb::SBTypeSynthetic &rhs);

  bool operator==(lldb::SBTypeSynthetic &rhs);

  bool operator!=(lldb::SBTypeSynthetic &rhs);

protected:
  friend class SBDebugger;
  friend class SBTypeCategory;
  friend class SBValue;

  lldb::ScriptedSyntheticChildrenSP GetSP();

  void SetSP(const lldb::ScriptedSyntheticChildrenSP &typefilter_impl_sp);

  lldb::ScriptedSyntheticChildrenSP m_opaque_sp;

  SBTypeSynthetic(const lldb::ScriptedSyntheticChildrenSP &);

  bool CopyOnWrite_Impl();
};

} // namespace lldb

#endif // LLDB_API_SBTYPESYNTHETIC_H
