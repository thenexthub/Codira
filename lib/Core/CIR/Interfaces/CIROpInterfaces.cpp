//====- CIROpInterfaces.cpp - Interface to AST Attributes ---------------===//
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
// Defines the interface to CIR operations.
//
//===----------------------------------------------------------------------===//
#include "language/Core/CIR/Interfaces/CIROpInterfaces.h"

using namespace cir;

/// Include the generated type qualifiers interfaces.
#include "language/Core/CIR/Interfaces/CIROpInterfaces.cpp.inc"

#include "language/Core/CIR/MissingFeatures.h"

bool CIRGlobalValueInterface::hasDefaultVisibility() {
  assert(!cir::MissingFeatures::hiddenVisibility());
  assert(!cir::MissingFeatures::protectedVisibility());
  return isPublic() || isPrivate();
}

bool CIRGlobalValueInterface::canBenefitFromLocalAlias() {
  assert(!cir::MissingFeatures::supportIFuncAttr());
  // hasComdat here should be isDeduplicateComdat, but as far as clang codegen
  // is concerned, there is no case for Comdat::NoDeduplicate as all comdat
  // would be Comdat::Any or Comdat::Largest (in the case of MS ABI). And CIRGen
  // wouldn't even generate Comdat::Largest comdat as it tries to leave ABI
  // specifics to LLVM lowering stage, thus here we don't need test Comdat
  // selectionKind.
  return hasDefaultVisibility() && hasExternalLinkage() && !isDeclaration() &&
         !hasComdat();
}
