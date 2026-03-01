//===-- SBFunction.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBFUNCTION_H
#define LLDB_API_SBFUNCTION_H

#include "lldb/API/SBAddress.h"
#include "lldb/API/SBAddressRangeList.h"
#include "lldb/API/SBDefines.h"
#include "lldb/API/SBInstructionList.h"

namespace lldb {

class LLDB_API SBFunction {
public:
  SBFunction();

  SBFunction(const lldb::SBFunction &rhs);

  const lldb::SBFunction &operator=(const lldb::SBFunction &rhs);

  ~SBFunction();

  explicit operator bool() const;

  bool IsValid() const;

  const char *GetName() const;

  const char *GetDisplayName() const;

  const char *GetMangledName() const;

  const char *GetBaseName() const;

  lldb::SBInstructionList GetInstructions(lldb::SBTarget target);

  lldb::SBInstructionList GetInstructions(lldb::SBTarget target,
                                          const char *flavor);

  lldb::SBAddress GetStartAddress();

  LLDB_DEPRECATED_FIXME("Not compatible with discontinuous functions.",
                        "GetRanges()")
  lldb::SBAddress GetEndAddress();

  lldb::SBAddressRangeList GetRanges();

  const char *GetArgumentName(uint32_t arg_idx);

  uint32_t GetPrologueByteSize();

  lldb::SBType GetType();

  lldb::SBBlock GetBlock();

  lldb::LanguageType GetLanguage();

  bool GetIsOptimized();

  bool operator==(const lldb::SBFunction &rhs) const;

  bool operator!=(const lldb::SBFunction &rhs) const;

  bool GetDescription(lldb::SBStream &description);

protected:
  lldb_private::Function *get();

  void reset(lldb_private::Function *lldb_object_ptr);

private:
  friend class SBAddress;
  friend class SBFrame;
  friend class SBSymbolContext;

  SBFunction(lldb_private::Function *lldb_object_ptr);

  lldb_private::Function *m_opaque_ptr = nullptr;
};

} // namespace lldb

#endif // LLDB_API_SBFUNCTION_H
