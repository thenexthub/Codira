//===-- language/Compability-rt/runtime/namelist.h ---------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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

// Defines the data structure used for NAMELIST I/O

#ifndef FLANG_RT_RUNTIME_NAMELIST_H_
#define FLANG_RT_RUNTIME_NAMELIST_H_

#include "non-tbp-dio.h"
#include "language/Compability/Common/api-attrs.h"

#include <cstddef>

namespace language::Compability::runtime {
class Descriptor;
class IoStatementState;
} // namespace language::Compability::runtime

namespace language::Compability::runtime::io {

// A NAMELIST group is a named ordered collection of distinct variable names.
// It is packaged by lowering into an instance of this class.
// If all the items are variables with fixed addresses, the NAMELIST group
// description can be in a read-only section.
class NamelistGroup {
public:
  struct Item {
    const char *name; // NUL-terminated lower-case
    const Descriptor &descriptor;
  };
  const char *groupName{nullptr}; // NUL-terminated lower-case
  std::size_t items{0};
  const Item *item{nullptr}; // in original declaration order

  // When the uses of a namelist group appear in scopes with distinct sets
  // of non-type-bound defined formatted I/O interfaces, they require the
  // use of distinct NamelistGroups pointing to distinct NonTbpDefinedIoTables.
  // Multiple NamelistGroup instances may share a NonTbpDefinedIoTable..
  const NonTbpDefinedIoTable *nonTbpDefinedIo{nullptr};
};

// Look ahead on input for a '/' or an identifier followed by a '=', '(', or '%'
// character; for use in disambiguating a name-like value (e.g. F or T) from a
// NAMELIST group item name and for coping with short arrays.  Always false
// when not reading a NAMELIST.
RT_API_ATTRS bool IsNamelistNameOrSlash(IoStatementState &);

} // namespace language::Compability::runtime::io
#endif // FLANG_RT_RUNTIME_NAMELIST_H_
