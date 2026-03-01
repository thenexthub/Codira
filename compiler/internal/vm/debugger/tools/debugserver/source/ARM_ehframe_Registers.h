//===-- ARM_ehframe_Registers.h -------------------------------------*- C++
//-*-===//
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

#ifndef utility_ARM_ehframe_Registers_h_
#define utility_ARM_ehframe_Registers_h_

enum {
  ehframe_r0 = 0,
  ehframe_r1,
  ehframe_r2,
  ehframe_r3,
  ehframe_r4,
  ehframe_r5,
  ehframe_r6,
  ehframe_r7,
  ehframe_r8,
  ehframe_r9,
  ehframe_r10,
  ehframe_r11,
  ehframe_r12,
  ehframe_sp,
  ehframe_lr,
  ehframe_pc,
  ehframe_cpsr
};

#endif // utility_ARM_ehframe_Registers_h_
