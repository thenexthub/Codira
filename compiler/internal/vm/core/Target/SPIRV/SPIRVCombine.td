//=- SPIRVCombine.td - Define SPIRV Combine Rules -------------*-tablegen -*-=//
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

include "toolchain/Target/GlobalISel/Combine.td"


def vector_length_sub_to_distance_lowering : GICombineRule <
  (defs root:$root),
  (match (wip_match_opcode G_INTRINSIC):$root,
          [{ return Helper.matchLengthToDistance(*${root}); }]),
  (apply [{ Helper.applySPIRVDistance(*${root}); }])
>;

def vector_select_to_faceforward_lowering : GICombineRule <
  (defs root:$root),
  (match (wip_match_opcode G_SELECT):$root,
          [{ return Helper.matchSelectToFaceForward(*${root}); }]),
  (apply [{ Helper.applySPIRVFaceForward(*${root}); }])
>;

def matrix_transpose_lowering
    : GICombineRule<(defs root:$root),
                    (match (wip_match_opcode G_INTRINSIC):$root,
                        [{ return Helper.matchMatrixTranspose(*${root}); }]),
                    (apply [{ Helper.applyMatrixTranspose(*${root}); }])>;

def matrix_multiply_lowering
    : GICombineRule<(defs root:$root),
                    (match (wip_match_opcode G_INTRINSIC):$root,
                        [{ return Helper.matchMatrixMultiply(*${root}); }]),
                    (apply [{ Helper.applyMatrixMultiply(*${root}); }])>;

def SPIRVPreLegalizerCombiner
    : GICombiner<"SPIRVPreLegalizerCombinerImpl",
                 [vector_length_sub_to_distance_lowering,
                  vector_select_to_faceforward_lowering,
                  matrix_transpose_lowering, matrix_multiply_lowering]> {
  let CombineAllMethodName = "tryCombineAllImpl";
}
