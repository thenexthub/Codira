//===-- SBTypeNameSpecifier.h --------------------------------------*- C++
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

#ifndef LLDB_API_SBTYPENAMESPECIFIER_H
#define LLDB_API_SBTYPENAMESPECIFIER_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBTypeNameSpecifier {
public:
  SBTypeNameSpecifier();

  SBTypeNameSpecifier(const char *name, bool is_regex = false);

  SBTypeNameSpecifier(const char *name,
                      lldb::FormatterMatchType match_type);

  SBTypeNameSpecifier(SBType type);

  SBTypeNameSpecifier(const lldb::SBTypeNameSpecifier &rhs);

  ~SBTypeNameSpecifier();

  explicit operator bool() const;

  bool IsValid() const;

  const char *GetName();

  SBType GetType();

  lldb::FormatterMatchType GetMatchType();

  bool IsRegex();

  bool GetDescription(lldb::SBStream &description,
                      lldb::DescriptionLevel description_level);

  lldb::SBTypeNameSpecifier &operator=(const lldb::SBTypeNameSpecifier &rhs);

  bool IsEqualTo(lldb::SBTypeNameSpecifier &rhs);

  bool operator==(lldb::SBTypeNameSpecifier &rhs);

  bool operator!=(lldb::SBTypeNameSpecifier &rhs);

protected:
  friend class SBDebugger;
  friend class SBTypeCategory;

  lldb::TypeNameSpecifierImplSP GetSP();

  void SetSP(const lldb::TypeNameSpecifierImplSP &type_namespec_sp);

  lldb::TypeNameSpecifierImplSP m_opaque_sp;

  SBTypeNameSpecifier(const lldb::TypeNameSpecifierImplSP &);
};

} // namespace lldb

#endif // LLDB_API_SBTYPENAMESPECIFIER_H
