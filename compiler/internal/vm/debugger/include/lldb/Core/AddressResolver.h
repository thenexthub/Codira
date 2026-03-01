//===-- AddressResolver.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_CORE_ADDRESSRESOLVER_H
#define LLDB_CORE_ADDRESSRESOLVER_H

#include "lldb/Core/AddressRange.h"
#include "lldb/Core/SearchFilter.h"
#include "lldb/lldb-defines.h"

#include <cstddef>
#include <vector>

namespace lldb_private {
class ModuleList;
class Stream;

/// \class AddressResolver AddressResolver.h "lldb/Core/AddressResolver.h"
/// This class works with SearchFilter to resolve function names and source
/// file locations to their concrete addresses.

/// General Outline:
/// The AddressResolver is a Searcher.  In that protocol, the SearchFilter
/// asks the question "At what depth of the symbol context descent do you want
/// your callback to get called?" of the filter.  The resolver answers this
/// question (in the GetDepth method) and provides the resolution callback.

class AddressResolver : public Searcher {
public:
  enum MatchType { Exact, Regexp, Glob };

  AddressResolver();

  ~AddressResolver() override;

  virtual void ResolveAddress(SearchFilter &filter);

  virtual void ResolveAddressInModules(SearchFilter &filter,
                                       ModuleList &modules);

  void GetDescription(Stream *s) override = 0;

  std::vector<AddressRange> &GetAddressRanges();

  size_t GetNumberOfAddresses();

  AddressRange &GetAddressRangeAtIndex(size_t idx);

protected:
  std::vector<AddressRange> m_address_ranges;

private:
  AddressResolver(const AddressResolver &) = delete;
  const AddressResolver &operator=(const AddressResolver &) = delete;
};

} // namespace lldb_private

#endif // LLDB_CORE_ADDRESSRESOLVER_H
