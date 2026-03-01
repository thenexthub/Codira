//===---------------------- X86Combiner.cpp ------------------------------===//
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

include "toolchain/Target/GlobalISel/Combine.td"

// all_x86combines is based on generic all_combines, currently x86 gisel does not
// have vector support and few open issue to address which resulted in failure with
// combines. We will introduce more combines gradually.

def all_x86combines : GICombineGroup<[identity_combines, reassocs, 
    const_combines, undef_combines, trivial_combines, simplify_add_to_sub]>;

def X86PreLegalizerCombiner : GICombiner<"X86PreLegalizerCombinerImpl", [all_x86combines]> {
    let CombineAllMethodName = "tryCombineAllImpl";
}
