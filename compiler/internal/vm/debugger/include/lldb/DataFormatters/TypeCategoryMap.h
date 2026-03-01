//===-- TypeCategoryMap.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_DATAFORMATTERS_TYPECATEGORYMAP_H
#define LLDB_DATAFORMATTERS_TYPECATEGORYMAP_H

#include <functional>
#include <list>
#include <map>
#include <mutex>

#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-public.h"

#include "lldb/DataFormatters/FormatClasses.h"
#include "lldb/DataFormatters/FormattersContainer.h"
#include "lldb/DataFormatters/TypeCategory.h"

namespace lldb_private {
class TypeCategoryMap {
private:
  typedef std::list<lldb::TypeCategoryImplSP> ActiveCategoriesList;
  typedef ActiveCategoriesList::iterator ActiveCategoriesIterator;

public:
  typedef ConstString KeyType;
  typedef std::map<KeyType, lldb::TypeCategoryImplSP> MapType;
  typedef MapType::iterator MapIterator;
  typedef std::function<bool(const lldb::TypeCategoryImplSP &)> ForEachCallback;

  typedef uint32_t Position;

  static const Position First = 0;
  static const Position Default = 1;
  static const Position Last = UINT32_MAX;

  TypeCategoryMap(IFormatChangeListener *lst);

  void Add(KeyType name, const lldb::TypeCategoryImplSP &entry);

  bool Delete(KeyType name);

  bool Enable(KeyType category_name, Position pos = Default);

  bool Disable(KeyType category_name);

  bool Enable(lldb::TypeCategoryImplSP category, Position pos = Default);

  bool Disable(lldb::TypeCategoryImplSP category);

  void EnableAllCategories();

  void DisableAllCategories();

  void Clear();

  bool Get(KeyType name, lldb::TypeCategoryImplSP &entry);

  void ForEach(ForEachCallback callback);

  lldb::TypeCategoryImplSP GetAtIndex(uint32_t);

  bool
  AnyMatches(const FormattersMatchCandidate &candidate_type,
             TypeCategoryImpl::FormatCategoryItems items =
                 TypeCategoryImpl::ALL_ITEM_TYPES,
             bool only_enabled = true, const char **matching_category = nullptr,
             TypeCategoryImpl::FormatCategoryItems *matching_type = nullptr);

  uint32_t GetCount() { return m_map.size(); }

  template <typename ImplSP> void Get(FormattersMatchData &, ImplSP &);

private:
  class delete_matching_categories {
    lldb::TypeCategoryImplSP ptr;

  public:
    delete_matching_categories(lldb::TypeCategoryImplSP p)
        : ptr(std::move(p)) {}

    bool operator()(const lldb::TypeCategoryImplSP &other) {
      return ptr.get() == other.get();
    }
  };

  std::recursive_mutex m_map_mutex;
  IFormatChangeListener *listener;

  MapType m_map;
  ActiveCategoriesList m_active_categories;
};
} // namespace lldb_private

#endif // LLDB_DATAFORMATTERS_TYPECATEGORYMAP_H
