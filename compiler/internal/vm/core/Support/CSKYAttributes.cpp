//===-- CSKYAttributes.cpp - CSKY Attributes ------------------------------===//
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

#include "vm/core/Support/CSKYAttributes.h"

using namespace vm::core;
using namespace vm::core::CSKYAttrs;

static const TagNameItem tagData[] = {
    {CSKY_ARCH_NAME, "Tag_CSKY_ARCH_NAME"},
    {CSKY_CPU_NAME, "Tag_CSKY_CPU_NAME"},
    {CSKY_CPU_NAME, "Tag_CSKY_CPU_NAME"},
    {CSKY_ISA_FLAGS, "Tag_CSKY_ISA_FLAGS"},
    {CSKY_ISA_EXT_FLAGS, "Tag_CSKY_ISA_EXT_FLAGS"},
    {CSKY_DSP_VERSION, "Tag_CSKY_DSP_VERSION"},
    {CSKY_VDSP_VERSION, "Tag_CSKY_VDSP_VERSION"},
    {CSKY_FPU_VERSION, "Tag_CSKY_FPU_VERSION"},
    {CSKY_FPU_ABI, "Tag_CSKY_FPU_ABI"},
    {CSKY_FPU_ROUNDING, "Tag_CSKY_FPU_ROUNDING"},
    {CSKY_FPU_DENORMAL, "Tag_CSKY_FPU_DENORMAL"},
    {CSKY_FPU_EXCEPTION, "Tag_CSKY_FPU_EXCEPTION"},
    {CSKY_FPU_NUMBER_MODULE, "Tag_CSKY_FPU_NUMBER_MODULE"},
    {CSKY_FPU_HARDFP, "Tag_CSKY_FPU_HARDFP"}};

constexpr TagNameMap CSKYAttributeTags{tagData};
const TagNameMap &toolchain::CSKYAttrs::getCSKYAttributeTags() {
  return CSKYAttributeTags;
}
