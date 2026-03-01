//===- SystemZConstantPoolValue.h - SystemZ constant-pool value -*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZCONSTANTPOOLVALUE_H
#define LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZCONSTANTPOOLVALUE_H

#include "vm/core/CodeGen/MachineConstantPool.h"
#include "vm/core/Support/ErrorHandling.h"

namespace vm::core {

class GlobalValue;

namespace SystemZCP {
enum SystemZCPModifier {
  TLSGD,
  TLSLDM,
  DTPOFF,
  NTPOFF
};
} // end namespace SystemZCP

/// A SystemZ-specific constant pool value.  At present, the only
/// defined constant pool values are module IDs or offsets of
/// thread-local variables (written x@TLSGD, x@TLSLDM, x@DTPOFF,
/// or x@NTPOFF).
class SystemZConstantPoolValue : public MachineConstantPoolValue {
  const GlobalValue *GV;
  SystemZCP::SystemZCPModifier Modifier;

protected:
  SystemZConstantPoolValue(const GlobalValue *GV,
                           SystemZCP::SystemZCPModifier Modifier);

public:
  static SystemZConstantPoolValue *
    Create(const GlobalValue *GV, SystemZCP::SystemZCPModifier Modifier);

  // Override MachineConstantPoolValue.
  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;
  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;
  void print(raw_ostream &O) const override;

  // Access SystemZ-specific fields.
  const GlobalValue *getGlobalValue() const { return GV; }
  SystemZCP::SystemZCPModifier getModifier() const { return Modifier; }
};

} // end namespace vm::core

#endif
