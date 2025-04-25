//===--- Effects.cpp - Effect Checking ASTs -------------------------------===//
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
//
//  This file implements some logic for rethrows and reasync checking.
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTContext.h"
#include "language/AST/Effects.h"
#include "language/AST/Evaluator.h"
#include "language/AST/Decl.h"
#include "language/AST/ProtocolConformanceRef.h"
#include "language/AST/Type.h"
#include "language/AST/Types.h"
#include "language/AST/TypeCheckRequests.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/raw_ostream.h"

using namespace language;

bool AnyFunctionType::hasEffect(EffectKind kind) const {
  switch (kind) {
  case EffectKind::Throws: return getExtInfo().isThrowing();
  case EffectKind::Async: return getExtInfo().isAsync();
  case EffectKind::Unsafe: return false;
  }
  llvm_unreachable("Bad effect kind");
}

void swift::simple_display(llvm::raw_ostream &out, const EffectKind kind) {
  switch (kind) {
  case EffectKind::Throws: out << "throws"; return;
  case EffectKind::Async: out << "async"; return;
  case EffectKind::Unsafe: out << "@unsafe"; return;
  }
  llvm_unreachable("Bad effect kind");
}

void swift::simple_display(llvm::raw_ostream &out,
                           const PolymorphicEffectRequirementList list) {
  for (auto req : list.getRequirements()) {
    simple_display(out, req);
    out << "\n";
  }

  for (auto conf : list.getConformances()) {
    simple_display(out, conf.first);
    out << " : ";
    simple_display(out, conf.second);
    llvm::errs() << "\n";
  }
}

PolymorphicEffectRequirementList 
ProtocolDecl::getPolymorphicEffectRequirements(EffectKind kind) const {
  return evaluateOrDefault(getASTContext().evaluator,
    PolymorphicEffectRequirementsRequest{kind, const_cast<ProtocolDecl *>(this)},
    PolymorphicEffectRequirementList());
}

bool ProtocolDecl::hasPolymorphicEffect(EffectKind kind) const {
  switch (kind) {
  case EffectKind::Throws:
    return getAttrs().hasAttribute<swift::AtRethrowsAttr>();
  case EffectKind::Async:
    return getAttrs().hasAttribute<swift::AtReasyncAttr>();
  case EffectKind::Unsafe:
    return false;
  }
  llvm_unreachable("Bad effect kind");
}

bool AbstractFunctionDecl::hasEffect(EffectKind kind) const {
  switch (kind) {
  case EffectKind::Throws:
    return hasThrows();
  case EffectKind::Async:
    return hasAsync();
  case EffectKind::Unsafe:
    return getExplicitSafety() == ExplicitSafety::Unsafe;
  }
  llvm_unreachable("Bad effect kind");
}

bool AbstractFunctionDecl::hasPolymorphicEffect(EffectKind kind) const {
  switch (kind) {
  case EffectKind::Throws:
    return getAttrs().hasAttribute<swift::RethrowsAttr>();
  case EffectKind::Async:
    return getAttrs().hasAttribute<swift::ReasyncAttr>();
  case EffectKind::Unsafe:
    return false;
  }
  llvm_unreachable("Bad effect kind");
}

PolymorphicEffectKind
AbstractFunctionDecl::getPolymorphicEffectKind(EffectKind kind) const {
  return evaluateOrDefault(getASTContext().evaluator,
    PolymorphicEffectKindRequest{kind, const_cast<AbstractFunctionDecl *>(this)},
    PolymorphicEffectKind::Invalid);
}

void swift::simple_display(llvm::raw_ostream &out,
                           PolymorphicEffectKind kind) {
  switch (kind) {
  case PolymorphicEffectKind::None:
    out << "none";
    break;
  case PolymorphicEffectKind::ByClosure:
    out << "by closure";
    break;
  case PolymorphicEffectKind::ByConformance:
    out << "by conformance";
    break;
  case PolymorphicEffectKind::AsyncSequenceRethrows:
    out << "by async sequence implicit @rethrows";
    break;
  case PolymorphicEffectKind::Always:
    out << "always";
    break;
  case PolymorphicEffectKind::Invalid:
    out << "invalid";
    break;
  }
}

bool ProtocolConformanceRef::hasEffect(EffectKind kind) const {
  if (!isConcrete()) { return kind != EffectKind::Unsafe; }
  return evaluateOrDefault(getProtocol()->getASTContext().evaluator,
     ConformanceHasEffectRequest{kind, getConcrete()},
     true);
}
