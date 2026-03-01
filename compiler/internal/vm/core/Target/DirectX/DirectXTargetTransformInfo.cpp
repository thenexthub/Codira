//===- DirectXTargetTransformInfo.cpp - DirectX TTI ---------------*- C++
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
///
//===----------------------------------------------------------------------===//

#include "DirectXTargetTransformInfo.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/IR/IntrinsicsDirectX.h"

using namespace vm::core;

bool DirectXTTIImpl::isTargetIntrinsicWithScalarOpAtArg(
    Intrinsic::ID ID, unsigned ScalarOpdIdx) const {
  switch (ID) {
  case Intrinsic::dx_wave_readlane:
    return ScalarOpdIdx == 1;
  default:
    return false;
  }
}

bool DirectXTTIImpl::isTargetIntrinsicWithOverloadTypeAtArg(Intrinsic::ID ID,
                                                            int OpdIdx) const {
  switch (ID) {
  case Intrinsic::dx_asdouble:
  case Intrinsic::dx_firstbitlow:
  case Intrinsic::dx_firstbitshigh:
  case Intrinsic::dx_firstbituhigh:
  case Intrinsic::dx_isinf:
  case Intrinsic::dx_isnan:
  case Intrinsic::dx_legacyf16tof32:
    return OpdIdx == 0;
  default:
    return OpdIdx == -1;
  }
}

bool DirectXTTIImpl::isTargetIntrinsicTriviallyScalarizable(
    Intrinsic::ID ID) const {
  switch (ID) {
  case Intrinsic::dx_asdouble:
  case Intrinsic::dx_firstbitlow:
  case Intrinsic::dx_firstbitshigh:
  case Intrinsic::dx_firstbituhigh:
  case Intrinsic::dx_frac:
  case Intrinsic::dx_isinf:
  case Intrinsic::dx_isnan:
  case Intrinsic::dx_legacyf16tof32:
  case Intrinsic::dx_rsqrt:
  case Intrinsic::dx_saturate:
  case Intrinsic::dx_splitdouble:
  case Intrinsic::dx_wave_readlane:
  case Intrinsic::dx_wave_reduce_max:
  case Intrinsic::dx_wave_reduce_min:
  case Intrinsic::dx_wave_reduce_sum:
  case Intrinsic::dx_wave_reduce_umax:
  case Intrinsic::dx_wave_reduce_umin:
  case Intrinsic::dx_wave_reduce_usum:
  case Intrinsic::dx_imad:
  case Intrinsic::dx_umad:
  case Intrinsic::dx_ddx_coarse:
  case Intrinsic::dx_ddy_coarse:
  case Intrinsic::dx_ddx_fine:
  case Intrinsic::dx_ddy_fine:
    return true;
  default:
    return false;
  }
}
