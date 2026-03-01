//===-- SBAddress.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBADDRESS_H
#define LLDB_API_SBADDRESS_H

#include "lldb/API/SBDefines.h"
#include "lldb/API/SBModule.h"

namespace lldb {

class LLDB_API SBAddress {
public:
  SBAddress();

  SBAddress(const lldb::SBAddress &rhs);

  SBAddress(lldb::SBSection section, lldb::addr_t offset);

  // Create an address by resolving a load address using the supplied target
  SBAddress(lldb::addr_t load_addr, lldb::SBTarget &target);

  ~SBAddress();

  const lldb::SBAddress &operator=(const lldb::SBAddress &rhs);

  explicit operator bool() const;

  // operator== is a free function

  bool operator!=(const SBAddress &rhs) const;

  bool IsValid() const;

  void Clear();

  addr_t GetFileAddress() const;

  addr_t GetLoadAddress(const lldb::SBTarget &target) const;

  void SetAddress(lldb::SBSection section, lldb::addr_t offset);

  void SetLoadAddress(lldb::addr_t load_addr, lldb::SBTarget &target);
  bool OffsetAddress(addr_t offset);

  bool GetDescription(lldb::SBStream &description);

  // The following queries can lookup symbol information for a given address.
  // An address might refer to code or data from an existing module, or it
  // might refer to something on the stack or heap. The following functions
  // will only return valid values if the address has been resolved to a code
  // or data address using "void SBAddress::SetLoadAddress(...)" or
  // "lldb::SBAddress SBTarget::ResolveLoadAddress (...)".
  lldb::SBSymbolContext GetSymbolContext(uint32_t resolve_scope);

  // The following functions grab individual objects for a given address and
  // are less efficient if you want more than one symbol related objects. Use
  // one of the following when you want multiple debug symbol related objects
  // for an address:
  //    lldb::SBSymbolContext SBAddress::GetSymbolContext (uint32_t
  //    resolve_scope);
  //    lldb::SBSymbolContext SBTarget::ResolveSymbolContextForAddress (const
  //    SBAddress &addr, uint32_t resolve_scope);
  // One or more bits from the SymbolContextItem enumerations can be logically
  // OR'ed together to more efficiently retrieve multiple symbol objects.

  lldb::SBSection GetSection();

  lldb::addr_t GetOffset();

  lldb::SBModule GetModule();

  lldb::SBCompileUnit GetCompileUnit();

  lldb::SBFunction GetFunction();

  lldb::SBBlock GetBlock();

  lldb::SBSymbol GetSymbol();

  lldb::SBLineEntry GetLineEntry();

protected:
  friend class SBAddressRange;
  friend class SBBlock;
  friend class SBBreakpoint;
  friend class SBBreakpointLocation;
  friend class SBFrame;
  friend class SBFunction;
  friend class SBLineEntry;
  friend class SBInstruction;
  friend class SBModule;
  friend class SBSection;
  friend class SBSymbol;
  friend class SBSymbolContext;
  friend class SBTarget;
  friend class SBThread;
  friend class SBThreadPlan;
  friend class SBValue;
  friend class SBQueueItem;

  lldb_private::Address *operator->();

  const lldb_private::Address *operator->() const;

#ifndef SWIG
  friend bool LLDB_API operator==(const SBAddress &lhs, const SBAddress &rhs);
#endif

  lldb_private::Address *get();

  lldb_private::Address &ref();

  const lldb_private::Address &ref() const;

  SBAddress(const lldb_private::Address &address);

  void SetAddress(const lldb_private::Address &address);

private:
  std::unique_ptr<lldb_private::Address> m_opaque_up;
};

#ifndef SWIG
bool LLDB_API operator==(const SBAddress &lhs, const SBAddress &rhs);
#endif

} // namespace lldb

#endif // LLDB_API_SBADDRESS_H
