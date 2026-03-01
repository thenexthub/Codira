//===- Attributes.cpp - MLIR Affine Expr Classes --------------------------===//
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

#include "mlir/IR/Attributes.h"
#include "mlir/IR/Dialect.h"

using namespace mlir;
using namespace mlir::detail;

//===----------------------------------------------------------------------===//
// AbstractAttribute
//===----------------------------------------------------------------------===//

void AbstractAttribute::walkImmediateSubElements(
    Attribute attr, function_ref<void(Attribute)> walkAttrsFn,
    function_ref<void(Type)> walkTypesFn) const {
  walkImmediateSubElementsFn(attr, walkAttrsFn, walkTypesFn);
}

Attribute
AbstractAttribute::replaceImmediateSubElements(Attribute attr,
                                               ArrayRef<Attribute> replAttrs,
                                               ArrayRef<Type> replTypes) const {
  return replaceImmediateSubElementsFn(attr, replAttrs, replTypes);
}

//===----------------------------------------------------------------------===//
// Attribute
//===----------------------------------------------------------------------===//

/// Return the context this attribute belongs to.
MLIRContext *Attribute::getContext() const { return getDialect().getContext(); }

//===----------------------------------------------------------------------===//
// NamedAttribute
//===----------------------------------------------------------------------===//

NamedAttribute::NamedAttribute(StringAttr name, Attribute value)
    : name(name), value(value) {
  assert(name && value && "expected valid attribute name and value");
  assert(!name.empty() && "expected valid attribute name");
}

NamedAttribute::NamedAttribute(StringRef name, Attribute value) : value(value) {
  assert(value && "expected valid attribute value");
  assert(!name.empty() && "expected valid attribute name");
  this->name = StringAttr::get(value.getContext(), name);
}

StringAttr NamedAttribute::getName() const {
  return toolchain::cast<StringAttr>(name);
}

Dialect *NamedAttribute::getNameDialect() const {
  return getName().getReferencedDialect();
}

void NamedAttribute::setName(StringAttr newName) {
  assert(name && "expected valid attribute name");
  name = newName;
}

bool NamedAttribute::operator<(const NamedAttribute &rhs) const {
  return getName().compare(rhs.getName()) < 0;
}

bool NamedAttribute::operator<(StringRef rhs) const {
  return getName().getValue().compare(rhs) < 0;
}
