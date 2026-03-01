//===--- loongarch.cpp - Generic JITLink loongarch edge kinds, utilities --===//
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
// Generic utilities for graphs representing loongarch objects.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/JITLink/loongarch.h"

#define DEBUG_TYPE "jitlink"

namespace vm::core {
namespace jitlink {
namespace loongarch {

const char NullPointerContent[8] = {0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00};

const uint8_t LA64StubContent[StubEntrySize] = {
    0x14, 0x00, 0x00, 0x1a, // pcalau12i $t8, %page20(imm)
    0x94, 0x02, 0xc0, 0x28, // ld.d $t8, $t8, %pageoff12(imm)
    0x80, 0x02, 0x00, 0x4c  // jr $t8
};

const uint8_t LA32StubContent[StubEntrySize] = {
    0x14, 0x00, 0x00, 0x1c, // pcaddu12i $t8, %pcadd20(imm)
    0x94, 0x02, 0x80, 0x28, // ld.w $t8, $t8, %pcadd12(.Lpcadd_hi)
    0x80, 0x02, 0x00, 0x4c  // jr $t8
};

const char *getEdgeKindName(Edge::Kind K) {
#define KIND_NAME_CASE(K)                                                      \
  case K:                                                                      \
    return #K;

  switch (K) {
    KIND_NAME_CASE(Pointer64)
    KIND_NAME_CASE(Pointer32)
    KIND_NAME_CASE(Delta32)
    KIND_NAME_CASE(NegDelta32)
    KIND_NAME_CASE(Delta64)
    KIND_NAME_CASE(Branch16PCRel)
    KIND_NAME_CASE(Branch21PCRel)
    KIND_NAME_CASE(Branch26PCRel)
    KIND_NAME_CASE(Page20)
    KIND_NAME_CASE(PageOffset12)
    KIND_NAME_CASE(PCAddHi20)
    KIND_NAME_CASE(PCAddLo12)
    KIND_NAME_CASE(RequestGOTAndTransformToPage20)
    KIND_NAME_CASE(RequestGOTAndTransformToPageOffset12)
    KIND_NAME_CASE(RequestGOTAndTransformToPCAddHi20)
    KIND_NAME_CASE(Call30PCRel)
    KIND_NAME_CASE(Call36PCRel)
    KIND_NAME_CASE(Add6)
    KIND_NAME_CASE(Add8)
    KIND_NAME_CASE(Add16)
    KIND_NAME_CASE(Add32)
    KIND_NAME_CASE(Add64)
    KIND_NAME_CASE(AddUleb128)
    KIND_NAME_CASE(Sub6)
    KIND_NAME_CASE(Sub8)
    KIND_NAME_CASE(Sub16)
    KIND_NAME_CASE(Sub32)
    KIND_NAME_CASE(Sub64)
    KIND_NAME_CASE(SubUleb128)
    KIND_NAME_CASE(AlignRelaxable)
  default:
    return getGenericEdgeKindName(K);
  }
#undef KIND_NAME_CASE
}

} // namespace loongarch
} // namespace jitlink
} // namespace vm::core
