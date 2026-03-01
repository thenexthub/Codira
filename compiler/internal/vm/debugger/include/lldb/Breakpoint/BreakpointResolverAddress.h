//===-- BreakpointResolverAddress.h -----------------------------*- C++ -*-===//
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

#ifndef LLDB_BREAKPOINT_BREAKPOINTRESOLVERADDRESS_H
#define LLDB_BREAKPOINT_BREAKPOINTRESOLVERADDRESS_H

#include "lldb/Breakpoint/BreakpointResolver.h"
#include "lldb/Core/ModuleSpec.h"

namespace lldb_private {

/// \class BreakpointResolverAddress BreakpointResolverAddress.h
/// "lldb/Breakpoint/BreakpointResolverAddress.h" This class sets breakpoints
/// on a given Address.  This breakpoint only takes once, and then it won't
/// attempt to reset itself.

class BreakpointResolverAddress : public BreakpointResolver {
public:
  BreakpointResolverAddress(const lldb::BreakpointSP &bkpt,
                            const Address &addr);

  BreakpointResolverAddress(const lldb::BreakpointSP &bkpt,
                            const Address &addr,
                            const FileSpec &module_spec);

  ~BreakpointResolverAddress() override = default;

  static lldb::BreakpointResolverSP
  CreateFromStructuredData(const StructuredData::Dictionary &options_dict,
                           Status &error);

  StructuredData::ObjectSP SerializeToStructuredData() override;

  void ResolveBreakpoint(SearchFilter &filter) override;

  void ResolveBreakpointInModules(SearchFilter &filter,
                                  ModuleList &modules) override;

  Searcher::CallbackReturn SearchCallback(SearchFilter &filter,
                                          SymbolContext &context,
                                          Address *addr) override;

  lldb::SearchDepth GetDepth() override;

  void GetDescription(Stream *s) override;

  void Dump(Stream *s) const override;

  /// Methods for support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const BreakpointResolverAddress *) { return true; }
  static inline bool classof(const BreakpointResolver *V) {
    return V->getResolverID() == BreakpointResolver::AddressResolver;
  }

  lldb::BreakpointResolverSP
  CopyForBreakpoint(lldb::BreakpointSP &breakpoint) override;

protected:
  Address m_addr;               // The address - may be Section Offset or
                                // may be just an offset
  lldb::addr_t m_resolved_addr; // The current value of the resolved load
                                // address for this breakpoint,
  FileSpec m_module_filespec;   // If this filespec is Valid, and m_addr is an
                                // offset, then it will be converted
  // to a Section+Offset address in this module, whenever that module gets
  // around to being loaded.
private:
  BreakpointResolverAddress(const BreakpointResolverAddress &) = delete;
  const BreakpointResolverAddress &
  operator=(const BreakpointResolverAddress &) = delete;
};

} // namespace lldb_private

#endif // LLDB_BREAKPOINT_BREAKPOINTRESOLVERADDRESS_H
