//===- XtensaConstantPoolValue.h - Xtensa constantpool value ----*- C++ -*-===//
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
// This file implements the Xtensa specific constantpool value class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XTENSA_XTENSACONSTANTPOOLVALUE_H
#define LLVM_LIB_TARGET_XTENSA_XTENSACONSTANTPOOLVALUE_H

#include "vm/core/CodeGen/MachineConstantPool.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/ErrorHandling.h"
#include <cstddef>
#include <string>
#include <vector>

namespace vm::core {

class BlockAddress;
class Constant;
class GlobalValue;
class LLVMContext;
class MachineBasicBlock;

namespace XtensaCP {
enum XtensaCPKind {
  CPExtSymbol,
  CPBlockAddress,
  CPMachineBasicBlock,
  CPJumpTable
};

enum XtensaCPModifier {
  no_modifier, // None
  TPOFF        // Thread Pointer Offset
};
} // namespace XtensaCP

/// XtensaConstantPoolValue - Xtensa specific constantpool value. This is used
/// to represent PC-relative displacement between the address of the load
/// instruction and the constant being loaded.
class XtensaConstantPoolValue : public MachineConstantPoolValue {
  unsigned LabelId;                    // Label id of the load.
  XtensaCP::XtensaCPKind Kind;         // Kind of constant.
  XtensaCP::XtensaCPModifier Modifier; // Symbol name modifier
                                       //(for example Global Variable name)

protected:
  XtensaConstantPoolValue(
      Type *Ty, unsigned ID, XtensaCP::XtensaCPKind Kind,
      XtensaCP::XtensaCPModifier Modifier = XtensaCP::no_modifier);

  XtensaConstantPoolValue(
      LLVMContext &C, unsigned id, XtensaCP::XtensaCPKind Kind,
      XtensaCP::XtensaCPModifier Modifier = XtensaCP::no_modifier);

  template <typename Derived>
  int getExistingMachineCPValueImpl(MachineConstantPool *CP, Align Alignment) {
    const std::vector<MachineConstantPoolEntry> &Constants = CP->getConstants();
    for (unsigned i = 0, e = Constants.size(); i != e; ++i) {
      if (Constants[i].isMachineConstantPoolEntry() &&
          (Constants[i].getAlign() >= Alignment)) {
        auto *CPV = static_cast<XtensaConstantPoolValue *>(
            Constants[i].Val.MachineCPVal);
        if (Derived *APC = dyn_cast<Derived>(CPV))
          if (cast<Derived>(this)->equals(APC))
            return i;
      }
    }

    return -1;
  }

public:
  ~XtensaConstantPoolValue() override;

  XtensaCP::XtensaCPModifier getModifier() const { return Modifier; }
  bool hasModifier() const { return Modifier != XtensaCP::no_modifier; }
  StringRef getModifierText() const;

  unsigned getLabelId() const { return LabelId; }
  void setLabelId(unsigned ID) { LabelId = ID; }

  bool isExtSymbol() const { return Kind == XtensaCP::CPExtSymbol; }
  bool isBlockAddress() const { return Kind == XtensaCP::CPBlockAddress; }
  bool isMachineBasicBlock() const {
    return Kind == XtensaCP::CPMachineBasicBlock;
  }
  bool isJumpTable() const { return Kind == XtensaCP::CPJumpTable; }

  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;

  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;

  /// hasSameValue - Return true if this Xtensa constpool value can share the
  /// same constantpool entry as another Xtensa constpool value.
  virtual bool hasSameValue(XtensaConstantPoolValue *ACPV);

  bool equals(const XtensaConstantPoolValue *A) const {
    return this->LabelId == A->LabelId && this->Modifier == A->Modifier;
  }

  void print(raw_ostream &O) const override;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
#endif
};

inline raw_ostream &operator<<(raw_ostream &O,
                               const XtensaConstantPoolValue &V) {
  V.print(O);
  return O;
}

/// XtensaConstantPoolConstant - Xtensa-specific constant pool values for
/// Constants (for example BlockAddresses).
class XtensaConstantPoolConstant : public XtensaConstantPoolValue {
  const Constant *CVal; // Constant being loaded.

  XtensaConstantPoolConstant(const Constant *C, unsigned ID,
                             XtensaCP::XtensaCPKind Kind);

public:
  static XtensaConstantPoolConstant *Create(const Constant *C, unsigned ID,
                                            XtensaCP::XtensaCPKind Kind);

  const BlockAddress *getBlockAddress() const;

  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;

  /// hasSameValue - Return true if this Xtensa constpool value can share the
  /// same constantpool entry as another Xtensa constpool value.
  bool hasSameValue(XtensaConstantPoolValue *ACPV) override;

  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;

  void print(raw_ostream &O) const override;
  static bool classof(const XtensaConstantPoolValue *APV) {
    return APV->isBlockAddress();
  }

  bool equals(const XtensaConstantPoolConstant *A) const {
    return CVal == A->CVal && XtensaConstantPoolValue::equals(A);
  }
};

/// XtensaConstantPoolSymbol - Xtensa-specific constantpool values for external
/// symbols.
class XtensaConstantPoolSymbol : public XtensaConstantPoolValue {
  const std::string S; // ExtSymbol being loaded.
  bool PrivateLinkage;

  XtensaConstantPoolSymbol(
      LLVMContext &C, const char *S, unsigned Id, bool PrivLinkage,
      XtensaCP::XtensaCPModifier Modifier = XtensaCP::no_modifier);

public:
  static XtensaConstantPoolSymbol *
  Create(LLVMContext &C, const char *S, unsigned ID, bool PrivLinkage,
         XtensaCP::XtensaCPModifier Modifier = XtensaCP::no_modifier);

  const char *getSymbol() const { return S.c_str(); }

  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;

  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;

  /// hasSameValue - Return true if this Xtensa constpool value can share the
  /// same constantpool entry as another Xtensa constpool value.
  bool hasSameValue(XtensaConstantPoolValue *ACPV) override;

  bool isPrivateLinkage() { return PrivateLinkage; }

  void print(raw_ostream &O) const override;

  static bool classof(const XtensaConstantPoolValue *ACPV) {
    return ACPV->isExtSymbol();
  }

  bool equals(const XtensaConstantPoolSymbol *A) const {
    return S == A->S && XtensaConstantPoolValue::equals(A);
  }
};

/// XtensaConstantPoolMBB - Xtensa-specific constantpool value of a machine
/// basic block.
class XtensaConstantPoolMBB : public XtensaConstantPoolValue {
  const MachineBasicBlock *MBB; // Machine basic block.

  XtensaConstantPoolMBB(LLVMContext &C, const MachineBasicBlock *M,
                        unsigned ID);

public:
  static XtensaConstantPoolMBB *Create(LLVMContext &C,
                                       const MachineBasicBlock *M, unsigned ID);

  const MachineBasicBlock *getMBB() const { return MBB; }

  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;

  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;

  /// hasSameValue - Return true if this Xtensa constpool value can share the
  /// same constantpool entry as another Xtensa constpool value.
  bool hasSameValue(XtensaConstantPoolValue *ACPV) override;

  void print(raw_ostream &O) const override;

  static bool classof(const XtensaConstantPoolValue *ACPV) {
    return ACPV->isMachineBasicBlock();
  }

  bool equals(const XtensaConstantPoolMBB *A) const {
    return MBB == A->MBB && XtensaConstantPoolValue::equals(A);
  }
};

/// XtensaConstantPoolJumpTable - Xtensa-specific constantpool values for Jump
/// Table symbols.
class XtensaConstantPoolJumpTable : public XtensaConstantPoolValue {
  unsigned Idx; // Jump Table Index.

  XtensaConstantPoolJumpTable(LLVMContext &C, unsigned Idx);

public:
  static XtensaConstantPoolJumpTable *Create(LLVMContext &C, unsigned Idx);

  unsigned getIndex() const { return Idx; }

  int getExistingMachineCPValue(MachineConstantPool *CP,
                                Align Alignment) override;

  void addSelectionDAGCSEId(FoldingSetNodeID &ID) override;

  /// hasSameValue - Return true if this Xtensa constpool value can share the
  /// same constantpool entry as another Xtensa constpool value.
  bool hasSameValue(XtensaConstantPoolValue *ACPV) override;

  void print(raw_ostream &O) const override;

  static bool classof(const XtensaConstantPoolValue *ACPV) {
    return ACPV->isJumpTable();
  }

  bool equals(const XtensaConstantPoolJumpTable *A) const {
    return Idx == A->Idx && XtensaConstantPoolValue::equals(A);
  }
};

} // namespace vm::core

#endif /* LLVM_LIB_TARGET_XTENSA_XTENSACONSTANTPOOLVALUE_H */
