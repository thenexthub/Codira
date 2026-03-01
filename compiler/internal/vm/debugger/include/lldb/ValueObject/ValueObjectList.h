//===-- ValueObjectList.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_VALUEOBJECT_VALUEOBJECTLIST_H
#define LLDB_VALUEOBJECT_VALUEOBJECTLIST_H

#include "lldb/lldb-forward.h"
#include "lldb/lldb-types.h"

#include <vector>

#include <cstddef>

namespace lldb_private {
class ValueObject;

/// A collection of ValueObject values that.
class ValueObjectList {
public:
  void Append(const lldb::ValueObjectSP &val_obj_sp);

  void Append(const ValueObjectList &valobj_list);

  lldb::ValueObjectSP FindValueObjectByPointer(ValueObject *valobj);

  size_t GetSize() const;

  void Resize(size_t size);

  lldb::ValueObjectSP GetValueObjectAtIndex(size_t idx);

  lldb::ValueObjectSP RemoveValueObjectAtIndex(size_t idx);

  void SetValueObjectAtIndex(size_t idx, const lldb::ValueObjectSP &valobj_sp);

  lldb::ValueObjectSP FindValueObjectByValueName(const char *name);

  lldb::ValueObjectSP FindValueObjectByUID(lldb::user_id_t uid);

  void Swap(ValueObjectList &value_object_list);

  void Clear() { m_value_objects.clear(); }

  const std::vector<lldb::ValueObjectSP> &GetObjects() const {
    return m_value_objects;
  }

protected:
  typedef std::vector<lldb::ValueObjectSP> collection;
  // Classes that inherit from ValueObjectList can see and modify these
  collection m_value_objects;
};

} // namespace lldb_private

#endif // LLDB_VALUEOBJECT_VALUEOBJECTLIST_H
