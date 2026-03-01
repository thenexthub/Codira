//===-- CSKYConstantPoolValue.cpp - CSKY constantpool value ---------------===//
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
// This file implements the CSKY specific constantpool value class.
//
//===----------------------------------------------------------------------===//

#include "CSKYConstantPoolValue.h"
#include "vm/core/ADT/FoldingSet.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/IR/Constant.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/IR/Type.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

//===----------------------------------------------------------------------===//
// CSKYConstantPoolValue
//===----------------------------------------------------------------------===//

CSKYConstantPoolValue::CSKYConstantPoolValue(Type *Ty, CSKYCP::CSKYCPKind Kind,
                                             unsigned PCAdjust,
                                             CSKYCP::CSKYCPModifier Modifier,
                                             bool AddCurrentAddress,
                                             unsigned ID)
    : MachineConstantPoolValue(Ty), Kind(Kind), PCAdjust(PCAdjust),
      Modifier(Modifier), AddCurrentAddress(AddCurrentAddress), LabelId(ID) {}

const char *CSKYConstantPoolValue::getModifierText() const {
  switch (Modifier) {
  case CSKYCP::ADDR:
    return "ADDR";
  case CSKYCP::GOT:
    return "GOT";
  case CSKYCP::GOTOFF:
    return "GOTOFF";
  case CSKYCP::PLT:
    return "PLT";
  case CSKYCP::TLSIE:
    return "TLSIE";
  case CSKYCP::TLSLE:
    return "TLSLE";
  case CSKYCP::TLSGD:
    return "TLSGD";
  case CSKYCP::NO_MOD:
    return "";
  }
  llvm_unreachable("Unknown modifier!");
}

int CSKYConstantPoolValue::getExistingMachineCPValue(MachineConstantPool *CP,
                                                     Align Alignment) {
  llvm_unreachable("Shouldn't be calling this directly!");
}

void CSKYConstantPoolValue::addSelectionDAGCSEId(FoldingSetNodeID &ID) {
  ID.AddInteger(LabelId);
  ID.AddInteger(PCAdjust);
  ID.AddInteger(Modifier);
}

void CSKYConstantPoolValue::print(raw_ostream &O) const {
  if (Modifier)
    O << "(" << getModifierText() << ")";
  if (PCAdjust)
    O << " + " << PCAdjust;
}

//===----------------------------------------------------------------------===//
// CSKYConstantPoolConstant
//===----------------------------------------------------------------------===//

CSKYConstantPoolConstant::CSKYConstantPoolConstant(
    const Constant *C, Type *Ty, CSKYCP::CSKYCPKind Kind, unsigned PCAdjust,
    CSKYCP::CSKYCPModifier Modifier, bool AddCurrentAddress, unsigned ID)
    : CSKYConstantPoolValue(Ty, Kind, PCAdjust, Modifier, AddCurrentAddress,
                            ID),
      CVal(C) {}

CSKYConstantPoolConstant *CSKYConstantPoolConstant::Create(
    const Constant *C, CSKYCP::CSKYCPKind Kind, unsigned PCAdjust,
    CSKYCP::CSKYCPModifier Modifier, bool AddCurrentAddress, unsigned ID) {
  return new CSKYConstantPoolConstant(C, C->getType(), Kind, PCAdjust, Modifier,
                                      AddCurrentAddress, ID);
}

CSKYConstantPoolConstant *CSKYConstantPoolConstant::Create(
    const Constant *C, Type *Ty, CSKYCP::CSKYCPKind Kind, unsigned PCAdjust,
    CSKYCP::CSKYCPModifier Modifier, bool AddCurrentAddress, unsigned ID) {
  return new CSKYConstantPoolConstant(C, Ty, Kind, PCAdjust, Modifier,
                                      AddCurrentAddress, ID);
}

const GlobalValue *CSKYConstantPoolConstant::getGV() const {
  assert(isa<GlobalValue>(CVal) && "CVal should be GlobalValue");
  return cast<GlobalValue>(CVal);
}

const BlockAddress *CSKYConstantPoolConstant::getBlockAddress() const {
  assert(isa<BlockAddress>(CVal) && "CVal should be BlockAddress");
  return cast<BlockAddress>(CVal);
}

const Constant *CSKYConstantPoolConstant::getConstantPool() const {
  return CVal;
}

int CSKYConstantPoolConstant::getExistingMachineCPValue(MachineConstantPool *CP,
                                                        Align Alignment) {
  return getExistingMachineCPValueImpl<CSKYConstantPoolConstant>(CP, Alignment);
}

void CSKYConstantPoolConstant::addSelectionDAGCSEId(FoldingSetNodeID &ID) {
  ID.AddPointer(CVal);

  CSKYConstantPoolValue::addSelectionDAGCSEId(ID);
}

void CSKYConstantPoolConstant::print(raw_ostream &O) const {
  O << CVal->getName();
  CSKYConstantPoolValue::print(O);
}

//===----------------------------------------------------------------------===//
// CSKYConstantPoolSymbol
//===----------------------------------------------------------------------===//

CSKYConstantPoolSymbol::CSKYConstantPoolSymbol(Type *Ty, const char *S,
                                               unsigned PCAdjust,
                                               CSKYCP::CSKYCPModifier Modifier,
                                               bool AddCurrentAddress)
    : CSKYConstantPoolValue(Ty, CSKYCP::CPExtSymbol, PCAdjust, Modifier,
                            AddCurrentAddress),
      S(strdup(S)) {}

CSKYConstantPoolSymbol *
CSKYConstantPoolSymbol::Create(Type *Ty, const char *S, unsigned PCAdjust,
                               CSKYCP::CSKYCPModifier Modifier) {
  return new CSKYConstantPoolSymbol(Ty, S, PCAdjust, Modifier, false);
}

int CSKYConstantPoolSymbol::getExistingMachineCPValue(MachineConstantPool *CP,
                                                      Align Alignment) {

  return getExistingMachineCPValueImpl<CSKYConstantPoolSymbol>(CP, Alignment);
}

void CSKYConstantPoolSymbol::addSelectionDAGCSEId(FoldingSetNodeID &ID) {
  ID.AddString(S);
  CSKYConstantPoolValue::addSelectionDAGCSEId(ID);
}

void CSKYConstantPoolSymbol::print(raw_ostream &O) const {
  O << S;
  CSKYConstantPoolValue::print(O);
}

//===----------------------------------------------------------------------===//
// CSKYConstantPoolMBB
//===----------------------------------------------------------------------===//

CSKYConstantPoolMBB::CSKYConstantPoolMBB(Type *Ty, const MachineBasicBlock *Mbb,
                                         unsigned PCAdjust,
                                         CSKYCP::CSKYCPModifier Modifier,
                                         bool AddCurrentAddress)
    : CSKYConstantPoolValue(Ty, CSKYCP::CPMachineBasicBlock, PCAdjust, Modifier,
                            AddCurrentAddress),
      MBB(Mbb) {}

CSKYConstantPoolMBB *CSKYConstantPoolMBB::Create(Type *Ty,
                                                 const MachineBasicBlock *Mbb,
                                                 unsigned PCAdjust) {
  return new CSKYConstantPoolMBB(Ty, Mbb, PCAdjust, CSKYCP::ADDR, false);
}

int CSKYConstantPoolMBB::getExistingMachineCPValue(MachineConstantPool *CP,
                                                   Align Alignment) {
  return getExistingMachineCPValueImpl<CSKYConstantPoolMBB>(CP, Alignment);
}

void CSKYConstantPoolMBB::addSelectionDAGCSEId(FoldingSetNodeID &ID) {
  ID.AddPointer(MBB);
  CSKYConstantPoolValue::addSelectionDAGCSEId(ID);
}

void CSKYConstantPoolMBB::print(raw_ostream &O) const {
  O << "BB#" << MBB->getNumber();
  CSKYConstantPoolValue::print(O);
}

//===----------------------------------------------------------------------===//
// CSKYConstantPoolJT
//===----------------------------------------------------------------------===//

CSKYConstantPoolJT::CSKYConstantPoolJT(Type *Ty, int JTIndex, unsigned PCAdj,
                                       CSKYCP::CSKYCPModifier Modifier,
                                       bool AddCurrentAddress)
    : CSKYConstantPoolValue(Ty, CSKYCP::CPJT, PCAdj, Modifier,
                            AddCurrentAddress),
      JTI(JTIndex) {}

CSKYConstantPoolJT *
CSKYConstantPoolJT::Create(Type *Ty, int JTI, unsigned PCAdj,
                           CSKYCP::CSKYCPModifier Modifier) {
  return new CSKYConstantPoolJT(Ty, JTI, PCAdj, Modifier, false);
}

int CSKYConstantPoolJT::getExistingMachineCPValue(MachineConstantPool *CP,
                                                  Align Alignment) {
  return getExistingMachineCPValueImpl<CSKYConstantPoolJT>(CP, Alignment);
}

void CSKYConstantPoolJT::addSelectionDAGCSEId(FoldingSetNodeID &ID) {
  ID.AddInteger(JTI);
  CSKYConstantPoolValue::addSelectionDAGCSEId(ID);
}

void CSKYConstantPoolJT::print(raw_ostream &O) const {
  O << "JTI#" << JTI;
  CSKYConstantPoolValue::print(O);
}
