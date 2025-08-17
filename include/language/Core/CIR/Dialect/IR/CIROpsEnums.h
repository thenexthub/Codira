//===----------------------------------------------------------------------===//
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
// This file declares the CIR enumerations.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_CIR_DIALECT_IR_CIROPSENUMS_H
#define CLANG_CIR_DIALECT_IR_CIROPSENUMS_H

#include "mlir/IR/BuiltinAttributes.h"
#include "language/Core/CIR/Dialect/IR/CIROpsEnums.h.inc"

namespace cir {

static bool isExternalLinkage(GlobalLinkageKind linkage) {
  return linkage == GlobalLinkageKind::ExternalLinkage;
}
static bool isAvailableExternallyLinkage(GlobalLinkageKind linkage) {
  return linkage == GlobalLinkageKind::AvailableExternallyLinkage;
}
static bool isLinkOnceAnyLinkage(GlobalLinkageKind linkage) {
  return linkage == GlobalLinkageKind::LinkOnceAnyLinkage;
}
static bool isLinkOnceODRLinkage(GlobalLinkageKind linkage) {
  return linkage == GlobalLinkageKind::LinkOnceODRLinkage;
}
static bool isLinkOnceLinkage(GlobalLinkageKind linkage) {
  return isLinkOnceAnyLinkage(linkage) || isLinkOnceODRLinkage(linkage);
}
static bool isWeakAnyLinkage(GlobalLinkageKind linkage) {
  return linkage == GlobalLinkageKind::WeakAnyLinkage;
}
static bool isWeakODRLinkage(GlobalLinkageKind linkage) {
  return linkage == GlobalLinkageKind::WeakODRLinkage;
}
static bool isWeakLinkage(GlobalLinkageKind linkage) {
  return isWeakAnyLinkage(linkage) || isWeakODRLinkage(linkage);
}
static bool isInternalLinkage(GlobalLinkageKind linkage) {
  return linkage == GlobalLinkageKind::InternalLinkage;
}
static bool isPrivateLinkage(GlobalLinkageKind linkage) {
  return linkage == GlobalLinkageKind::PrivateLinkage;
}
static bool isLocalLinkage(GlobalLinkageKind linkage) {
  return isInternalLinkage(linkage) || isPrivateLinkage(linkage);
}
static bool isExternalWeakLinkage(GlobalLinkageKind linkage) {
  return linkage == GlobalLinkageKind::ExternalWeakLinkage;
}
LLVM_ATTRIBUTE_UNUSED static bool isCommonLinkage(GlobalLinkageKind linkage) {
  return linkage == GlobalLinkageKind::CommonLinkage;
}
LLVM_ATTRIBUTE_UNUSED static bool
isValidDeclarationLinkage(GlobalLinkageKind linkage) {
  return isExternalWeakLinkage(linkage) || isExternalLinkage(linkage);
}

/// Whether the definition of this global may be replaced by something
/// non-equivalent at link time. For example, if a function has weak linkage
/// then the code defining it may be replaced by different code.
LLVM_ATTRIBUTE_UNUSED static bool
isInterposableLinkage(GlobalLinkageKind linkage) {
  switch (linkage) {
  case GlobalLinkageKind::WeakAnyLinkage:
  case GlobalLinkageKind::LinkOnceAnyLinkage:
  case GlobalLinkageKind::CommonLinkage:
  case GlobalLinkageKind::ExternalWeakLinkage:
    return true;

  case GlobalLinkageKind::AvailableExternallyLinkage:
  case GlobalLinkageKind::LinkOnceODRLinkage:
  case GlobalLinkageKind::WeakODRLinkage:
    // The above three cannot be overridden but can be de-refined.

  case GlobalLinkageKind::ExternalLinkage:
  case GlobalLinkageKind::InternalLinkage:
  case GlobalLinkageKind::PrivateLinkage:
    return false;
  }
  toolchain_unreachable("Fully covered switch above!");
}

/// Whether the definition of this global may be discarded if it is not used
/// in its compilation unit.
LLVM_ATTRIBUTE_UNUSED static bool
isDiscardableIfUnused(GlobalLinkageKind linkage) {
  return isLinkOnceLinkage(linkage) || isLocalLinkage(linkage) ||
         isAvailableExternallyLinkage(linkage);
}

/// Whether the definition of this global may be replaced at link time.  NB:
/// Using this method outside of the code generators is almost always a
/// mistake: when working at the IR level use isInterposable instead as it
/// knows about ODR semantics.
LLVM_ATTRIBUTE_UNUSED static bool isWeakForLinker(GlobalLinkageKind linkage) {
  return linkage == GlobalLinkageKind::WeakAnyLinkage ||
         linkage == GlobalLinkageKind::WeakODRLinkage ||
         linkage == GlobalLinkageKind::LinkOnceAnyLinkage ||
         linkage == GlobalLinkageKind::LinkOnceODRLinkage ||
         linkage == GlobalLinkageKind::CommonLinkage ||
         linkage == GlobalLinkageKind::ExternalWeakLinkage;
}

LLVM_ATTRIBUTE_UNUSED static bool isValidLinkage(GlobalLinkageKind gl) {
  return isExternalLinkage(gl) || isLocalLinkage(gl) || isWeakLinkage(gl) ||
         isLinkOnceLinkage(gl);
}

} // namespace cir

#endif // CLANG_CIR_DIALECT_IR_CIROPSENUMS_H
