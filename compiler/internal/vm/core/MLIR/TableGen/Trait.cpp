//===- Trait.cpp ----------------------------------------------------------===//
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
// Trait wrapper to simplify using TableGen Record defining a MLIR Trait.
//
//===----------------------------------------------------------------------===//

#include "mlir/TableGen/Trait.h"
#include "mlir/TableGen/Interfaces.h"
#include "mlir/TableGen/Predicate.h"
#include "vm/core/TableGen/Error.h"
#include "vm/core/TableGen/Record.h"

using namespace mlir;
using namespace mlir::tblgen;

//===----------------------------------------------------------------------===//
// Trait
//===----------------------------------------------------------------------===//

Trait Trait::create(const toolchain::Init *init) {
  auto *def = cast<toolchain::DefInit>(init)->getDef();
  if (def->isSubClassOf("PredTrait"))
    return Trait(Kind::Pred, def);
  if (def->isSubClassOf("GenInternalTrait"))
    return Trait(Kind::Internal, def);
  if (def->isSubClassOf("InterfaceTrait"))
    return Trait(Kind::Interface, def);
  assert(def->isSubClassOf("NativeTrait"));
  return Trait(Kind::Native, def);
}

Trait::Trait(Kind kind, const toolchain::Record *def) : def(def), kind(kind) {}

//===----------------------------------------------------------------------===//
// NativeTrait
//===----------------------------------------------------------------------===//

std::string NativeTrait::getFullyQualifiedTraitName() const {
  toolchain::StringRef trait = def->getValueAsString("trait");
  toolchain::StringRef cppNamespace = def->getValueAsString("cppNamespace");
  return cppNamespace.empty() ? trait.str()
                              : (cppNamespace + "::" + trait).str();
}

bool NativeTrait::isStructuralOpTrait() const {
  return def->isSubClassOf("StructuralOpTrait");
}

StringRef NativeTrait::getExtraConcreteClassDeclaration() const {
  return def->getValueAsString("extraConcreteClassDeclaration");
}

StringRef NativeTrait::getExtraConcreteClassDefinition() const {
  return def->getValueAsString("extraConcreteClassDefinition");
}

//===----------------------------------------------------------------------===//
// InternalTrait
//===----------------------------------------------------------------------===//

toolchain::StringRef InternalTrait::getFullyQualifiedTraitName() const {
  return def->getValueAsString("trait");
}

//===----------------------------------------------------------------------===//
// PredTrait
//===----------------------------------------------------------------------===//

std::string PredTrait::getPredTemplate() const {
  auto pred = Pred(def->getValueInit("predicate"));
  return pred.getCondition();
}

toolchain::StringRef PredTrait::getSummary() const {
  return def->getValueAsString("summary");
}

//===----------------------------------------------------------------------===//
// InterfaceTrait
//===----------------------------------------------------------------------===//

Interface InterfaceTrait::getInterface() const { return Interface(def); }

std::string InterfaceTrait::getFullyQualifiedTraitName() const {
  toolchain::StringRef trait = def->getValueAsString("trait");
  toolchain::StringRef cppNamespace = def->getValueAsString("cppNamespace");
  return cppNamespace.empty() ? trait.str()
                              : (cppNamespace + "::" + trait).str();
}

bool InterfaceTrait::shouldDeclareMethods() const {
  return def->isSubClassOf("DeclareInterfaceMethods");
}

std::vector<StringRef> InterfaceTrait::getAlwaysDeclaredMethods() const {
  return def->getValueAsListOfStrings("alwaysOverriddenMethods");
}
