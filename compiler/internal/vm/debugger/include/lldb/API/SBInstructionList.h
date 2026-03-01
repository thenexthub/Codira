//===-- SBInstructionList.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBINSTRUCTIONLIST_H
#define LLDB_API_SBINSTRUCTIONLIST_H

#include "lldb/API/SBDefines.h"

#include <cstdio>

namespace lldb {

class LLDB_API SBInstructionList {
public:
  SBInstructionList();

  SBInstructionList(const SBInstructionList &rhs);

  const SBInstructionList &operator=(const SBInstructionList &rhs);

  ~SBInstructionList();

  explicit operator bool() const;

  bool IsValid() const;

  size_t GetSize();

  lldb::SBInstruction GetInstructionAtIndex(uint32_t idx);

  // Returns the number of instructions between the start and end address. If
  // canSetBreakpoint is true then the count will be the number of
  // instructions on which a breakpoint can be set.
  size_t GetInstructionsCount(const SBAddress &start,
                              const SBAddress &end,
                              bool canSetBreakpoint = false);                                   

  void Clear();

  void AppendInstruction(lldb::SBInstruction inst);

#ifndef SWIG
  void Print(FILE *out);
#endif

  void Print(SBFile out);

  void Print(FileSP BORROWED);

  bool GetDescription(lldb::SBStream &description);

  // Writes assembly instructions to `description` with load addresses using
  // `exe_ctx`.
  bool GetDescription(lldb::SBStream &description,
                      lldb::SBExecutionContext &exe_ctx);

  bool DumpEmulationForAllInstructions(const char *triple);

protected:
  friend class SBFunction;
  friend class SBSymbol;
  friend class SBTarget;

  void SetDisassembler(const lldb::DisassemblerSP &opaque_sp);
  bool GetDescription(lldb_private::Stream &description,
                      lldb_private::ExecutionContext *exe_ctx = nullptr);

private:
  lldb::DisassemblerSP m_opaque_sp;
};

} // namespace lldb

#endif // LLDB_API_SBINSTRUCTIONLIST_H
