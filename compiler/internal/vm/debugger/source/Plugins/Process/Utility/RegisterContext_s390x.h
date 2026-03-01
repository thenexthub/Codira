//===-- RegisterContext_s390x.h ---------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXT_S390X_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXT_S390X_H

// SystemZ ehframe, dwarf regnums

// EHFrame and DWARF Register numbers (eRegisterKindEHFrame &
// eRegisterKindDWARF)
enum {
  // General Purpose Registers
  dwarf_r0_s390x = 0,
  dwarf_r1_s390x,
  dwarf_r2_s390x,
  dwarf_r3_s390x,
  dwarf_r4_s390x,
  dwarf_r5_s390x,
  dwarf_r6_s390x,
  dwarf_r7_s390x,
  dwarf_r8_s390x,
  dwarf_r9_s390x,
  dwarf_r10_s390x,
  dwarf_r11_s390x,
  dwarf_r12_s390x,
  dwarf_r13_s390x,
  dwarf_r14_s390x,
  dwarf_r15_s390x,
  // Floating Point Registers / Vector Registers 0-15
  dwarf_f0_s390x = 16,
  dwarf_f2_s390x,
  dwarf_f4_s390x,
  dwarf_f6_s390x,
  dwarf_f1_s390x,
  dwarf_f3_s390x,
  dwarf_f5_s390x,
  dwarf_f7_s390x,
  dwarf_f8_s390x,
  dwarf_f10_s390x,
  dwarf_f12_s390x,
  dwarf_f14_s390x,
  dwarf_f9_s390x,
  dwarf_f11_s390x,
  dwarf_f13_s390x,
  dwarf_f15_s390x,
  // Access Registers
  dwarf_acr0_s390x = 48,
  dwarf_acr1_s390x,
  dwarf_acr2_s390x,
  dwarf_acr3_s390x,
  dwarf_acr4_s390x,
  dwarf_acr5_s390x,
  dwarf_acr6_s390x,
  dwarf_acr7_s390x,
  dwarf_acr8_s390x,
  dwarf_acr9_s390x,
  dwarf_acr10_s390x,
  dwarf_acr11_s390x,
  dwarf_acr12_s390x,
  dwarf_acr13_s390x,
  dwarf_acr14_s390x,
  dwarf_acr15_s390x,
  // Program Status Word
  dwarf_pswm_s390x = 64,
  dwarf_pswa_s390x,
  // Vector Registers 16-31
  dwarf_v16_s390x = 68,
  dwarf_v18_s390x,
  dwarf_v20_s390x,
  dwarf_v22_s390x,
  dwarf_v17_s390x,
  dwarf_v19_s390x,
  dwarf_v21_s390x,
  dwarf_v23_s390x,
  dwarf_v24_s390x,
  dwarf_v26_s390x,
  dwarf_v28_s390x,
  dwarf_v30_s390x,
  dwarf_v25_s390x,
  dwarf_v27_s390x,
  dwarf_v29_s390x,
  dwarf_v31_s390x,
};

#endif
