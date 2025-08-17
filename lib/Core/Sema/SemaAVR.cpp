//===------ SemaAVR.cpp ---------- AVR target-specific routines -----------===//
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
//  This file implements semantic analysis functions specific to AVR.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Sema/SemaAVR.h"
#include "language/Core/AST/DeclBase.h"
#include "language/Core/Basic/DiagnosticSema.h"
#include "language/Core/Sema/Attr.h"
#include "language/Core/Sema/ParsedAttr.h"
#include "language/Core/Sema/Sema.h"

namespace language::Core {
SemaAVR::SemaAVR(Sema &S) : SemaBase(S) {}

void SemaAVR::handleInterruptAttr(Decl *D, const ParsedAttr &AL) {
  if (!isFuncOrMethodForAttrSubject(D)) {
    Diag(D->getLocation(), diag::warn_attribute_wrong_decl_type)
        << AL << AL.isRegularKeywordAttribute() << ExpectedFunction;
    return;
  }

  if (!AL.checkExactlyNumArgs(SemaRef, 0))
    return;

  // AVR interrupt handlers must have no parameter and be void type.
  if (hasFunctionProto(D) && getFunctionOrMethodNumParams(D) != 0) {
    Diag(D->getLocation(), diag::warn_interrupt_signal_attribute_invalid)
        << /*AVR*/ 3 << /*interrupt*/ 0 << 0;
    return;
  }
  if (!getFunctionOrMethodResultType(D)->isVoidType()) {
    Diag(D->getLocation(), diag::warn_interrupt_signal_attribute_invalid)
        << /*AVR*/ 3 << /*interrupt*/ 0 << 1;
    return;
  }

  handleSimpleAttribute<AVRInterruptAttr>(*this, D, AL);
}

void SemaAVR::handleSignalAttr(Decl *D, const ParsedAttr &AL) {
  if (!isFuncOrMethodForAttrSubject(D)) {
    Diag(D->getLocation(), diag::warn_attribute_wrong_decl_type)
        << AL << AL.isRegularKeywordAttribute() << ExpectedFunction;
    return;
  }

  if (!AL.checkExactlyNumArgs(SemaRef, 0))
    return;

  // AVR signal handlers must have no parameter and be void type.
  if (hasFunctionProto(D) && getFunctionOrMethodNumParams(D) != 0) {
    Diag(D->getLocation(), diag::warn_interrupt_signal_attribute_invalid)
        << /*AVR*/ 3 << /*signal*/ 1 << 0;
    return;
  }
  if (!getFunctionOrMethodResultType(D)->isVoidType()) {
    Diag(D->getLocation(), diag::warn_interrupt_signal_attribute_invalid)
        << /*AVR*/ 3 << /*signal*/ 1 << 1;
    return;
  }

  handleSimpleAttribute<AVRSignalAttr>(*this, D, AL);
}

} // namespace language::Core
