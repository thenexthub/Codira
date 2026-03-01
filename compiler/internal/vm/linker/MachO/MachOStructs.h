//===- MachOStructs.h -------------------------------------------*- C++ -*-===//
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
// This file defines structures used in the MachO object file format. Note that
// unlike llvm/BinaryFormat/MachO.h, the structs here are defined in terms of
// endian- and alignment-compatibility wrappers.
//
//===----------------------------------------------------------------------===//

#ifndef LLD_MACHO_MACHO_STRUCTS_H
#define LLD_MACHO_MACHO_STRUCTS_H

#include "llvm/Support/Endian.h"

namespace lld::structs {

struct nlist_64 {
  llvm::support::ulittle32_t n_strx;
  uint8_t n_type;
  uint8_t n_sect;
  llvm::support::ulittle16_t n_desc;
  llvm::support::ulittle64_t n_value;
};

struct nlist {
  llvm::support::ulittle32_t n_strx;
  uint8_t n_type;
  uint8_t n_sect;
  llvm::support::ulittle16_t n_desc;
  llvm::support::ulittle32_t n_value;
};

struct entry_point_command {
  llvm::support::ulittle32_t cmd;
  llvm::support::ulittle32_t cmdsize;
  llvm::support::ulittle64_t entryoff;
  llvm::support::ulittle64_t stacksize;
};

} // namespace lld::structs

#endif
