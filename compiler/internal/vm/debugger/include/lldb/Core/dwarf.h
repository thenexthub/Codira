//===-- dwarf.h -------------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_CORE_DWARF_H
#define LLDB_CORE_DWARF_H

#include <cstdint>

// Get the DWARF constant definitions from llvm
#include "llvm/BinaryFormat/Dwarf.h"

typedef llvm::dwarf::Attribute dw_attr_t;
typedef llvm::dwarf::Form dw_form_t;
typedef llvm::dwarf::Tag dw_tag_t;
typedef uint64_t dw_addr_t; // Dwarf address define that must be big enough for
                            // any addresses in the compile units that get
                            // parsed

typedef uint64_t dw_offset_t; // Dwarf Debug Information Entry offset for any
                              // offset into the file

/* Constants */
#define DW_DIE_OFFSET_MAX_BITSIZE 40
#define DW_INVALID_OFFSET (((uint64_t)1u << DW_DIE_OFFSET_MAX_BITSIZE) - 1)
#define DW_INVALID_INDEX 0xFFFFFFFFul

// #define DW_ADDR_none 0x0

#define DW_EH_PE_MASK_ENCODING 0x0F

#endif // LLDB_CORE_DWARF_H
