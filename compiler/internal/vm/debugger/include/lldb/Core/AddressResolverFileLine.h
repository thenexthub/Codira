//===-- AddressResolverFileLine.h -------------------------------*- C++ -*-===//
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

#ifndef LLDB_CORE_ADDRESSRESOLVERFILELINE_H
#define LLDB_CORE_ADDRESSRESOLVERFILELINE_H

#include "lldb/Core/AddressResolver.h"
#include "lldb/Core/SearchFilter.h"
#include "lldb/Core/SourceLocationSpec.h"
#include "lldb/lldb-defines.h"

#include <cstdint>

namespace lldb_private {
class Address;
class Stream;
class SymbolContext;

/// \class AddressResolverFileLine AddressResolverFileLine.h
/// "lldb/Core/AddressResolverFileLine.h" This class finds address for source
/// file and line.  Optionally, it will look for inlined instances of the file
/// and line specification.

class AddressResolverFileLine : public AddressResolver {
public:
  AddressResolverFileLine(SourceLocationSpec location_spec);

  ~AddressResolverFileLine() override;

  Searcher::CallbackReturn SearchCallback(SearchFilter &filter,
                                          SymbolContext &context,
                                          Address *addr) override;

  lldb::SearchDepth GetDepth() override;

  void GetDescription(Stream *s) override;

protected:
  SourceLocationSpec m_src_location_spec;

private:
  AddressResolverFileLine(const AddressResolverFileLine &) = delete;
  const AddressResolverFileLine &
  operator=(const AddressResolverFileLine &) = delete;
};

} // namespace lldb_private

#endif // LLDB_CORE_ADDRESSRESOLVERFILELINE_H
