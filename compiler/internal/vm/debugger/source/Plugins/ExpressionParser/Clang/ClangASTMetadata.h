//===-- ClangASTMetadata.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGASTMETADATA_H
#define LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGASTMETADATA_H

#include "lldb/Core/dwarf.h"
#include "lldb/lldb-defines.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-private-enumerations.h"

namespace lldb_private {

class ClangASTMetadata {
public:
  ClangASTMetadata()
      : m_user_id(0), m_union_is_user_id(false), m_union_is_isa_ptr(false),
        m_has_object_ptr(false), m_is_self(false),
        m_is_forcefully_completed(false) {
    SetIsDynamicCXXType(std::nullopt);
  }

  std::optional<bool> GetIsDynamicCXXType() const;

  void SetIsDynamicCXXType(std::optional<bool> b);

  void SetUserID(lldb::user_id_t user_id) {
    m_user_id = user_id;
    m_union_is_user_id = true;
    m_union_is_isa_ptr = false;
  }

  lldb::user_id_t GetUserID() const {
    if (m_union_is_user_id)
      return m_user_id;
    else
      return LLDB_INVALID_UID;
  }

  void SetISAPtr(uint64_t isa_ptr) {
    m_isa_ptr = isa_ptr;
    m_union_is_user_id = false;
    m_union_is_isa_ptr = true;
  }

  uint64_t GetISAPtr() const {
    if (m_union_is_isa_ptr)
      return m_isa_ptr;
    else
      return 0;
  }

  void SetObjectPtrName(const char *name) {
    m_has_object_ptr = true;
    if (strcmp(name, "self") == 0)
      m_is_self = true;
    else if (strcmp(name, "this") == 0)
      m_is_self = false;
    else
      m_has_object_ptr = false;
  }

  lldb::LanguageType GetObjectPtrLanguage() const {
    if (m_has_object_ptr) {
      if (m_is_self)
        return lldb::eLanguageTypeObjC;
      else
        return lldb::eLanguageTypeC_plus_plus;
    }
    return lldb::eLanguageTypeUnknown;
  }

  const char *GetObjectPtrName() const {
    if (m_has_object_ptr) {
      if (m_is_self)
        return "self";
      else
        return "this";
    } else
      return nullptr;
  }

  bool HasObjectPtr() const { return m_has_object_ptr; }

  /// A type is "forcefully completed" if it was declared complete to satisfy an
  /// AST invariant (e.g. base classes must be complete types), but in fact we
  /// were not able to find a actual definition for it.
  bool IsForcefullyCompleted() const { return m_is_forcefully_completed; }

  void SetIsForcefullyCompleted(bool value = true) {
    m_is_forcefully_completed = true;
  }

  void Dump(Stream *s);

private:
  union {
    lldb::user_id_t m_user_id;
    uint64_t m_isa_ptr;
  };

  unsigned m_union_is_user_id : 1, m_union_is_isa_ptr : 1, m_has_object_ptr : 1,
      m_is_self : 1, m_is_dynamic_cxx : 2, m_is_forcefully_completed : 1;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGASTMETADATA_H
