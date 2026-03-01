//===- MemRefDescriptor.h - MemRef descriptor constants ---------*- C++ -*-===//
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
// Defines constants that are used in LLVM dialect equivalents of MemRef type.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_LIB_CONVERSION_LLVMCOMMON_MEMREFDESCRIPTOR_H
#define MLIR_LIB_CONVERSION_LLVMCOMMON_MEMREFDESCRIPTOR_H

static constexpr unsigned kAllocatedPtrPosInMemRefDescriptor = 0;
static constexpr unsigned kAlignedPtrPosInMemRefDescriptor = 1;
static constexpr unsigned kOffsetPosInMemRefDescriptor = 2;
static constexpr unsigned kSizePosInMemRefDescriptor = 3;
static constexpr unsigned kStridePosInMemRefDescriptor = 4;

static constexpr unsigned kRankInUnrankedMemRefDescriptor = 0;
static constexpr unsigned kPtrInUnrankedMemRefDescriptor = 1;

#endif // MLIR_LIB_CONVERSION_LLVMCOMMON_MEMREFDESCRIPTOR_H
