//===-- BreakpointResolverFileLine.h ----------------------------*- C++ -*-===//
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

#ifndef LLDB_BREAKPOINT_BREAKPOINTRESOLVERFILELINE_H
#define LLDB_BREAKPOINT_BREAKPOINTRESOLVERFILELINE_H

#include "lldb/Breakpoint/BreakpointResolver.h"
#include "lldb/Core/SourceLocationSpec.h"
#include <optional>

namespace lldb_private {

/// \class BreakpointResolverFileLine BreakpointResolverFileLine.h
/// "lldb/Breakpoint/BreakpointResolverFileLine.h" This class sets breakpoints
/// by file and line.  Optionally, it will look for inlined instances of the
/// file and line specification.

class BreakpointResolverFileLine : public BreakpointResolver {
public:
  BreakpointResolverFileLine(
      const lldb::BreakpointSP &bkpt, lldb::addr_t offset, bool skip_prologue,
      const SourceLocationSpec &location_spec,
      std::optional<llvm::StringRef> removed_prefix_opt = std::nullopt);

  static lldb::BreakpointResolverSP
  CreateFromStructuredData(const StructuredData::Dictionary &data_dict,
                           Status &error);

  StructuredData::ObjectSP SerializeToStructuredData() override;

  ~BreakpointResolverFileLine() override = default;

  Searcher::CallbackReturn SearchCallback(SearchFilter &filter,
                                          SymbolContext &context,
                                          Address *addr) override;

  lldb::SearchDepth GetDepth() override;

  void GetDescription(Stream *s) override;

  void Dump(Stream *s) const override;

  /// Methods for support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const BreakpointResolverFileLine *) {
    return true;
  }
  static inline bool classof(const BreakpointResolver *V) {
    return V->getResolverID() == BreakpointResolver::FileLineResolver;
  }

  lldb::BreakpointResolverSP
  CopyForBreakpoint(lldb::BreakpointSP &breakpoint) override;

protected:
  void FilterContexts(SymbolContextList &sc_list);
  void DeduceSourceMapping(const SymbolContextList &sc_list);

  friend class Breakpoint;
  SourceLocationSpec m_location_spec;
  bool m_skip_prologue;
  // Any previously removed file path prefix by reverse source mapping.
  // This is used to auto deduce source map if needed.
  std::optional<llvm::StringRef> m_removed_prefix_opt;

private:
  BreakpointResolverFileLine(const BreakpointResolverFileLine &) = delete;
  const BreakpointResolverFileLine &
  operator=(const BreakpointResolverFileLine &) = delete;
};

} // namespace lldb_private

#endif // LLDB_BREAKPOINT_BREAKPOINTRESOLVERFILELINE_H
