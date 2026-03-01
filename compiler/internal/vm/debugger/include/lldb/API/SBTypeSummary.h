//===-- SBTypeSummary.h -------------------------------------------*- C++
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

#ifndef LLDB_API_SBTYPESUMMARY_H
#define LLDB_API_SBTYPESUMMARY_H

#include "lldb/API/SBDefines.h"

namespace lldb_private {
namespace python {
class SWIGBridge;
}
} // namespace lldb_private

namespace lldb {
class LLDB_API SBTypeSummaryOptions {
public:
  SBTypeSummaryOptions();

  SBTypeSummaryOptions(const lldb::SBTypeSummaryOptions &rhs);


  ~SBTypeSummaryOptions();

  explicit operator bool() const;

  bool IsValid();

  lldb::LanguageType GetLanguage();

  lldb::TypeSummaryCapping GetCapping();

  void SetLanguage(lldb::LanguageType);

  void SetCapping(lldb::TypeSummaryCapping);

protected:
  friend class SBValue;
  friend class SBTypeSummary;

  friend class lldb_private::python::SWIGBridge;

  SBTypeSummaryOptions(const lldb_private::TypeSummaryOptions &lldb_object);

  lldb_private::TypeSummaryOptions *operator->();

  const lldb_private::TypeSummaryOptions *operator->() const;

  lldb_private::TypeSummaryOptions *get();

  lldb_private::TypeSummaryOptions &ref();

  const lldb_private::TypeSummaryOptions &ref() const;

private:
  std::unique_ptr<lldb_private::TypeSummaryOptions> m_opaque_up;
};

class SBTypeSummary {
public:
  SBTypeSummary();

  // Native function summary formatter callback
  typedef bool (*FormatCallback)(SBValue, SBTypeSummaryOptions, SBStream &);

  static SBTypeSummary
  CreateWithSummaryString(const char *data,
                          uint32_t options = 0); // see lldb::eTypeOption values

  static SBTypeSummary
  CreateWithFunctionName(const char *data,
                         uint32_t options = 0); // see lldb::eTypeOption values

  static SBTypeSummary
  CreateWithScriptCode(const char *data,
                       uint32_t options = 0); // see lldb::eTypeOption values

#ifndef SWIG
  static SBTypeSummary CreateWithCallback(FormatCallback cb,
                                          uint32_t options = 0,
                                          const char *description = nullptr);
#endif

  SBTypeSummary(const lldb::SBTypeSummary &rhs);

  ~SBTypeSummary();

  explicit operator bool() const;

  bool IsValid() const;

  bool IsFunctionCode();

  bool IsFunctionName();

  bool IsSummaryString();

  const char *GetData();

  void SetSummaryString(const char *data);

  void SetFunctionName(const char *data);

  void SetFunctionCode(const char *data);

  uint32_t GetPtrMatchDepth();

  void SetPtrMatchDepth(uint32_t ptr_match_depth);

  uint32_t GetOptions();

  void SetOptions(uint32_t);

  bool GetDescription(lldb::SBStream &description,
                      lldb::DescriptionLevel description_level);

  lldb::SBTypeSummary &operator=(const lldb::SBTypeSummary &rhs);

  bool DoesPrintValue(lldb::SBValue value);

  bool IsEqualTo(lldb::SBTypeSummary &rhs);

  bool operator==(lldb::SBTypeSummary &rhs);

  bool operator!=(lldb::SBTypeSummary &rhs);

protected:
  friend class SBDebugger;
  friend class SBTypeCategory;
  friend class SBValue;

  lldb::TypeSummaryImplSP GetSP();

  void SetSP(const lldb::TypeSummaryImplSP &typefilter_impl_sp);

  lldb::TypeSummaryImplSP m_opaque_sp;

  SBTypeSummary(const lldb::TypeSummaryImplSP &);

  bool CopyOnWrite_Impl();

  bool ChangeSummaryType(bool want_script);
};

} // namespace lldb

#endif // LLDB_API_SBTYPESUMMARY_H
