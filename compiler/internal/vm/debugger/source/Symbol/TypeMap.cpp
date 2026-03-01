//===-- TypeMap.cpp -------------------------------------------------------===//
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

#include <vector>

#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_ostream.h"

#include "lldb/Symbol/SymbolFile.h"
#include "lldb/Symbol/SymbolVendor.h"
#include "lldb/Symbol/Type.h"
#include "lldb/Symbol/TypeMap.h"

using namespace lldb;
using namespace lldb_private;

TypeMap::TypeMap() : m_types() {}

// Destructor
TypeMap::~TypeMap() = default;

void TypeMap::Insert(const TypeSP &type_sp) {
  // Just push each type on the back for now. We will worry about uniquing
  // later
  if (type_sp)
    m_types.insert(std::make_pair(type_sp->GetID(), type_sp));
}

bool TypeMap::InsertUnique(const TypeSP &type_sp) {
  if (type_sp) {
    user_id_t type_uid = type_sp->GetID();
    iterator pos, end = m_types.end();

    for (pos = m_types.find(type_uid);
         pos != end && pos->second->GetID() == type_uid; ++pos) {
      if (pos->second.get() == type_sp.get())
        return false;
    }
    Insert(type_sp);
  }
  return true;
}

// Find a base type by its unique ID.
// TypeSP
// TypeMap::FindType(lldb::user_id_t uid)
//{
//    iterator pos = m_types.find(uid);
//    if (pos != m_types.end())
//        return pos->second;
//    return TypeSP();
//}

// Find a type by name.
// TypeMap
// TypeMap::FindTypes (ConstString name)
//{
//    // Do we ever need to make a lookup by name map? Here we are doing
//    // a linear search which isn't going to be fast.
//    TypeMap types(m_ast.getTargetInfo()->getTriple().getTriple().c_str());
//    iterator pos, end;
//    for (pos = m_types.begin(), end = m_types.end(); pos != end; ++pos)
//        if (pos->second->GetName() == name)
//            types.Insert (pos->second);
//    return types;
//}

void TypeMap::Clear() { m_types.clear(); }

uint32_t TypeMap::GetSize() const { return m_types.size(); }

bool TypeMap::Empty() const { return m_types.empty(); }

// GetTypeAtIndex isn't used a lot for large type lists, currently only for
// type lists that are returned for "image dump -t TYPENAME" commands and other
// simple symbol queries that grab the first result...

TypeSP TypeMap::GetTypeAtIndex(uint32_t idx) {
  iterator pos, end;
  uint32_t i = idx;
  for (pos = m_types.begin(), end = m_types.end(); pos != end; ++pos) {
    if (i == 0)
      return pos->second;
    --i;
  }
  return TypeSP();
}

lldb::TypeSP TypeMap::FirstType() const {
  if (m_types.empty())
    return TypeSP();
  return m_types.begin()->second;
}

void TypeMap::ForEach(
    std::function<bool(const lldb::TypeSP &type_sp)> const &callback) const {
  for (auto pos = m_types.begin(), end = m_types.end(); pos != end; ++pos) {
    if (!callback(pos->second))
      break;
  }
}

void TypeMap::ForEach(
    std::function<bool(lldb::TypeSP &type_sp)> const &callback) {
  for (auto pos = m_types.begin(), end = m_types.end(); pos != end; ++pos) {
    if (!callback(pos->second))
      break;
  }
}

bool TypeMap::Remove(const lldb::TypeSP &type_sp) {
  if (type_sp) {
    lldb::user_id_t uid = type_sp->GetID();
    for (iterator pos = m_types.find(uid), end = m_types.end();
         pos != end && pos->first == uid; ++pos) {
      if (pos->second == type_sp) {
        m_types.erase(pos);
        return true;
      }
    }
  }
  return false;
}

void TypeMap::Dump(Stream *s, bool show_context,
                   lldb::DescriptionLevel level) const {
  for (const auto &pair : m_types)
    pair.second->Dump(s, show_context, level);
}
