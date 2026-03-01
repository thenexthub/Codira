//===-- SBBlock.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBBLOCK_H
#define LLDB_API_SBBLOCK_H

#include "lldb/API/SBAddressRange.h"
#include "lldb/API/SBAddressRangeList.h"
#include "lldb/API/SBDefines.h"
#include "lldb/API/SBFrame.h"
#include "lldb/API/SBTarget.h"
#include "lldb/API/SBValueList.h"

namespace lldb {

class LLDB_API SBBlock {
public:
  SBBlock();

  SBBlock(const lldb::SBBlock &rhs);

  ~SBBlock();

  const lldb::SBBlock &operator=(const lldb::SBBlock &rhs);

  bool IsInlined() const;

  explicit operator bool() const;

  bool IsValid() const;

  const char *GetInlinedName() const;

  lldb::SBFileSpec GetInlinedCallSiteFile() const;

  uint32_t GetInlinedCallSiteLine() const;

  uint32_t GetInlinedCallSiteColumn() const;

  lldb::SBBlock GetParent();

  lldb::SBBlock GetSibling();

  lldb::SBBlock GetFirstChild();

  uint32_t GetNumRanges();

  lldb::SBAddress GetRangeStartAddress(uint32_t idx);

  lldb::SBAddress GetRangeEndAddress(uint32_t idx);

  lldb::SBAddressRangeList GetRanges();

  uint32_t GetRangeIndexForBlockAddress(lldb::SBAddress block_addr);

  lldb::SBValueList GetVariables(lldb::SBFrame &frame, bool arguments,
                                 bool locals, bool statics,
                                 lldb::DynamicValueType use_dynamic);

  lldb::SBValueList GetVariables(lldb::SBTarget &target, bool arguments,
                                 bool locals, bool statics);
  /// Get the inlined block that contains this block.
  ///
  /// \return
  ///     If this block is inlined, it will return this block, else
  ///     parent blocks will be searched to see if any contain this
  ///     block and are themselves inlined. An invalid SBBlock will
  ///     be returned if this block nor any parent blocks are inlined
  ///     function blocks.
  lldb::SBBlock GetContainingInlinedBlock();

  bool GetDescription(lldb::SBStream &description);

private:
  friend class SBAddress;
  friend class SBFrame;
  friend class SBFunction;
  friend class SBSymbolContext;

  lldb_private::Block *GetPtr();

  void SetPtr(lldb_private::Block *lldb_object_ptr);

  SBBlock(lldb_private::Block *lldb_object_ptr);

  void AppendVariables(bool can_create, bool get_parent_variables,
                       lldb_private::VariableList *var_list);

  lldb_private::Block *m_opaque_ptr = nullptr;
};

} // namespace lldb

#endif // LLDB_API_SBBLOCK_H
