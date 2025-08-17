//===-- language/Compability/Semantics/runtime-type-info.h -------------*- C++ -*-===//
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

// BuildRuntimeDerivedTypeTables() translates the scopes of derived types
// and parameterized derived type instantiations into the type descriptions
// defined in module/__fortran_type_info.f90, packaging these descriptions
// as static initializers for compiler-created objects.

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_RUNTIME_TYPE_INFO_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_RUNTIME_TYPE_INFO_H_

#include "language/Compability/Common/reference.h"
#include "language/Compability/Semantics/symbol.h"
#include <map>
#include <set>
#include <string>
#include <vector>

namespace toolchain {
class raw_ostream;
}

namespace language::Compability::semantics {

struct RuntimeDerivedTypeTables {
  Scope *schemata{nullptr};
  std::set<std::string> names;
};

RuntimeDerivedTypeTables BuildRuntimeDerivedTypeTables(SemanticsContext &);

/// Name of the builtin module that defines builtin derived types meant
/// to describe other derived types at runtime in flang descriptor.
constexpr char typeInfoBuiltinModule[]{"__fortran_type_info"};

/// Name of the builtin derived type in __fortran_type_inf that is used for
/// derived type descriptors.
constexpr char typeDescriptorTypeName[]{"derivedtype"};

/// Name of the bindings descriptor component in the DerivedType type of the
/// __Fortran_type_info module
constexpr char bindingDescCompName[]{"binding"};

/// Name of the __builtin_c_funptr component in the Binding type  of the
/// __Fortran_type_info module
constexpr char procCompName[]{"proc"};

SymbolVector CollectBindings(const Scope &dtScope);

enum NonTbpDefinedIoFlags {
  IsDtvArgPolymorphic = 1 << 0,
  DefinedIoInteger8 = 1 << 1,
};

struct NonTbpDefinedIo {
  const Symbol *subroutine;
  common::DefinedIo definedIo;
  std::uint8_t flags;
};

std::multimap<const Symbol *, NonTbpDefinedIo>
CollectNonTbpDefinedIoGenericInterfaces(
    const Scope &, bool useRuntimeTypeInfoEntries);

bool ShouldIgnoreRuntimeTypeInfoNonTbpGenericInterfaces(
    const Scope &, const DerivedTypeSpec *);
bool ShouldIgnoreRuntimeTypeInfoNonTbpGenericInterfaces(
    const Scope &, const DeclTypeSpec *);
bool ShouldIgnoreRuntimeTypeInfoNonTbpGenericInterfaces(
    const Scope &, const Symbol *);

} // namespace language::Compability::semantics
#endif // FORTRAN_SEMANTICS_RUNTIME_TYPE_INFO_H_
