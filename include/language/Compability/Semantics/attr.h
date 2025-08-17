//===-- language/Compability/Semantics/attr.h --------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_ATTR_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_ATTR_H_

#include "language/Compability/Common/enum-set.h"
#include "language/Compability/Common/idioms.h"
#include <cinttypes>
#include <string>

namespace toolchain {
class raw_ostream;
}

namespace language::Compability::semantics {

// All available attributes.
ENUM_CLASS(Attr, ABSTRACT, ALLOCATABLE, ASYNCHRONOUS, BIND_C, CONTIGUOUS,
    DEFERRED, ELEMENTAL, EXTENDS, EXTERNAL, IMPURE, INTENT_IN, INTENT_INOUT,
    INTENT_OUT, INTRINSIC, MODULE, NON_OVERRIDABLE, NON_RECURSIVE, NOPASS,
    OPTIONAL, PARAMETER, PASS, POINTER, PRIVATE, PROTECTED, PUBLIC, PURE,
    RECURSIVE, SAVE, TARGET, VALUE, VOLATILE)

// Set of attributes
class Attrs : public common::EnumSet<Attr, Attr_enumSize> {
private:
  using enumSetType = common::EnumSet<Attr, Attr_enumSize>;

public:
  using enumSetType::enumSetType;
  Attrs(const enumSetType &attrs) : enumSetType(attrs) {}
  Attrs(enumSetType &&attrs) : enumSetType(std::move(attrs)) {}
  constexpr bool HasAny(const Attrs &x) const { return !(*this & x).none(); }
  constexpr bool HasAll(const Attrs &x) const { return (~*this & x).none(); }
  // Internal error if any of these attributes are not in allowed.
  void CheckValid(const Attrs &allowed) const;

private:
  friend toolchain::raw_ostream &operator<<(toolchain::raw_ostream &, const Attrs &);
};

// Return string representation of attr that matches Fortran source.
std::string AttrToString(Attr attr);

toolchain::raw_ostream &operator<<(toolchain::raw_ostream &o, Attr attr);
toolchain::raw_ostream &operator<<(toolchain::raw_ostream &o, const Attrs &attrs);
} // namespace language::Compability::semantics
#endif // FORTRAN_SEMANTICS_ATTR_H_
