//==- HexagonInstrFormatsV65.td - Hexagon Instruction Formats -*- tablegen -==//
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
// This file describes the Hexagon V60 instruction classes in TableGen format.
//
//===----------------------------------------------------------------------===//

//----------------------------------------------------------------------------//
//                         Hexagon Instruction Flags +
//
//                        *** Must match BaseInfo.h ***
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//                         Instruction Classes Definitions +
//----------------------------------------------------------------------------//

class CVI_GATHER_TMP_LD_Resource_NoOpcode<dag outs, dag ins, string asmstr,
                        list<dag> pattern = [], string cstr = "",
                        InstrItinClass itin = CVI_GATHER_PSEUDO>
   : InstHexagon<outs, ins, asmstr, pattern, cstr, itin, TypeCVI_GATHER>;
