//===--- Bridging/DeclContextBridging.cpp ---------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTBridging.h"

#include "language/AST/DeclContext.h"
#include "language/AST/Expr.h"

using namespace language;

//===----------------------------------------------------------------------===//
// MARK: DeclContexts
//===----------------------------------------------------------------------===//

BridgedPatternBindingInitializer
BridgedPatternBindingInitializer_create(BridgedDeclContext cDeclContext) {
  return PatternBindingInitializer::create(cDeclContext.unbridged());
}

BridgedDeclContext BridgedPatternBindingInitializer_asDeclContext(
    BridgedPatternBindingInitializer cInit) {
  return cInit.unbridged();
}

BridgedDefaultArgumentInitializer
BridgedDefaultArgumentInitializer_create(BridgedDeclContext cDeclContext,
                                         size_t index) {
  return DefaultArgumentInitializer::create(cDeclContext.unbridged(), index);
}

BridgedDeclContext DefaultArgumentInitializer_asDeclContext(
    BridgedDefaultArgumentInitializer cInit) {
  return cInit.unbridged();
}

BridgedCustomAttributeInitializer
BridgedCustomAttributeInitializer_create(BridgedDeclContext cDeclContext) {
  return CustomAttributeInitializer::create(cDeclContext.unbridged());
}

BridgedDeclContext BridgedCustomAttributeInitializer_asDeclContext(
    BridgedCustomAttributeInitializer cInit) {
  return cInit.unbridged();
}

BridgedDeclContext
BridgedClosureExpr_asDeclContext(BridgedClosureExpr cClosure) {
  return cClosure.unbridged();
}
