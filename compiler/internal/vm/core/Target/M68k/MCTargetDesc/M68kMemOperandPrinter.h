//===-- M68kMemOperandPrinter.h - Memory operands printing ------*- C++ -*-===//
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
/// \file
/// This file contains memory operand printing logics shared between AsmPrinter
//  and MCInstPrinter.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_M68K_MEMOPERANDPRINTER_M68KINSTPRINTER_H
#define LLVM_LIB_TARGET_M68K_MEMOPERANDPRINTER_M68KINSTPRINTER_H

#include "M68kBaseInfo.h"

#include "vm/core/Support/raw_ostream.h"

namespace vm::core {
template <class Derived, typename InstTy> class M68kMemOperandPrinter {
  Derived &impl() { return *static_cast<Derived *>(this); }

protected:
  void printARIMem(const InstTy *MI, unsigned OpNum, raw_ostream &O) {
    O << '(';
    impl().printOperand(MI, OpNum, O);
    O << ')';
  }

  void printARIPIMem(const InstTy *MI, unsigned OpNum, raw_ostream &O) {
    O << "(";
    impl().printOperand(MI, OpNum, O);
    O << ")+";
  }

  void printARIPDMem(const InstTy *MI, unsigned OpNum, raw_ostream &O) {
    O << "-(";
    impl().printOperand(MI, OpNum, O);
    O << ")";
  }

  void printARIDMem(const InstTy *MI, unsigned OpNum, raw_ostream &O) {
    O << '(';
    impl().printDisp(MI, OpNum + M68k::MemDisp, O);
    O << ',';
    impl().printOperand(MI, OpNum + M68k::MemBase, O);
    O << ')';
  }

  void printARIIMem(const InstTy *MI, unsigned OpNum, raw_ostream &O) {
    O << '(';
    impl().printDisp(MI, OpNum + M68k::MemDisp, O);
    O << ',';
    impl().printOperand(MI, OpNum + M68k::MemBase, O);
    O << ',';
    impl().printOperand(MI, OpNum + M68k::MemIndex, O);
    O << ')';
  }

  void printPCDMem(const InstTy *MI, uint64_t Address, unsigned OpNum,
                   raw_ostream &O) {
    O << '(';
    impl().printDisp(MI, OpNum + M68k::PCRelDisp, O);
    O << ",%pc)";
  }

  void printPCIMem(const InstTy *MI, uint64_t Address, unsigned OpNum,
                   raw_ostream &O) {
    O << '(';
    impl().printDisp(MI, OpNum + M68k::PCRelDisp, O);
    O << ",%pc,";
    impl().printOperand(MI, OpNum + M68k::PCRelIndex, O);
    O << ')';
  }
};
} // end namespace vm::core
#endif
