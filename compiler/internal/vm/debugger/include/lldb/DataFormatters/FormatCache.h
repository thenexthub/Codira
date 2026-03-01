//===-- FormatCache.h ---------------------------------------------*- C++
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

#ifndef LLDB_DATAFORMATTERS_FORMATCACHE_H
#define LLDB_DATAFORMATTERS_FORMATCACHE_H

#include <map>
#include <mutex>

#include "lldb/Utility/ConstString.h"
#include "lldb/lldb-public.h"

namespace lldb_private {
class FormatCache {
private:
  struct Entry {
  private:
    bool m_format_cached : 1;
    bool m_summary_cached : 1;
    bool m_synthetic_cached : 1;

    lldb::TypeFormatImplSP m_format_sp;
    lldb::TypeSummaryImplSP m_summary_sp;
    lldb::SyntheticChildrenSP m_synthetic_sp;

  public:
    Entry();

    template<typename ImplSP> bool IsCached();
    bool IsFormatCached();
    bool IsSummaryCached();
    bool IsSyntheticCached();

    void Get(lldb::TypeFormatImplSP &);
    void Get(lldb::TypeSummaryImplSP &);
    void Get(lldb::SyntheticChildrenSP &);

    void Set(lldb::TypeFormatImplSP);
    void Set(lldb::TypeSummaryImplSP);
    void Set(lldb::SyntheticChildrenSP);
  };
  std::map<ConstString, Entry> m_entries;
  std::recursive_mutex m_mutex;

  uint64_t m_cache_hits = 0;
  uint64_t m_cache_misses = 0;

public:
  FormatCache() = default;

  template <typename ImplSP> bool Get(ConstString type, ImplSP &format_impl_sp);
  void Set(ConstString type, lldb::TypeFormatImplSP &format_sp);
  void Set(ConstString type, lldb::TypeSummaryImplSP &summary_sp);
  void Set(ConstString type, lldb::SyntheticChildrenSP &synthetic_sp);

  void Clear();

  uint64_t GetCacheHits() { return m_cache_hits; }

  uint64_t GetCacheMisses() { return m_cache_misses; }
};

} // namespace lldb_private

#endif // LLDB_DATAFORMATTERS_FORMATCACHE_H
