//===-- LanguageCategory.h----------------------------------------*- C++
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

#ifndef LLDB_DATAFORMATTERS_LANGUAGECATEGORY_H
#define LLDB_DATAFORMATTERS_LANGUAGECATEGORY_H

#include "lldb/DataFormatters/FormatCache.h"
#include "lldb/DataFormatters/FormatClasses.h"
#include "lldb/lldb-public.h"

#include <memory>

namespace lldb_private {

class LanguageCategory {
public:
  typedef std::unique_ptr<LanguageCategory> UniquePointer;

  LanguageCategory(lldb::LanguageType lang_type);

  template <typename ImplSP>
  bool Get(FormattersMatchData &match_data, ImplSP &format_sp);
  template <typename ImplSP>
  bool GetHardcoded(FormatManager &fmt_mgr, FormattersMatchData &match_data,
                    ImplSP &format_sp);

  lldb::TypeCategoryImplSP GetCategory() const;

  FormatCache &GetFormatCache();

  void Enable();

  void Disable();

  bool IsEnabled();

private:
  lldb::TypeCategoryImplSP m_category_sp;

  HardcodedFormatters::HardcodedFormatFinder m_hardcoded_formats;
  HardcodedFormatters::HardcodedSummaryFinder m_hardcoded_summaries;
  HardcodedFormatters::HardcodedSyntheticFinder m_hardcoded_synthetics;

  template <typename ImplSP>
  auto &GetHardcodedFinder();

  lldb_private::FormatCache m_format_cache;

  bool m_enabled;
};

} // namespace lldb_private

#endif // LLDB_DATAFORMATTERS_LANGUAGECATEGORY_H
