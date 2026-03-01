//=-HexagonScheduleV65.td - HexagonV65 Scheduling Definitions *- tablegen -*-=//
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
// ScalarItin and HVXItin contain some old itineraries
// still used by a handful of instructions. Hopefully, we will be able
// to get rid of them soon.

def HexagonV65ItinList : DepScalarItinV65, ScalarItin,
                         DepHVXItinV65, HVXItin, PseudoItin {
  list<InstrItinData> ItinList =
    !listconcat(DepScalarItinV65_list, ScalarItin_list,
                DepHVXItinV65_list, HVXItin_list, PseudoItin_list);
}

def HexagonItinerariesV65 :
      ProcessorItineraries<[SLOT0, SLOT1, SLOT2, SLOT3, SLOT_ENDLOOP,
                            CVI_ST, CVI_XLANE, CVI_SHIFT, CVI_MPY0, CVI_MPY1,
                            CVI_LD, CVI_XLSHF, CVI_MPY01, CVI_ALL,
                            CVI_ALL_NOMEM, CVI_ZW],
                            [Hex_FWD, HVX_FWD],
                            HexagonV65ItinList.ItinList>;

def HexagonModelV65 : SchedMachineModel {
  // Max issue per cycle == bundle width.
  let IssueWidth = 4;
  let Itineraries = HexagonItinerariesV65;
  let LoadLatency = 1;
  let CompleteModel = 0;
}

//===----------------------------------------------------------------------===//
// Hexagon V65 Resource Definitions -
//===----------------------------------------------------------------------===//
