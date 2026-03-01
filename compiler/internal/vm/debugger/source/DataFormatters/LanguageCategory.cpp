//===-- LanguageCategory.cpp ----------------------------------------------===//
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

#include "lldb/DataFormatters/LanguageCategory.h"

#include "lldb/DataFormatters/FormatManager.h"
#include "lldb/DataFormatters/TypeCategory.h"
#include "lldb/DataFormatters/TypeFormat.h"
#include "lldb/DataFormatters/TypeSummary.h"
#include "lldb/DataFormatters/TypeSynthetic.h"
#include "lldb/Target/Language.h"

using namespace lldb;
using namespace lldb_private;

LanguageCategory::LanguageCategory(lldb::LanguageType lang_type)
    : m_category_sp(), m_hardcoded_formats(), m_hardcoded_summaries(),
      m_hardcoded_synthetics(), m_format_cache(), m_enabled(false) {
  if (Language *language_plugin = Language::FindPlugin(lang_type)) {
    m_category_sp = language_plugin->GetFormatters();
    m_hardcoded_formats = language_plugin->GetHardcodedFormats();
    m_hardcoded_summaries = language_plugin->GetHardcodedSummaries();
    m_hardcoded_synthetics = language_plugin->GetHardcodedSynthetics();
  }
  Enable();
}

template<typename ImplSP>
bool LanguageCategory::Get(FormattersMatchData &match_data,
                           ImplSP &retval_sp) {
  if (!m_category_sp)
    return false;

  if (!IsEnabled())
    return false;

  if (match_data.GetTypeForCache()) {
    if (m_format_cache.Get(match_data.GetTypeForCache(), retval_sp))
      return (bool)retval_sp;
  }

  ValueObject &valobj(match_data.GetValueObject());
  bool result = m_category_sp->Get(valobj.GetObjectRuntimeLanguage(),
                                   match_data.GetMatchesVector(), retval_sp);
  if (match_data.GetTypeForCache() &&
      (!retval_sp || !retval_sp->NonCacheable())) {
    m_format_cache.Set(match_data.GetTypeForCache(), retval_sp);
  }
  return result;
}

namespace lldb_private {

/// Explicit instantiations for the three types.
/// \{
template bool
LanguageCategory::Get<lldb::TypeFormatImplSP>(FormattersMatchData &,
                                              lldb::TypeFormatImplSP &);
template bool
LanguageCategory::Get<lldb::TypeSummaryImplSP>(FormattersMatchData &,
                                               lldb::TypeSummaryImplSP &);
template bool
LanguageCategory::Get<lldb::SyntheticChildrenSP>(FormattersMatchData &,
                                                 lldb::SyntheticChildrenSP &);
/// \}

template <>
auto &LanguageCategory::GetHardcodedFinder<lldb::TypeFormatImplSP>() {
  return m_hardcoded_formats;
}

template <>
auto &LanguageCategory::GetHardcodedFinder<lldb::TypeSummaryImplSP>() {
  return m_hardcoded_summaries;
}

template <>
auto &LanguageCategory::GetHardcodedFinder<lldb::SyntheticChildrenSP>() {
  return m_hardcoded_synthetics;
}

} // namespace lldb_private

template <typename ImplSP>
bool LanguageCategory::GetHardcoded(FormatManager &fmt_mgr,
                                    FormattersMatchData &match_data,
                                    ImplSP &retval_sp) {
  if (!IsEnabled())
    return false;

  ValueObject &valobj(match_data.GetValueObject());
  lldb::DynamicValueType use_dynamic(match_data.GetDynamicValueType());

  for (auto &candidate : GetHardcodedFinder<ImplSP>()) {
    if (auto result = candidate(valobj, use_dynamic, fmt_mgr)) {
      retval_sp = result;
      break;
    }
  }
  return (bool)retval_sp;
}

/// Explicit instantiations for the three types.
/// \{
template bool LanguageCategory::GetHardcoded<lldb::TypeFormatImplSP>(
    FormatManager &, FormattersMatchData &, lldb::TypeFormatImplSP &);
template bool LanguageCategory::GetHardcoded<lldb::TypeSummaryImplSP>(
    FormatManager &, FormattersMatchData &, lldb::TypeSummaryImplSP &);
template bool LanguageCategory::GetHardcoded<lldb::SyntheticChildrenSP>(
    FormatManager &, FormattersMatchData &, lldb::SyntheticChildrenSP &);
/// \}

lldb::TypeCategoryImplSP LanguageCategory::GetCategory() const {
  return m_category_sp;
}

FormatCache &LanguageCategory::GetFormatCache() { return m_format_cache; }

void LanguageCategory::Enable() {
  if (m_category_sp)
    m_category_sp->Enable(true, TypeCategoryMap::Default);
  m_enabled = true;
}

void LanguageCategory::Disable() {
  if (m_category_sp)
    m_category_sp->Disable();
  m_enabled = false;
}

bool LanguageCategory::IsEnabled() { return m_enabled; }
