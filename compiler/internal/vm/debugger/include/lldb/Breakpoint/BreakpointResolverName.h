//===-- BreakpointResolverName.h --------------------------------*- C++ -*-===//
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

#ifndef LLDB_BREAKPOINT_BREAKPOINTRESOLVERNAME_H
#define LLDB_BREAKPOINT_BREAKPOINTRESOLVERNAME_H

#include <string>
#include <vector>

#include "lldb/Breakpoint/BreakpointResolver.h"
#include "lldb/Core/Module.h"

namespace lldb_private {

/// \class BreakpointResolverName BreakpointResolverName.h
/// "lldb/Breakpoint/BreakpointResolverName.h" This class sets breakpoints on
/// a given function name, either by exact match or by regular expression.

class BreakpointResolverName : public BreakpointResolver {
public:
  BreakpointResolverName(const lldb::BreakpointSP &bkpt, const char *name,
                         lldb::FunctionNameType name_type_mask,
                         lldb::LanguageType language,
                         Breakpoint::MatchType type, lldb::addr_t offset,
                         bool offset_is_insn_count, bool skip_prologue);

  // This one takes an array of names.  It is always MatchType = Exact.
  BreakpointResolverName(const lldb::BreakpointSP &bkpt, const char *names[],
                         size_t num_names,
                         lldb::FunctionNameType name_type_mask,
                         lldb::LanguageType language, lldb::addr_t offset,
                         bool skip_prologue);

  // This one takes a C++ array of names.  It is always MatchType = Exact.
  BreakpointResolverName(const lldb::BreakpointSP &bkpt,
                         const std::vector<std::string> &names,
                         lldb::FunctionNameType name_type_mask,
                         lldb::LanguageType language, lldb::addr_t offset,
                         bool skip_prologue);

  // Creates a function breakpoint by regular expression.  Takes over control
  // of the lifespan of func_regex.
  BreakpointResolverName(const lldb::BreakpointSP &bkpt,
                         RegularExpression func_regex,
                         lldb::LanguageType language, lldb::addr_t offset,
                         bool skip_prologue);

  static lldb::BreakpointResolverSP
  CreateFromStructuredData(const StructuredData::Dictionary &data_dict,
                           Status &error);

  StructuredData::ObjectSP SerializeToStructuredData() override;

  ~BreakpointResolverName() override = default;

  Searcher::CallbackReturn SearchCallback(SearchFilter &filter,
                                          SymbolContext &context,
                                          Address *addr) override;

  lldb::SearchDepth GetDepth() override;

  void GetDescription(Stream *s) override;

  void Dump(Stream *s) const override;

  /// Methods for support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const BreakpointResolverName *) { return true; }
  static inline bool classof(const BreakpointResolver *V) {
    return V->getResolverID() == BreakpointResolver::NameResolver;
  }

  lldb::BreakpointResolverSP
  CopyForBreakpoint(lldb::BreakpointSP &breakpoint) override;

protected:
  BreakpointResolverName(const BreakpointResolverName &rhs);

  std::vector<Module::LookupInfo> m_lookups;
  RegularExpression m_regex;
  Breakpoint::MatchType m_match_type;
  lldb::LanguageType m_language;
  bool m_skip_prologue;

  void AddNameLookup(ConstString name,
                     lldb::FunctionNameType name_type_mask);
};

} // namespace lldb_private

#endif // LLDB_BREAKPOINT_BREAKPOINTRESOLVERNAME_H
