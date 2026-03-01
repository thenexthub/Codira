//==- HexagonPatternsV65.td -------------------------------*- tablegen -*-===//
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

multiclass vgathermh<RegisterClass RC> {
  let isCodeGenOnly = 1, isPseudo = 1, mayLoad = 1,
  mayStore = 1, addrMode = BaseImmOffset, accessSize = HalfWordAccess in
  def NAME : CVI_GATHER_TMP_LD_Resource_NoOpcode<(outs ),
                           (ins IntRegs:$_dst_, s4_0Imm:$Ii,
                                IntRegs:$Rt, ModRegs:$Mu, RC:$Vv),
                           ".error \"should not emit\" ",
                           []>;
}

multiclass vgathermw<RegisterClass RC> {
  let isCodeGenOnly = 1, isPseudo = 1, mayLoad = 1,
  mayStore = 1, addrMode = BaseImmOffset, accessSize = WordAccess in
  def NAME : CVI_GATHER_TMP_LD_Resource_NoOpcode<(outs ),
                           (ins IntRegs:$_dst_, s4_0Imm:$Ii,
                                IntRegs:$Rt, ModRegs:$Mu, RC:$Vv),
                           ".error \"should not emit\" ",
                           []>;
}

multiclass vgathermhw<RegisterClass RC> {
  let isCodeGenOnly = 1, isPseudo = 1, mayLoad = 1,
   mayStore = 1, addrMode = BaseImmOffset, accessSize = HalfWordAccess in
  def NAME : CVI_GATHER_TMP_LD_Resource_NoOpcode<(outs ),
                           (ins IntRegs:$_dst_, s4_0Imm:$Ii,
                                IntRegs:$Rt, ModRegs:$Mu, RC:$Vv),
                           ".error \"should not emit\" ",
                           []>;
}

defm V6_vgathermh_pseudo  : vgathermh<HvxVR>;
defm V6_vgathermw_pseudo  : vgathermw<HvxVR>;
defm V6_vgathermhw_pseudo  : vgathermhw<HvxWR>;


multiclass vgather_scatter_mh<RegisterClass RC> {
  let isCodeGenOnly = 1, isPseudo = 1, mayLoad = 1,
  mayStore = 1, addrMode = BaseImmOffset, accessSize = HalfWordAccess in
  def NAME : CVI_GATHER_TMP_LD_Resource_NoOpcode<(outs ),
                           (ins IntRegs:$_dst_, s4_0Imm:$Ii,
                                IntRegs:$Rt, ModRegs:$Mu, RC:$Vv),
                           ".error \"should not emit\" ",
                           []>;
}

defm V6_vgather_vscatter_mh_pseudo  : vgather_scatter_mh<HvxVR>;

multiclass vgathermhq<RegisterClass RC1, RegisterClass RC2> {
  let isCodeGenOnly = 1, isPseudo = 1, mayLoad = 1,
  mayStore = 1, addrMode = BaseImmOffset, accessSize = HalfWordAccess in
  def NAME : CVI_GATHER_TMP_LD_Resource_NoOpcode<(outs ),
                           (ins IntRegs:$_dst_, s4_0Imm:$Ii,
                                RC2:$Vq, IntRegs:$Rt, ModRegs:$Mu,
                                RC1:$Vv),
                           ".error \"should not emit\" ",
                           []>;
}

multiclass vgathermwq<RegisterClass RC1, RegisterClass RC2> {
  let isCodeGenOnly = 1, isPseudo = 1, mayLoad = 1,
  mayStore = 1, addrMode = BaseImmOffset, accessSize = WordAccess in
  def NAME : CVI_GATHER_TMP_LD_Resource_NoOpcode<(outs ),
                           (ins IntRegs:$_dst_, s4_0Imm:$Ii,
                                RC2:$Vq, IntRegs:$Rt, ModRegs:$Mu,
                                RC1:$Vv),
                           ".error \"should not emit\" ",
                           []>;
}

multiclass vgathermhwq<RegisterClass RC1, RegisterClass RC2> {
  let isCodeGenOnly = 1, isPseudo = 1, mayLoad = 1,
  mayStore = 1, addrMode = BaseImmOffset, accessSize = HalfWordAccess  in
  def NAME : CVI_GATHER_TMP_LD_Resource_NoOpcode<(outs ),
                           (ins IntRegs:$_dst_, s4_0Imm:$Ii,
                              RC2:$Vq, IntRegs:$Rt, ModRegs:$Mu,
                              RC1:$Vv),
                           ".error \"should not emit\" ",
                           []>;
}

defm V6_vgathermhq_pseudo  : vgathermhq<HvxVR, HvxQR>;
defm V6_vgathermwq_pseudo  : vgathermwq<HvxVR, HvxQR>;
defm V6_vgathermhwq_pseudo  : vgathermhwq<HvxWR, HvxQR>;
