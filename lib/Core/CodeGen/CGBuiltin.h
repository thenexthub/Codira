//===------ CGBuiltin.h - Emit LLVM Code for builtins ---------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CGBUILTIN_H
#define LANGUAGE_CORE_LIB_CODEGEN_CGBUILTIN_H

#include "CodeGenFunction.h"

// Many of MSVC builtins are on x64, ARM and AArch64; to avoid repeating code,
// we handle them here.
enum class language::Core::CodeGen::CodeGenFunction::MSVCIntrin {
  _BitScanForward,
  _BitScanReverse,
  _InterlockedAnd,
  _InterlockedCompareExchange,
  _InterlockedDecrement,
  _InterlockedExchange,
  _InterlockedExchangeAdd,
  _InterlockedExchangeSub,
  _InterlockedIncrement,
  _InterlockedOr,
  _InterlockedXor,
  _InterlockedExchangeAdd_acq,
  _InterlockedExchangeAdd_rel,
  _InterlockedExchangeAdd_nf,
  _InterlockedExchange_acq,
  _InterlockedExchange_rel,
  _InterlockedExchange_nf,
  _InterlockedCompareExchange_acq,
  _InterlockedCompareExchange_rel,
  _InterlockedCompareExchange_nf,
  _InterlockedCompareExchange128,
  _InterlockedCompareExchange128_acq,
  _InterlockedCompareExchange128_rel,
  _InterlockedCompareExchange128_nf,
  _InterlockedOr_acq,
  _InterlockedOr_rel,
  _InterlockedOr_nf,
  _InterlockedXor_acq,
  _InterlockedXor_rel,
  _InterlockedXor_nf,
  _InterlockedAnd_acq,
  _InterlockedAnd_rel,
  _InterlockedAnd_nf,
  _InterlockedIncrement_acq,
  _InterlockedIncrement_rel,
  _InterlockedIncrement_nf,
  _InterlockedDecrement_acq,
  _InterlockedDecrement_rel,
  _InterlockedDecrement_nf,
  __fastfail,
};

// Emit a simple intrinsic that has N scalar arguments and a return type
// matching the argument type. It is assumed that only the first argument is
// overloaded.
template <unsigned N>
toolchain::Value *emitBuiltinWithOneOverloadedType(language::Core::CodeGen::CodeGenFunction &CGF,
                                              const language::Core::CallExpr *E,
                                              unsigned IntrinsicID,
                                              toolchain::StringRef Name = "") {
  static_assert(N, "expect non-empty argument");
  language::Core::SmallVector<toolchain::Value *, N> Args;
  for (unsigned I = 0; I < N; ++I)
    Args.push_back(CGF.EmitScalarExpr(E->getArg(I)));
  toolchain::Function *F = CGF.CGM.getIntrinsic(IntrinsicID, Args[0]->getType());
  return CGF.Builder.CreateCall(F, Args, Name);
}

toolchain::Value *emitUnaryMaybeConstrainedFPBuiltin(language::Core::CodeGen::CodeGenFunction &CGF,
                                                const language::Core::CallExpr *E,
                                                unsigned IntrinsicID,
                                                unsigned ConstrainedIntrinsicID);

toolchain::Value *EmitToInt(language::Core::CodeGen::CodeGenFunction &CGF, toolchain::Value *V,
                       language::Core::QualType T, toolchain::IntegerType *IntType);

toolchain::Value *EmitFromInt(language::Core::CodeGen::CodeGenFunction &CGF, toolchain::Value *V,
                         language::Core::QualType T, toolchain::Type *ResultType);

language::Core::CodeGen::Address CheckAtomicAlignment(language::Core::CodeGen::CodeGenFunction &CGF,
                                             const language::Core::CallExpr *E);

toolchain::Value *MakeBinaryAtomicValue(language::Core::CodeGen::CodeGenFunction &CGF,
                                   toolchain::AtomicRMWInst::BinOp Kind,
                                   const language::Core::CallExpr *E,
                                   toolchain::AtomicOrdering Ordering =
                                      toolchain::AtomicOrdering::SequentiallyConsistent);

toolchain::Value *EmitOverflowIntrinsic(language::Core::CodeGen::CodeGenFunction &CGF,
                                   const toolchain::Intrinsic::ID IntrinsicID,
                                   toolchain::Value *X,
                                   toolchain::Value *Y,
                                   toolchain::Value *&Carry);

toolchain::Value *MakeAtomicCmpXchgValue(language::Core::CodeGen::CodeGenFunction &CGF,
                                    const language::Core::CallExpr *E,
                                    bool ReturnBool);

#endif
