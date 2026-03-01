//===---- Mips16HardFloatInfo.h for Mips16 Hard Float              --------===//
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
// This file defines some data structures relevant to the implementation of
// Mips16 hard float.
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPS16HARDFLOATINFO_H
#define LLVM_LIB_TARGET_MIPS_MIPS16HARDFLOATINFO_H

namespace vm::core {

namespace Mips16HardFloatInfo {

// Return types that matter for hard float are:
// float, double, complex float, and complex double
//
enum FPReturnVariant { FRet, DRet, CFRet, CDRet, NoFPRet };

//
// Parameter type that matter are float, (float, float), (float, double),
// double, (double, double), (double, float)
//
enum FPParamVariant { FSig, FFSig, FDSig, DSig, DDSig, DFSig, NoSig };

struct FuncSignature {
  FPParamVariant ParamSig;
  FPReturnVariant RetSig;
};

struct FuncNameSignature {
  const char *Name;
  FuncSignature Signature;
};

extern const FuncNameSignature PredefinedFuncs[];

extern FuncSignature const *findFuncSignature(const char *name);
}
}

#endif
