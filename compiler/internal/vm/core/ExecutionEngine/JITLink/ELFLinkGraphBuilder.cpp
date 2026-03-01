//=----------- ELFLinkGraphBuilder.cpp - ELF LinkGraph builder ------------===//
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
//
// Generic ELF LinkGraph building code.
//
//===----------------------------------------------------------------------===//

#include "ELFLinkGraphBuilder.h"

#define DEBUG_TYPE "jitlink"

static const char *DWSecNames[] = {
#define HANDLE_DWARF_SECTION(ENUM_NAME, ELF_NAME, CMDLINE_NAME, OPTION)        \
  ELF_NAME,
#include "vm/core/BinaryFormat/Dwarf.def"
#undef HANDLE_DWARF_SECTION
};

namespace vm::core {
namespace jitlink {

StringRef ELFLinkGraphBuilderBase::CommonSectionName(".common");
ArrayRef<const char *> ELFLinkGraphBuilderBase::DwarfSectionNames = DWSecNames;

ELFLinkGraphBuilderBase::~ELFLinkGraphBuilderBase() = default;

} // end namespace jitlink
} // end namespace vm::core
