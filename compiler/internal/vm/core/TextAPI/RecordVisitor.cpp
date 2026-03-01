//===- RecordVisitor.cpp --------------------------------------------------===//
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
/// Implements the TAPI Record Visitor.
///
//===----------------------------------------------------------------------===//

#include "vm/core/TextAPI/RecordVisitor.h"

using namespace vm::core;
using namespace vm::core::MachO;

RecordVisitor::~RecordVisitor() = default;
void RecordVisitor::visitObjCInterface(const ObjCInterfaceRecord &) {}
void RecordVisitor::visitObjCCategory(const ObjCCategoryRecord &) {}

static bool shouldSkipRecord(const Record &R, const bool RecordUndefs) {
  if (R.isExported())
    return false;

  // Skip non exported symbols unless for flat namespace libraries.
  return !(RecordUndefs && R.isUndefined());
}

void SymbolConverter::visitGlobal(const GlobalRecord &GR) {
  auto [SymName, SymKind, InterfaceType] = parseSymbol(GR.getName());
  if (shouldSkipRecord(GR, RecordUndefs))
    return;
  Symbols->addGlobal(SymKind, SymName, GR.getFlags(), Targ);

  if (InterfaceType == ObjCIFSymbolKind::None) {
    Symbols->addGlobal(SymKind, SymName, GR.getFlags(), Targ);
    return;
  }

  // It is impossible to hold a complete ObjCInterface with a single
  // GlobalRecord, so continue to treat this symbol a generic global.
  Symbols->addGlobal(EncodeKind::GlobalSymbol, GR.getName(), GR.getFlags(),
                     Targ);
}

void SymbolConverter::addIVars(const ArrayRef<ObjCIVarRecord *> IVars,
                               StringRef ContainerName) {
  for (auto *IV : IVars) {
    if (shouldSkipRecord(*IV, RecordUndefs))
      continue;
    std::string Name =
        ObjCIVarRecord::createScopedName(ContainerName, IV->getName());
    Symbols->addGlobal(EncodeKind::ObjectiveCInstanceVariable, Name,
                       IV->getFlags(), Targ);
  }
}

void SymbolConverter::visitObjCInterface(const ObjCInterfaceRecord &ObjCR) {
  if (!shouldSkipRecord(ObjCR, RecordUndefs)) {
    if (ObjCR.isCompleteInterface()) {
      Symbols->addGlobal(EncodeKind::ObjectiveCClass, ObjCR.getName(),
                         ObjCR.getFlags(), Targ);
      if (ObjCR.hasExceptionAttribute())
        Symbols->addGlobal(EncodeKind::ObjectiveCClassEHType, ObjCR.getName(),
                           ObjCR.getFlags(), Targ);
    } else {
      // Because there is not a complete interface, visit individual symbols
      // instead.
      if (ObjCR.isExportedSymbol(ObjCIFSymbolKind::EHType))
        Symbols->addGlobal(EncodeKind::GlobalSymbol,
                           (ObjC2EHTypePrefix + ObjCR.getName()).str(),
                           ObjCR.getFlags(), Targ);
      if (ObjCR.isExportedSymbol(ObjCIFSymbolKind::Class))
        Symbols->addGlobal(EncodeKind::GlobalSymbol,
                           (ObjC2ClassNamePrefix + ObjCR.getName()).str(),
                           ObjCR.getFlags(), Targ);
      if (ObjCR.isExportedSymbol(ObjCIFSymbolKind::MetaClass))
        Symbols->addGlobal(EncodeKind::GlobalSymbol,
                           (ObjC2MetaClassNamePrefix + ObjCR.getName()).str(),
                           ObjCR.getFlags(), Targ);
    }
  }

  addIVars(ObjCR.getObjCIVars(), ObjCR.getName());
  for (const auto *Cat : ObjCR.getObjCCategories())
    addIVars(Cat->getObjCIVars(), ObjCR.getName());
}

void SymbolConverter::visitObjCCategory(const ObjCCategoryRecord &Cat) {
  addIVars(Cat.getObjCIVars(), Cat.getSuperClassName());
}
