//===--- HexagonIICHVX.td -------------------------------------------------===//
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

def CVI_GATHER_PSEUDO : InstrItinClass;
def CVI_VA            : InstrItinClass;

class HVXItin {
  list<InstrItinData> HVXItin_list = [
    InstrItinData<CVI_VA,
      [InstrStage<1, [SLOT0,SLOT1,SLOT2,SLOT3], 0>,
       InstrStage<1, [CVI_XLANE,CVI_SHIFT, CVI_MPY0, CVI_MPY1]>],
      [9, 7, 7, 7], [HVX_FWD, HVX_FWD, HVX_FWD]>,

    // Used by gather pseudo-instructions which are expanded into V6_vgather*
    // and V6_vS32b_new_ai. Even though these instructions use CVI_LD resource,
    // it's not included below to avoid having more than 4 InstrStages and
    // thus changing 'MaxResTerms' to 5. Instead, both SLOT0 and SLOT1 are
    // used, which should be sufficient.
    InstrItinData <CVI_GATHER_PSEUDO,
      [InstrStage<1, [SLOT0], 0>,
       InstrStage<1, [SLOT1], 0>,
       InstrStage<1, [CVI_ST], 0>,
       InstrStage<1, [CVI_MPY01, CVI_XLSHF]>]>];
}
