//===-- FormatClasses.cpp -------------------------------------------------===//
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

#include "lldb/DataFormatters/FormatClasses.h"

#include "lldb/DataFormatters/FormatManager.h"





using namespace lldb;
using namespace lldb_private;

FormattersMatchData::FormattersMatchData(ValueObject &valobj,
                                         lldb::DynamicValueType use_dynamic)
    : m_valobj(valobj), m_dynamic_value_type(use_dynamic),
      m_formatters_match_vector({}, false), m_type_for_cache(),
      m_candidate_languages() {
  m_type_for_cache = FormatManager::GetTypeForCache(valobj, use_dynamic);
  m_candidate_languages =
      FormatManager::GetCandidateLanguages(valobj.GetObjectRuntimeLanguage());
}

FormattersMatchVector FormattersMatchData::GetMatchesVector() {
  if (!m_formatters_match_vector.second) {
    m_formatters_match_vector.second = true;
    m_formatters_match_vector.first =
        FormatManager::GetPossibleMatches(m_valobj, m_dynamic_value_type);
  }
  return m_formatters_match_vector.first;
}

ConstString FormattersMatchData::GetTypeForCache() { return m_type_for_cache; }

CandidateLanguagesVector FormattersMatchData::GetCandidateLanguages() {
  return m_candidate_languages;
}

ValueObject &FormattersMatchData::GetValueObject() { return m_valobj; }

lldb::DynamicValueType FormattersMatchData::GetDynamicValueType() {
  return m_dynamic_value_type;
}
