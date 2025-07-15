//===--- CodeCompletionResultsArray.h - -------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKITD_CODECOMPLETION_RESULTS_ARRAY_H
#define TOOLCHAIN_SOURCEKITD_CODECOMPLETION_RESULTS_ARRAY_H

#include "sourcekitd/Internal.h"

namespace sourcekitd {

VariantFunctions *getVariantFunctionsForCodeCompletionResultsArray();

class CodeCompletionResultsArrayBuilder {
public:
  CodeCompletionResultsArrayBuilder();
  ~CodeCompletionResultsArrayBuilder();

  void add(SourceKit::UIdent Kind, toolchain::StringRef Name,
           toolchain::StringRef Description, toolchain::StringRef SourceText,
           toolchain::StringRef TypeName, std::optional<toolchain::StringRef> ModuleName,
           std::optional<toolchain::StringRef> DocBrief,
           std::optional<toolchain::StringRef> AssocUSRs,
           SourceKit::UIdent SemanticContext, SourceKit::UIdent TypeRelation,
           bool NotRecommended, bool IsSystem, unsigned NumBytesToErase);

  std::unique_ptr<toolchain::MemoryBuffer> createBuffer();

private:
  struct Implementation;
  Implementation &Impl;
};

}

#endif
