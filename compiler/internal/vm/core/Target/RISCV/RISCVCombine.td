//=- RISCVCombine.td - Define RISC-V Combine Rules -----------*- tablegen -*-=//
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
//
//===----------------------------------------------------------------------===//

include "toolchain/Target/GlobalISel/Combine.td"

def RISCVPreLegalizerCombiner: GICombiner<
  "RISCVPreLegalizerCombinerImpl", [all_combines]> {
}

def RISCVO0PreLegalizerCombiner: GICombiner<
  "RISCVO0PreLegalizerCombinerImpl", [optnone_combines]> {
}

// Rule: fold store (fp +0.0) -> store (int zero [XLEN])
def fp_zero_store_matchdata : GIDefMatchData<"Register">;
def fold_fp_zero_store : GICombineRule<
  (defs root:$root, fp_zero_store_matchdata:$matchinfo),
  (match (G_STORE $src, $addr):$root,
         [{ return matchFoldFPZeroStore(*${root}, MRI, STI, ${matchinfo}); }]),
  (apply [{ applyFoldFPZeroStore(*${root}, MRI, B, STI, ${matchinfo}); }])>;

// Post-legalization combines which are primarily optimizations.
// TODO: Add more combines.
def RISCVPostLegalizerCombiner
    : GICombiner<"RISCVPostLegalizerCombinerImpl",
                 [sub_to_add, combines_for_extload, redundant_and,
                  identity_combines, shift_immed_chain,
                  commute_constant_to_rhs, simplify_neg_minmax,
                  fold_fp_zero_store]> {
}
