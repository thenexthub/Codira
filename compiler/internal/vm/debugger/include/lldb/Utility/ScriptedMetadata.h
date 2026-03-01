//===-- ScriptedMetadata.h ------------------------------------ -*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_SCRIPTEDMETADATA_H
#define LLDB_INTERPRETER_SCRIPTEDMETADATA_H

#include "lldb/Utility/ProcessInfo.h"
#include "lldb/Utility/StreamString.h"
#include "lldb/Utility/StructuredData.h"
#include "llvm/ADT/Hashing.h"

namespace lldb_private {
class ScriptedMetadata {
public:
  ScriptedMetadata(llvm::StringRef class_name,
                   StructuredData::DictionarySP dict_sp)
      : m_class_name(class_name.data()), m_args_sp(dict_sp) {}

  ScriptedMetadata(const ProcessInfo &process_info) {
    lldb::ScriptedMetadataSP metadata_sp = process_info.GetScriptedMetadata();
    if (metadata_sp) {
      m_class_name = metadata_sp->GetClassName();
      m_args_sp = metadata_sp->GetArgsSP();
    }
  }

  ScriptedMetadata(const ScriptedMetadata &other)
      : m_class_name(other.m_class_name), m_args_sp(other.m_args_sp) {}

  explicit operator bool() const { return !m_class_name.empty(); }

  llvm::StringRef GetClassName() const { return m_class_name; }
  StructuredData::DictionarySP GetArgsSP() const { return m_args_sp; }

  /// Get a unique identifier for this metadata based on its contents.
  /// The ID is computed from the class name and arguments dictionary,
  /// not from the pointer address, so two metadata objects with the same
  /// contents will have the same ID.
  uint32_t GetID() const {
    if (m_class_name.empty())
      return 0;

    // Hash the class name.
    llvm::hash_code hash = llvm::hash_value(m_class_name);

    // Hash the arguments dictionary if present.
    if (m_args_sp) {
      StreamString ss;
      m_args_sp->GetDescription(ss);
      hash = llvm::hash_combine(hash, llvm::hash_value(ss.GetData()));
    }

    // Return the lower 32 bits of the hash.
    return static_cast<uint32_t>(hash);
  }

private:
  std::string m_class_name;
  StructuredData::DictionarySP m_args_sp;
};
} // namespace lldb_private

#endif // LLDB_INTERPRETER_SCRIPTEDMETADATA_H
