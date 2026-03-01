//===-- SBCompileUnit.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBCOMPILEUNIT_H
#define LLDB_API_SBCOMPILEUNIT_H

#include "lldb/API/SBDefines.h"
#include "lldb/API/SBFileSpec.h"

namespace lldb {

class LLDB_API SBCompileUnit {
public:
  SBCompileUnit();

  SBCompileUnit(const lldb::SBCompileUnit &rhs);

  ~SBCompileUnit();

  const lldb::SBCompileUnit &operator=(const lldb::SBCompileUnit &rhs);

  explicit operator bool() const;

  bool IsValid() const;

  lldb::SBFileSpec GetFileSpec() const;

  uint32_t GetNumLineEntries() const;

  lldb::SBLineEntry GetLineEntryAtIndex(uint32_t idx) const;

  uint32_t FindLineEntryIndex(lldb::SBLineEntry &line_entry,
                              bool exact = false) const;

  uint32_t FindLineEntryIndex(uint32_t start_idx, uint32_t line,
                              lldb::SBFileSpec *inline_file_spec) const;

  uint32_t FindLineEntryIndex(uint32_t start_idx, uint32_t line,
                              lldb::SBFileSpec *inline_file_spec,
                              bool exact) const;

  SBFileSpec GetSupportFileAtIndex(uint32_t idx) const;

  uint32_t GetNumSupportFiles() const;

  uint32_t FindSupportFileIndex(uint32_t start_idx, const SBFileSpec &sb_file,
                                bool full);

  /// Get all types matching \a type_mask from debug info in this
  /// compile unit.
  ///
  /// \param[in] type_mask
  ///    A bitfield that consists of one or more bits logically OR'ed
  ///    together from the lldb::TypeClass enumeration. This allows
  ///    you to request only structure types, or only class, struct
  ///    and union types. Passing in lldb::eTypeClassAny will return
  ///    all types found in the debug information for this compile
  ///    unit.
  ///
  /// \return
  ///    A list of types in this compile unit that match \a type_mask
  lldb::SBTypeList GetTypes(uint32_t type_mask = lldb::eTypeClassAny);

  lldb::LanguageType GetLanguage();

  bool operator==(const lldb::SBCompileUnit &rhs) const;

  bool operator!=(const lldb::SBCompileUnit &rhs) const;

  bool GetDescription(lldb::SBStream &description);

private:
  friend class SBAddress;
  friend class SBFrame;
  friend class SBSymbolContext;
  friend class SBModule;

  SBCompileUnit(lldb_private::CompileUnit *lldb_object_ptr);

  const lldb_private::CompileUnit *operator->() const;

  const lldb_private::CompileUnit &operator*() const;

  lldb_private::CompileUnit *get();

  void reset(lldb_private::CompileUnit *lldb_object_ptr);

  lldb_private::CompileUnit *m_opaque_ptr = nullptr;
};

} // namespace lldb

#endif // LLDB_API_SBCOMPILEUNIT_H
