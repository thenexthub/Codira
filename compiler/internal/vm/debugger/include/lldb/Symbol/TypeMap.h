//===-- TypeMap.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SYMBOL_TYPEMAP_H
#define LLDB_SYMBOL_TYPEMAP_H

#include "lldb/Symbol/Type.h"
#include "lldb/Utility/Iterable.h"
#include "lldb/lldb-private.h"
#include <functional>
#include <map>

namespace lldb_private {

class TypeMap {
public:
  // Constructors and Destructors
  TypeMap();

  virtual ~TypeMap();

  void Clear();

  void Dump(Stream *s, bool show_context,
            lldb::DescriptionLevel level = lldb::eDescriptionLevelFull) const;

  TypeMap FindTypes(ConstString name);

  void Insert(const lldb::TypeSP &type);

  bool Empty() const;

  bool InsertUnique(const lldb::TypeSP &type);

  uint32_t GetSize() const;

  lldb::TypeSP GetTypeAtIndex(uint32_t idx);

  lldb::TypeSP FirstType() const;

  typedef std::multimap<lldb::user_id_t, lldb::TypeSP> collection;
  typedef llvm::iterator_range<ValueMapIterator<collection::const_iterator>>
      TypeIterable;

  TypeIterable Types() const { return TypeIterable(m_types); }

  void ForEach(
      std::function<bool(const lldb::TypeSP &type_sp)> const &callback) const;

  void ForEach(std::function<bool(lldb::TypeSP &type_sp)> const &callback);

  bool Remove(const lldb::TypeSP &type_sp);

private:
  typedef collection::iterator iterator;
  typedef collection::const_iterator const_iterator;

  collection m_types;

  TypeMap(const TypeMap &) = delete;
  const TypeMap &operator=(const TypeMap &) = delete;
};

} // namespace lldb_private

#endif // LLDB_SYMBOL_TYPEMAP_H
