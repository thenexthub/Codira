//=======- DiagOutputUtils.h -------------------------------------*- C++ -*-==//
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

#ifndef LANGUAGE_CORE_ANALYZER_WEBKIT_DIAGPRINTUTILS_H
#define LANGUAGE_CORE_ANALYZER_WEBKIT_DIAGPRINTUTILS_H

#include "language/Core/AST/Decl.h"
#include "toolchain/Support/raw_ostream.h"

namespace language::Core {

template <typename NamedDeclDerivedT>
void printQuotedQualifiedName(toolchain::raw_ostream &Os,
                              const NamedDeclDerivedT &D) {
  Os << "'";
  D->getNameForDiagnostic(Os, D->getASTContext().getPrintingPolicy(),
                          /*Qualified=*/true);
  Os << "'";
}

template <typename NamedDeclDerivedT>
void printQuotedName(toolchain::raw_ostream &Os, const NamedDeclDerivedT &D) {
  Os << "'";
  D->getNameForDiagnostic(Os, D->getASTContext().getPrintingPolicy(),
                          /*Qualified=*/false);
  Os << "'";
}

} // namespace language::Core

#endif
