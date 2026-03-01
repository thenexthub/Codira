//===- EnumTables.cpp - Enum to string conversion tables --------*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/Native/EnumTables.h"
#include "vm/core/DebugInfo/PDB/Native/RawConstants.h"
#include "vm/core/Support/ScopedPrinter.h"

using namespace vm::core;
using namespace vm::core::pdb;

#define PDB_ENUM_CLASS_ENT(enum_class, enum)                                   \
  { #enum, std::underlying_type_t<enum_class>(enum_class::enum) }

#define PDB_ENUM_ENT(ns, enum)                                                 \
  { #enum, ns::enum }

static const EnumEntry<uint16_t> OMFSegMapDescFlagNames[] = {
    PDB_ENUM_CLASS_ENT(OMFSegDescFlags, Read),
    PDB_ENUM_CLASS_ENT(OMFSegDescFlags, Write),
    PDB_ENUM_CLASS_ENT(OMFSegDescFlags, Execute),
    PDB_ENUM_CLASS_ENT(OMFSegDescFlags, AddressIs32Bit),
    PDB_ENUM_CLASS_ENT(OMFSegDescFlags, IsSelector),
    PDB_ENUM_CLASS_ENT(OMFSegDescFlags, IsAbsoluteAddress),
    PDB_ENUM_CLASS_ENT(OMFSegDescFlags, IsGroup),
};

namespace vm::core {
namespace pdb {
ArrayRef<EnumEntry<uint16_t>> getOMFSegMapDescFlagNames() {
  return ArrayRef(OMFSegMapDescFlagNames);
}
}
}
