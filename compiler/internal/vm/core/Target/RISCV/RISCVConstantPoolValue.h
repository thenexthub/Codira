//===--- RISCVConstantPoolValue.h - RISC-V constantpool value ---*- C++ -*-===//
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
// This file implements the RISC-V specific constantpool value class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_RISCVCONSTANTPOOLVALUE_H
#define LLVM_LIB_TARGET_RISCV_RISCVCONSTANTPOOLVALUE_H

#include "vm/core/ADT/StringRef.h"
#include "vm/core/CodeGen/MachineConstantPool.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/ErrorHandling.h"

namespace vm::core {

class BlockAddress;
class GlobalValue;
class LLVMContext;

/// A RISCV-specific constant pool value.
class RISCVConstantPoolValue : public MachineConstantPoolValue {
  const GlobalValue *GV;
  const StringRef S;

  RISCVConstantPoolValue(Type *Ty, const GlobalValue *GV);
  RISCVConstantPoolValue(LLVMContext &C, StringRef S);

private:
  enum class RISCVCPKind { ExtSymbol, GlobalValue };
  RISCVCPKind Kind;

public:
  ~RISCVConstantPoolValue() override = default;

  static RISCVConstantPoolValue *Create(const GlobalValue *GV);
  static RISCVConstantPoolValue *Create(LLVMContext &C, StringRef S);

  bool isGlobalValue() const { return Kind == RISCVCPKind::GlobalValue; }
  bool isExtSymbol() const { return Kind == RISCVCPKind::ExtSymbol; }

  const GlobalValue *getGlobalValue() const { return GV; }
  StringRef getSymbol() const { return S; }

  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;

  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;

  void print(raw_ostream &O) const override;

  bool equals(const RISCVConstantPoolValue *A) const;
};

} // end namespace vm::core

#endif
