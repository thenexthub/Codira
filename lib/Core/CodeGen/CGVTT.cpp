//===--- CGVTT.cpp - Emit LLVM Code for C++ VTTs --------------------------===//
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
//
// This contains code dealing with C++ code generation of VTTs (vtable tables).
//
//===----------------------------------------------------------------------===//

#include "CodeGenModule.h"
#include "CGCXXABI.h"
#include "language/Core/AST/RecordLayout.h"
#include "language/Core/AST/VTTBuilder.h"
using namespace language::Core;
using namespace CodeGen;

static toolchain::GlobalVariable *
GetAddrOfVTTVTable(CodeGenVTables &CGVT, CodeGenModule &CGM,
                   const CXXRecordDecl *MostDerivedClass,
                   const VTTVTable &VTable,
                   toolchain::GlobalVariable::LinkageTypes Linkage,
                   VTableLayout::AddressPointsMapTy &AddressPoints) {
  if (VTable.getBase() == MostDerivedClass) {
    assert(VTable.getBaseOffset().isZero() &&
           "Most derived class vtable must have a zero offset!");
    // This is a regular vtable.
    return CGM.getCXXABI().getAddrOfVTable(MostDerivedClass, CharUnits());
  }

  return CGVT.GenerateConstructionVTable(MostDerivedClass,
                                         VTable.getBaseSubobject(),
                                         VTable.isVirtual(),
                                         Linkage,
                                         AddressPoints);
}

void
CodeGenVTables::EmitVTTDefinition(toolchain::GlobalVariable *VTT,
                                  toolchain::GlobalVariable::LinkageTypes Linkage,
                                  const CXXRecordDecl *RD) {
  VTTBuilder Builder(CGM.getContext(), RD, /*GenerateDefinition=*/true);
  toolchain::ArrayType *ArrayType = toolchain::ArrayType::get(
      CGM.GlobalsInt8PtrTy, Builder.getVTTComponents().size());

  SmallVector<toolchain::GlobalVariable *, 8> VTables;
  SmallVector<VTableAddressPointsMapTy, 8> VTableAddressPoints;
  for (const VTTVTable *i = Builder.getVTTVTables().begin(),
                       *e = Builder.getVTTVTables().end(); i != e; ++i) {
    VTableAddressPoints.push_back(VTableAddressPointsMapTy());
    VTables.push_back(GetAddrOfVTTVTable(*this, CGM, RD, *i, Linkage,
                                         VTableAddressPoints.back()));
  }

  SmallVector<toolchain::Constant *, 8> VTTComponents;
  for (const VTTComponent *i = Builder.getVTTComponents().begin(),
                          *e = Builder.getVTTComponents().end(); i != e; ++i) {
    const VTTVTable &VTTVT = Builder.getVTTVTables()[i->VTableIndex];
    toolchain::GlobalVariable *VTable = VTables[i->VTableIndex];
    VTableLayout::AddressPointLocation AddressPoint;
    if (VTTVT.getBase() == RD) {
      // Just get the address point for the regular vtable.
      AddressPoint =
          getItaniumVTableContext().getVTableLayout(RD).getAddressPoint(
              i->VTableBase);
    } else {
      AddressPoint = VTableAddressPoints[i->VTableIndex].lookup(i->VTableBase);
      assert(AddressPoint.AddressPointIndex != 0 &&
             "Did not find ctor vtable address point!");
    }

     toolchain::Value *Idxs[] = {
       toolchain::ConstantInt::get(CGM.Int32Ty, 0),
       toolchain::ConstantInt::get(CGM.Int32Ty, AddressPoint.VTableIndex),
       toolchain::ConstantInt::get(CGM.Int32Ty, AddressPoint.AddressPointIndex),
     };

     // Add inrange attribute to indicate that only the VTableIndex can be
     // accessed.
     unsigned ComponentSize =
         CGM.getDataLayout().getTypeAllocSize(getVTableComponentType());
     unsigned VTableSize = CGM.getDataLayout().getTypeAllocSize(
         cast<toolchain::StructType>(VTable->getValueType())
             ->getElementType(AddressPoint.VTableIndex));
     unsigned Offset = ComponentSize * AddressPoint.AddressPointIndex;
     toolchain::ConstantRange InRange(
         toolchain::APInt(32, (int)-Offset, true),
         toolchain::APInt(32, (int)(VTableSize - Offset), true));
     toolchain::Constant *Init = toolchain::ConstantExpr::getGetElementPtr(
         VTable->getValueType(), VTable, Idxs, /*InBounds=*/true, InRange);

     if (const auto &Schema =
             CGM.getCodeGenOpts().PointerAuth.CXXVTTVTablePointers)
       Init = CGM.getConstantSignedPointer(Init, Schema, nullptr, GlobalDecl(),
                                           QualType());

     VTTComponents.push_back(Init);
  }

  toolchain::Constant *Init = toolchain::ConstantArray::get(ArrayType, VTTComponents);

  VTT->setInitializer(Init);

  // Set the correct linkage.
  VTT->setLinkage(Linkage);

  if (CGM.supportsCOMDAT() && VTT->isWeakForLinker())
    VTT->setComdat(CGM.getModule().getOrInsertComdat(VTT->getName()));

  // Set the visibility. This will already have been set on the VTT declaration.
  // Set it again, now that we have a definition, as the implicit visibility can
  // apply differently to definitions.
  CGM.setGVProperties(VTT, RD);
}

toolchain::GlobalVariable *CodeGenVTables::GetAddrOfVTT(const CXXRecordDecl *RD) {
  assert(RD->getNumVBases() && "Only classes with virtual bases need a VTT");

  SmallString<256> OutName;
  toolchain::raw_svector_ostream Out(OutName);
  cast<ItaniumMangleContext>(CGM.getCXXABI().getMangleContext())
      .mangleCXXVTT(RD, Out);
  StringRef Name = OutName.str();

  // This will also defer the definition of the VTT.
  (void) CGM.getCXXABI().getAddrOfVTable(RD, CharUnits());

  VTTBuilder Builder(CGM.getContext(), RD, /*GenerateDefinition=*/false);

  toolchain::ArrayType *ArrayType = toolchain::ArrayType::get(
      CGM.GlobalsInt8PtrTy, Builder.getVTTComponents().size());
  toolchain::Align Align = CGM.getDataLayout().getABITypeAlign(CGM.GlobalsInt8PtrTy);

  toolchain::GlobalVariable *GV = CGM.CreateOrReplaceCXXRuntimeVariable(
      Name, ArrayType, toolchain::GlobalValue::ExternalLinkage, Align);
  GV->setUnnamedAddr(toolchain::GlobalValue::UnnamedAddr::Global);
  CGM.setGVProperties(GV, RD);
  return GV;
}

uint64_t CodeGenVTables::getSubVTTIndex(const CXXRecordDecl *RD,
                                        BaseSubobject Base) {
  BaseSubobjectPairTy ClassSubobjectPair(RD, Base);

  SubVTTIndicesMapTy::iterator I = SubVTTIndices.find(ClassSubobjectPair);
  if (I != SubVTTIndices.end())
    return I->second;

  VTTBuilder Builder(CGM.getContext(), RD, /*GenerateDefinition=*/false);

  for (toolchain::DenseMap<BaseSubobject, uint64_t>::const_iterator
           I = Builder.getSubVTTIndices().begin(),
           E = Builder.getSubVTTIndices().end();
       I != E; ++I) {
    // Insert all indices.
    BaseSubobjectPairTy ClassSubobjectPair(RD, I->first);

    SubVTTIndices.insert(std::make_pair(ClassSubobjectPair, I->second));
  }

  I = SubVTTIndices.find(ClassSubobjectPair);
  assert(I != SubVTTIndices.end() && "Did not find index!");

  return I->second;
}

uint64_t
CodeGenVTables::getSecondaryVirtualPointerIndex(const CXXRecordDecl *RD,
                                                BaseSubobject Base) {
  SecondaryVirtualPointerIndicesMapTy::iterator I =
    SecondaryVirtualPointerIndices.find(std::make_pair(RD, Base));

  if (I != SecondaryVirtualPointerIndices.end())
    return I->second;

  VTTBuilder Builder(CGM.getContext(), RD, /*GenerateDefinition=*/false);

  // Insert all secondary vpointer indices.
  for (toolchain::DenseMap<BaseSubobject, uint64_t>::const_iterator I =
       Builder.getSecondaryVirtualPointerIndices().begin(),
       E = Builder.getSecondaryVirtualPointerIndices().end(); I != E; ++I) {
    std::pair<const CXXRecordDecl *, BaseSubobject> Pair =
      std::make_pair(RD, I->first);

    SecondaryVirtualPointerIndices.insert(std::make_pair(Pair, I->second));
  }

  I = SecondaryVirtualPointerIndices.find(std::make_pair(RD, Base));
  assert(I != SecondaryVirtualPointerIndices.end() && "Did not find index!");

  return I->second;
}
