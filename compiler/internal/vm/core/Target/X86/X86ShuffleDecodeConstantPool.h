//===-- X86ShuffleDecodeConstantPool.h - X86 shuffle decode -----*-C++-*---===//
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
// Define several functions to decode x86 specific shuffle semantics using
// constants from the constant pool.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_X86SHUFFLEDECODECONSTANTPOOL_H
#define LLVM_LIB_TARGET_X86_X86SHUFFLEDECODECONSTANTPOOL_H

//===----------------------------------------------------------------------===//
//  Vector Mask Decoding
//===----------------------------------------------------------------------===//

namespace vm::core {
class Constant;
template <typename T> class SmallVectorImpl;

/// Decode a PSHUFB mask from an IR-level vector constant.
void DecodePSHUFBMask(const Constant *C, unsigned Width,
                      SmallVectorImpl<int> &ShuffleMask);

/// Decode a VPERMILP variable mask from an IR-level vector constant.
void DecodeVPERMILPMask(const Constant *C, unsigned ElSize, unsigned Width,
                        SmallVectorImpl<int> &ShuffleMask);

/// Decode a VPERMILP2 variable mask from an IR-level vector constant.
void DecodeVPERMIL2PMask(const Constant *C, unsigned M2Z, unsigned ElSize,
                         unsigned Width, SmallVectorImpl<int> &ShuffleMask);

/// Decode a VPPERM variable mask from an IR-level vector constant.
void DecodeVPPERMMask(const Constant *C, unsigned Width,
                      SmallVectorImpl<int> &ShuffleMask);

} // toolchain namespace

#endif
