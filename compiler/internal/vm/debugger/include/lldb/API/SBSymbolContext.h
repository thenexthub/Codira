//===-- SBSymbolContext.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBSYMBOLCONTEXT_H
#define LLDB_API_SBSYMBOLCONTEXT_H

#include "lldb/API/SBBlock.h"
#include "lldb/API/SBCompileUnit.h"
#include "lldb/API/SBDefines.h"
#include "lldb/API/SBFunction.h"
#include "lldb/API/SBLineEntry.h"
#include "lldb/API/SBModule.h"
#include "lldb/API/SBSymbol.h"

namespace lldb_private {
namespace python {
class SWIGBridge;
}
} // namespace lldb_private

namespace lldb {

class LLDB_API SBSymbolContext {
public:
  SBSymbolContext();

  SBSymbolContext(const lldb::SBSymbolContext &rhs);

  ~SBSymbolContext();

  explicit operator bool() const;

  bool IsValid() const;

  const lldb::SBSymbolContext &operator=(const lldb::SBSymbolContext &rhs);

  lldb::SBModule GetModule();
  lldb::SBCompileUnit GetCompileUnit();
  lldb::SBFunction GetFunction();
  lldb::SBBlock GetBlock();
  lldb::SBLineEntry GetLineEntry();
  lldb::SBSymbol GetSymbol();

  void SetModule(lldb::SBModule module);
  void SetCompileUnit(lldb::SBCompileUnit compile_unit);
  void SetFunction(lldb::SBFunction function);
  void SetBlock(lldb::SBBlock block);
  void SetLineEntry(lldb::SBLineEntry line_entry);
  void SetSymbol(lldb::SBSymbol symbol);

  SBSymbolContext GetParentOfInlinedScope(const SBAddress &curr_frame_pc,
                                          SBAddress &parent_frame_addr) const;

  bool GetDescription(lldb::SBStream &description);

protected:
  friend class SBAddress;
  friend class SBFrame;
  friend class SBModule;
  friend class SBThread;
  friend class SBTarget;
  friend class SBSymbolContextList;

  friend class lldb_private::ScriptInterpreter;
  friend class lldb_private::python::SWIGBridge;

  SBSymbolContext(const lldb_private::SymbolContext &sc_ptr);

  lldb_private::SymbolContext *operator->() const;

  lldb_private::SymbolContext &operator*();

  lldb_private::SymbolContext &ref();

  const lldb_private::SymbolContext &operator*() const;

  lldb_private::SymbolContext *get() const;

  friend class lldb_private::ScriptInterpreter;

private:
  std::unique_ptr<lldb_private::SymbolContext> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBSYMBOLCONTEXT_H
