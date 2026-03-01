//===-- MCELFObjectTargetWriter.cpp - ELF Target Writer Subclass ----------===//
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

#include "vm/core/MC/MCELFObjectWriter.h"

using namespace vm::core;

MCELFObjectTargetWriter::MCELFObjectTargetWriter(bool Is64Bit_, uint8_t OSABI_,
                                                 uint16_t EMachine_,
                                                 bool HasRelocationAddend_,
                                                 uint8_t ABIVersion_)
    : OSABI(OSABI_), ABIVersion(ABIVersion_), EMachine(EMachine_),
      HasRelocationAddend(HasRelocationAddend_), Is64Bit(Is64Bit_) {}

void MCELFObjectTargetWriter::sortRelocs(
    std::vector<ELFRelocationEntry> &Relocs) {}
