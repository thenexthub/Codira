//===- SPIRVTargetTransformInfo.cpp - SPIR-V specific TTI -------*- C++ -*-===//
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

#include "SPIRVTargetTransformInfo.h"
#include "vm/core/IR/IntrinsicsSPIRV.h"

using namespace vm::core;

bool toolchain::SPIRVTTIImpl::collectFlatAddressOperands(
    SmallVectorImpl<int> &OpIndexes, Intrinsic::ID IID) const {
  switch (IID) {
  case Intrinsic::spv_generic_cast_to_ptr_explicit:
    OpIndexes.push_back(0);
    return true;
  default:
    return false;
  }
}

Value *toolchain::SPIRVTTIImpl::rewriteIntrinsicWithAddressSpace(IntrinsicInst *II,
                                                            Value *OldV,
                                                            Value *NewV) const {
  auto IntrID = II->getIntrinsicID();
  switch (IntrID) {
  case Intrinsic::spv_generic_cast_to_ptr_explicit: {
    unsigned NewAS = NewV->getType()->getPointerAddressSpace();
    unsigned DstAS = II->getType()->getPointerAddressSpace();
    return NewAS == DstAS ? NewV
                          : ConstantPointerNull::get(
                                PointerType::get(NewV->getContext(), DstAS));
  }
  default:
    return nullptr;
  }
}
